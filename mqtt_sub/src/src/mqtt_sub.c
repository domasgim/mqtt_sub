#include "mqtt_sub.h"

atomic_int interrupt = 0;

void sigHandler(int signo) {
	interrupt = 1;
}

int digits_only(char * s) {
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }
    return 1;
}

int compare_strings(event_t event, char *json_item, char *topic) {
    char comparison_val_fixed[256];
    snprintf(comparison_val_fixed, sizeof(comparison_val_fixed), "\"%s\"", event.comparison_val);
    switch(event.operator) {
        case EQUAL:
            if(strcmp(json_item, comparison_val_fixed) == 0) {
                return 1;
            }
            break;
        case NOT_EQUAL:
            if(strcmp(json_item, comparison_val_fixed) != 0) {
                return 1;
            }
            break;
        case MORE_EQ:
            if(strcmp(json_item, comparison_val_fixed) >= 0) {
                return 1;
            }
            break;
        case LESS_EQ:
            if(strcmp(json_item, comparison_val_fixed) <= 0) {
                return 1;
            }
            break;
        default:
            return 0;
            break;
    }
    return 0;
}

int compare_integers(event_t event, int json_item, char *topic) {
    char json_item_str[256];
    snprintf(json_item_str, sizeof(json_item_str), "%d", json_item);
    if(digits_only(event.comparison_val) != 1) {
        fprintf(stderr, "Error: specified integer number to be compared to but the comparison contains characters (%s)\n", event.comparison_val);
        return -1;
    }
    int comparison_val_int = (int) strtol(event.comparison_val, (char **)NULL, 10);

    switch(event.operator) {
        case EQUAL:
            if(json_item == comparison_val_int) {
                return 1;
            }
            break;
        case NOT_EQUAL:
            if(json_item != comparison_val_int) {
                return 1;
            }
            break;
        case MORE:
            if(json_item > comparison_val_int) {
                return 1;
            }
            break;
        case MORE_EQ:
            if(json_item >= comparison_val_int) {
                return 1;
            }
            break;
        case LESS:
            if(json_item < comparison_val_int) {
                return 1;
            }
            break;
        case LESS_EQ:
            if(json_item <= comparison_val_int) {
                return 1;
            }
            break;
        default:
            return 0;
            break;
    }
    return 0;
}

int process_events(event_node_t *events, char * topic, char * payload) {
    int rc = 0;
    struct json_object *json_payload;
    struct json_object *item;
    event_node_t *events_tmp = events;

    json_payload = json_tokener_parse(payload);
    if(json_payload == NULL) {
        free(json_payload);
        return rc;
    }


    while (events_tmp != NULL) {
        item = json_object_object_get(json_payload, events_tmp->event.json_val);
        char *item_str = json_object_to_json_string(item);
        int item_int;
        if (item == NULL) {
            goto next_iteration;
        }
        if(json_object_is_type(item, 3)) {
            item_int = (int)json_object_get_int64(item);
        }

        switch(events_tmp->event.val_type_enum) {
            case STRING:
                if(compare_strings(events_tmp->event, item_str, topic) == 1) {
                    fprintf(stdout, "Event match on %s - %s: %s %s %s\n", topic, events_tmp->event.json_val, 
                        item_str, events_tmp->event.operator_str, events_tmp->event.comparison_val);
                    curl_send_email(events_tmp->event, item_str, topic);
                }
                break;

            case INT:
                printf("Integer\n");
                if(compare_integers(events_tmp->event, item_int, topic) == 1) {
                    fprintf(stdout, "Event match on %s - %s: %s %s %s\n", topic, events_tmp->event.json_val, 
                        item_str, events_tmp->event.operator_str, events_tmp->event.comparison_val);
                    curl_send_email(events_tmp->event, item_str, topic);
                }
                break;
            default:
                break;
        }
        next_iteration:
            events_tmp = events_tmp->next;
    }

    end:
        free(json_payload);
        return rc;

}

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    int rc;
    mqtt_options_t *mqtt_options = (mqtt_options_t*) obj;
    topic_node_t *tmp = mqtt_options->topics;
    printf("%s\n", mosquitto_connack_string(reason_code));

	if(reason_code != 0) {
		mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
	    mosquitto_lib_cleanup();
        exit(-1);
	}

    while (tmp != NULL) {
        rc = mosquitto_subscribe(mosq, NULL, tmp->topic.topic_name, tmp->topic.qos);
        if(rc != MOSQ_ERR_SUCCESS){
            fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
            mosquitto_disconnect(mosq);
        }
        tmp = tmp->next;
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    mqtt_options_t *mqtt_options = (mqtt_options_t*) obj;
    topic_node_t *topic_tmp = mqtt_options->topics;

    fprintf(stdout, "Message received on topic %s: %s\n", msg->topic, (char *) msg->payload);
    sqlite3_insert(mqtt_options->sqlite3_db, msg->topic, (char *) msg->payload);

    while (topic_tmp != NULL) {
        if(strcmp(topic_tmp->topic.topic_name, msg->topic) == 0) {
            process_events(topic_tmp->topic.events, msg->topic, msg->payload);
        }
        topic_tmp = topic_tmp->next;
    }
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
	int i;
	bool have_subscription = false;

	for(i=0; i<qos_count; i++){
		fprintf(stdout, "Subscribed, qos = %d\n", granted_qos[i]);
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
    char user_pw_command[] = "mqtt_sub.options.use_username_pw";
    char * use_username_pw = uci_get_config_entry(user_pw_command);

    int username_pw_flag = (int) strtol(use_username_pw, (char **)NULL, 10);

    if(username_pw_flag == 0) {
        char user_command = "mqtt_sub.options.username";
        char password_command = "mqtt_sub.options.password";
        char * username = uci_get_config_entry(user_command);
        char * password = uci_get_config_entry(password_command);

        rc = mosquitto_username_pw_set(mosq, username, password);
        if(rc) {
            fprintf(stderr, "Error setting username and password - %s\n", mosquitto_strerror(errno));
        } 
    }
    return rc;
}

int mosq_set_tls(struct mosquitto * mosq) {
    char use_tls_command[] = "mqtt_sub.options.use_tls_ssl";
    char * use_tls = uci_get_config_entry(use_tls_command);

    int tls_flag = (int) strtol(use_tls, (char **)NULL, 10);
    int rc = 0;

    if (tls_flag == 1) {
        char ca_command[] = "mqtt_sub.options.ca_file";
        char cert_command[] = "mqtt_sub.options.cert_file";
        char key_command[] = "mqtt_sub.options.key_file";

        char * ca_file = uci_get_config_entry(ca_command);
        char * cert_file = uci_get_config_entry(cert_command);
        char * key_file = uci_get_config_entry(key_command);

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

    char host_command[] = "mqtt_sub.config.host";
    char port_command[] = "mqtt_sub.config.port";
    char * host = uci_get_config_entry(host_command);
    char * port_str = uci_get_config_entry(port_command);

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

int mqtt_options_setup(mqtt_options_t *mqtt_options) {
    int rc;

    topic_node_t *topic_list = uci_get_topic_list();
    *mqtt_options->topics = *topic_list;

    sqlite3 *db = sqlite3_open_database();
    if(db == NULL) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    mqtt_options->sqlite3_db = db;
    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

    int rc = 0;

    mqtt_options_t mqtt_options;
    rc = mqtt_options_setup(&mqtt_options);
    if( rc ) {
        free(&mqtt_options);
        return -1;
    } 

	mosquitto_lib_init();
	struct mosquitto *mosq;

    mosq = mosquitto_new("mqtt_sub", true, &mqtt_options);
    if (mosq == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        sqlite3_close(mqtt_options.sqlite3_db);
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
        sqlite3_close(mqtt_options.sqlite3_db);
        return rc;
}