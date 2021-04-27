#include "mqtt_sub.h"

atomic_int interrupt = 0;

/**
 * @brief Database variable. Used when opening it
 * during startup and passing ONLY to MQTT on_message callback to store
 * topic information to sqlite3 database
 * 
 */
sqlite3 *db;


void sigHandler(int signo) {
	interrupt = 1;
}

int digits_only(char * s) {
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

int process_events(char * section_id, char * topic, char * payload) {
    int section_iteration = 0;
    int rc = 0;

    cJSON *json_payload = cJSON_Parse(payload);
    if (json_payload == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        rc = -1;
        goto end;
    }
    /* The mqtt_sub UCI configuration can contain information about events which have the same section ID of the topic. 
    We iterate through all of the available event sections and if a match is found, we send an email */
    while (section_iteration < 255) {
        const cJSON *json_val = NULL;
        const cJSON *json_val_name = NULL;
        char json_command_buffer[64];
        char type_command_buffer[64];
        char operator_command_buffer[64];
        char comparison_val_command_buffer[64];
        char email_group_buffer[64];
        char recip_email_buffer[64];
        char strcmp_command_buffer[64];

        snprintf(json_command_buffer, sizeof(json_command_buffer), "mqtt_sub.@%s[%i].json_val", section_id, section_iteration);
        snprintf(type_command_buffer, sizeof(type_command_buffer), "mqtt_sub.@%s[%i].val_type", section_id, section_iteration);
        snprintf(operator_command_buffer, sizeof(operator_command_buffer), "mqtt_sub.@%s[%i].operator", section_id, section_iteration);
        snprintf(comparison_val_command_buffer, sizeof(comparison_val_command_buffer), "mqtt_sub.@%s[%i].comparison_val", section_id, section_iteration);
        snprintf(email_group_buffer, sizeof(email_group_buffer), "mqtt_sub.@%s[%i].email_group", section_id, section_iteration);
        snprintf(recip_email_buffer, sizeof(recip_email_buffer), "mqtt_sub.@%s[%i].recip_email", section_id, section_iteration);

        char * json_val_str = uci_get_config_entry_V2(json_command_buffer);
        char * val_type = uci_get_config_entry_V2(type_command_buffer);
        char * operator = uci_get_config_entry_V2(operator_command_buffer);
        char * comparison_val = uci_get_config_entry_V2(comparison_val_command_buffer);
        char * email_group = uci_get_config_entry_V2(email_group_buffer);
        char * recip_email = uci_get_config_entry_V2(recip_email_buffer);

        if (json_val_str == NULL || val_type == NULL || operator == NULL || comparison_val == NULL
            || email_group == NULL || recip_email == NULL) {
                goto end;
        }
        json_val = cJSON_GetObjectItemCaseSensitive(json_payload, json_val_str);

        /* This part is a bit messy but it's purpose is to actually do the comparison part. We now have the operator value (+, -, =, etc.),
        the JSON value and the value to be compared to.
        */
        if(strcmp(val_type, "string") == 0 && cJSON_IsString(json_val) && (json_val->valuestring != NULL)) {
            if(strcmp(operator, "=") == 0) {
                if(strcmp(json_val->valuestring, comparison_val) == 0) { 
                    printf("Event match on %s - %s: %s %s %s\n", topic, json_val_str, json_val->valuestring, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if (strcmp(operator, "!=") == 0) {
                if(strcmp(json_val->valuestring, comparison_val) != 0) { 
                    printf("Event match on %s - %s: %s %s %s\n", topic, json_val_str, json_val->valuestring, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if (strcmp(operator, "<") == 0) {
                if(strcmp(json_val->valuestring, comparison_val) < 0) { 
                    printf("Event match on %s - %s: %s %s %s\n", topic, json_val_str, json_val->valuestring, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if (strcmp(operator, "<=") == 0) {
                if(strcmp(json_val->valuestring, comparison_val) <= 0) { 
                    printf("Event match on %s - %s: %s %s %s\n", topic, json_val_str, json_val->valuestring, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if (strcmp(operator, ">") == 0) {
                if(strcmp(json_val->valuestring, comparison_val) > 0) { 
                    printf("Event match on %s - %s: %s %s %s\n", topic, json_val_str, json_val->valuestring, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if (strcmp(operator, ">=") == 0) {
                if(strcmp(json_val->valuestring, comparison_val) >= 0) { 
                    printf("Event match on %s - %s: %s %s %s\n", topic, json_val_str, json_val->valuestring, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            }
        } else if(strcmp(val_type, "int") == 0 && cJSON_IsNumber(json_val) && (json_val->valueint != NULL)) {
            if(digits_only(comparison_val) != 1) {
                fprintf(stderr, "Error: specified integer number to be compared to but the comparison contains characters (%s)\n", comparison_val);
                rc = -1;
                goto end;
            }
            int comparison_val_int = (int) strtol(comparison_val, (char **)NULL, 10);
            if(strcmp(operator, "=") == 0) {
                if(json_val->valueint == comparison_val_int) { 
                    printf("Event match on %s - %s: %d %s %s\n", topic, json_val_str, json_val->valueint, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if(strcmp(operator, "!=") == 0) {
                if(json_val->valueint != comparison_val_int) { 
                    printf("Event match on %s - %s: %d %s %s\n", topic, json_val_str, json_val->valueint, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if(strcmp(operator, "<") == 0) {
                if(json_val->valueint < comparison_val_int) { 
                    printf("Event match on %s - %s: %d %s %s\n", topic, json_val_str, json_val->valueint, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if(strcmp(operator, "<=") == 0) {
                if(json_val->valueint <= comparison_val_int) { 
                    printf("Event match on %s - %s: %d %s %s\n", topic, json_val_str, json_val->valueint, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if(strcmp(operator, ">") == 0) {
                if(json_val->valueint > comparison_val_int) { 
                    printf("Event match on %s - %s: %d %s %s\n", topic, json_val_str, json_val->valueint, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            } else if(strcmp(operator, ">=") == 0) {
                if(json_val->valueint >= comparison_val_int) { 
                    printf("Event match on %s - %s: %d %s %s\n", topic, json_val_str, json_val->valueint, operator, comparison_val);
                    curl_send_email(email_group, recip_email, topic, json_val_str, json_val, operator, comparison_val);
                }
            }
        }
        section_iteration++;
    }
    end:
        cJSON_Delete(json_payload);
        return rc;
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

    if(topic_list == NULL || qos_list == NULL || topic_list->len != qos_list->len) {
        printf("No entries found or some values are missing\n");
        return;
    }
    list_node_t *node_topic;
    list_node_t *node_qos;
    list_iterator_t *it_topic = list_iterator_new(topic_list, LIST_HEAD);
    list_iterator_t *it_qos = list_iterator_new(qos_list, LIST_HEAD);
    while ((node_topic = list_iterator_next(it_topic)) && (node_qos = list_iterator_next(it_qos))) {
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

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
	printf("Message received on topic %s: %s\n", msg->topic, (char *) msg->payload);
    sqlite3_insert(db, msg->topic, (char *) msg->payload);
    char * sid = uci_get_topic_section_id(msg->topic);
    process_events(sid, msg->topic, msg->payload);
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
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
            fprintf(stderr, "Error setting TLS - %s\n", mosquitto_strerror(errno));
            return -1;
        }
    }
    return rc;
}

int mosq_setup(struct mosquitto * mosq) {
    int rc = 0;
    int id = 12;

    char * host = get_config_entry("mqtt_sub", "host");
    char * port_str = get_config_entry("mqtt_sub", "port");

    if (host == NULL || port_str == NULL) {
        fprintf(stderr, "Error: no host or port address specified\n");
        return -1;
    }

    int port = (int) strtol(port_str, (char **)NULL, 10);

    rc = mosq_set_username_pw(mosq);
    if(rc) {
        return rc;
    }

    rc = mosq_set_tls(mosq);
    if(rc) {
        return rc;
    }

	mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
	mosquitto_message_callback_set(mosq, on_message);
	
	rc = mosquitto_connect(mosq, host, port, 10);
    if(rc) {
		fprintf(stderr, "Could not connect to Broker - %s\n", mosquitto_strerror(errno));
		return rc;
	}

    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

    int rc = 0;
    int id = 12;

    rc = sqlite3_open("/log/mqtt_sub.db", &db);
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    } 

	mosquitto_lib_init();
	struct mosquitto *mosq;

    mosq = mosquitto_new("mqtt_sub", true, &id);
    if (mosq == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        sqlite3_close(db);
        return -1;
    }

    rc = mosq_setup(mosq);
    if(rc) {
        goto mosq_destroy;
    }

    mosquitto_loop_start(mosq);
    while (!interrupt) { }
	mosquitto_loop_stop(mosq, true);

	mosquitto_disconnect(mosq);
    mosq_destroy:
	    mosquitto_destroy(mosq);
	    mosquitto_lib_cleanup();
        sqlite3_close(db);
        return rc;
}