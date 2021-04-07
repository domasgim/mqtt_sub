#include "list.h"
#include "mqtt_sub.h"

atomic_int interrupt = 0;

void sigHandler(int signo) {
	interrupt = 1;
}

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    int rc;
    list_t *topic_list = list_new();
    list_t *qos_list = list_new();

    printf("%s\n", mosquitto_connack_string(reason_code));

	if(reason_code != 0) {
		mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
	    mosquitto_lib_cleanup();
        exit(-1);
	}

    topic_list = get_config_entry_list("mqtt_sub", "topic");
    qos_list = get_config_entry_list("mqtt_sub", "qos");

    if(topic_list == NULL || qos_list == NULL) {
        printf("No entries found\n");
    } else {
        list_node_t *node_topic;
        list_node_t *node_qos;
        list_iterator_t *it_topic = list_iterator_new(topic_list, LIST_HEAD);
        list_iterator_t *it_qos = list_iterator_new(qos_list, LIST_HEAD);
        while ((node_topic = list_iterator_next(it_topic)) && (node_qos = list_iterator_next(it_qos))) {
            // printf("%s - %s\n", node_topic->val, node_qos->val);
            int qos = (int) strtol(node_qos->val, (char **)NULL, 10);

            rc = mosquitto_subscribe(mosq, NULL, node_topic->val, qos);
            if(rc != MOSQ_ERR_SUCCESS){
                fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
                mosquitto_disconnect(mosq);
            }
        }

        list_iterator_destroy(it_topic);
        list_iterator_destroy(it_qos);

        list_destroy(topic_list);
        list_destroy(qos_list);
    }
	// mosquitto_subscribe(mosq, NULL, request, 0);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
	printf("Message received on topic %s: %s\n", msg->topic, (char *) msg->payload);
    sqlite3_insert(msg->topic, (char *) msg->payload);
    /* Write to logread but printf works too if we launch it with init.d */
    //openlog("mqtt_sub", LOG_PID, LOG_USER);
    //char * log_msg = "%s - %s\n", msg->topic, (char *) msg->payload;
    //syslog(LOG_INFO, "%s - %s\n", msg->topic, (char *) msg->payload);   /* Write to log */
    closelog();
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	bool have_subscription = false;

	for(i=0; i<qos_count; i++){
		printf("Subscribed, qos = %d\n", granted_qos[i]);
		if(granted_qos[i] <= 2){
			have_subscription = true;
		}
	}
	if(have_subscription == false){
		fprintf(stderr, "Error: All subscriptions rejected.\n");
		mosquitto_disconnect(mosq);
	}
}

int mosq_set_username_pw(struct mosquitto * mosq) {
    int rc = 0;
    char * use_username_pw = get_config_entry("mqtt_sub", "use_username_pw");

    int username_pw_flag = (int) strtol(use_username_pw, (char **)NULL, 10);

    if(username_pw_flag == 0) {
        char * username = get_config_entry("mqtt_sub", "username");
        char * password = get_config_entry("mqtt_sub", "password");

        rc = mosquitto_username_pw_set(mosq, username, password);
        if(rc) {
            fprintf(stderr, "Error setting username and password - %s\n", mosquitto_strerror(errno));
        } 
    }
    return rc;
}

int mosq_set_tls(struct mosquitto * mosq) {
    char * use_tls = get_config_entry("mqtt_sub", "use_tls_ssl");
    int tls_flag = (int) strtol(use_tls, (char **)NULL, 10);
    int rc = 0;

    if (tls_flag == 1) {
        char * ca_file = get_config_entry("mqtt_sub", "ca_file");
        char * cert_file = get_config_entry("mqtt_sub", "cert_file");
        char * key_file = get_config_entry("mqtt_sub", "key_file");

        if(ca_file == NULL || cert_file == NULL || key_file == NULL) {
            fprintf(stderr, "Error getting certificate files\n");
            return -1;
        }
            
        rc = mosquitto_tls_set(mosq, ca_file, NULL, cert_file, key_file, NULL);
        if(rc) {
            printf("Error setting TLS - %s\n", mosquitto_strerror(errno));
            return -1;
        }
    }
    return rc;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

    int rc = 0;
    int id = 12;
    char * host = get_config_entry("mqtt_sub", "host");
    char * port_str = get_config_entry("mqtt_sub", "port");

    int port = (int) strtol(port_str, (char **)NULL, 10);

	mosquitto_lib_init();
	struct mosquitto *mosq;

	mosq = mosquitto_new("mqtt_sub", true, &id);
    if (mosq == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        return 1;
    }

    rc = mosq_set_username_pw(mosq);
    if(rc) {
        goto mosq_destroy;
    }

    rc = mosq_set_tls(mosq);
    if(rc) {
        goto mosq_destroy;
    }

	mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
	mosquitto_message_callback_set(mosq, on_message);
	
	rc = mosquitto_connect(mosq, host, port, 10);
    if(rc) {
		printf("Could not connect to Broker - %s\n", mosquitto_strerror(errno));
		goto mosq_destroy;
	}
    mosquitto_loop_start(mosq);
    while (!interrupt) {

    }
	mosquitto_loop_stop(mosq, true);

	mosquitto_disconnect(mosq);
    mosq_destroy:
	    mosquitto_destroy(mosq);
	    mosquitto_lib_cleanup();
        return rc;
}