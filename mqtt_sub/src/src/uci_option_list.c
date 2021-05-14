#include "uci_option_list.h"

topic_node_t *topic_create_new_node(topic_t topic) {
    topic_node_t *result = malloc(sizeof(topic_node_t));
    result->topic = topic;
    result->next = NULL;
    return result;
}

topic_node_t *topic_insert_at_head(topic_node_t **head, topic_node_t *node_to_insert) {
    node_to_insert->next = *head;
    *head = node_to_insert;
    return node_to_insert;
}

event_node_t *event_create_new_node(event_t event) {
    event_node_t *result = malloc(sizeof(event_node_t));
    result->event = event;
    result->next = NULL;
    return result;
}

event_node_t *event_insert_at_head(event_node_t **head, event_node_t *node_to_insert) {
    node_to_insert->next = *head;
    *head = node_to_insert;
    return node_to_insert;
}

int event_empty(event_t event) {
    if(event.json_val == NULL || event.val_type == NULL 
        || event.comparison_val == NULL || event.email_group == NULL || event.recip_email == NULL
        || event.smtp_ip == NULL || event.smtp_port == NULL || event.secure_conn == NULL 
        || event.senderemail == NULL) {
        return 1;
    }
    return 0;
}

topic_t uci_get_topic_options(int topic_iteration) {
    char topic_command_buffer[64];
    char section_id_command_buffer[64];
    char qos_command_buffer[64];
        
    snprintf(topic_command_buffer, sizeof(topic_command_buffer), "mqtt_sub.@topic[%i].topic", topic_iteration);
    snprintf(section_id_command_buffer, sizeof(section_id_command_buffer), "mqtt_sub.@topic[%i].section_id", topic_iteration);
    snprintf(qos_command_buffer, sizeof(qos_command_buffer), "mqtt_sub.@topic[%i].qos", topic_iteration);

    char * topic_name = uci_get_config_entry(topic_command_buffer);
    char * section_id = uci_get_config_entry(section_id_command_buffer);
    char * qos_str = uci_get_config_entry(qos_command_buffer);

    topic_t topic;
    if(topic_name == NULL || section_id == NULL || qos_str == NULL) {
        // topic.topic_name == NULL;
        return topic;
    }

    int qos = (int) strtol(qos_str, (char **)NULL, 10);

    topic.topic_name = topic_name;
    topic.section_id = section_id;
    topic.qos = qos;

    return topic;
}

event_t uci_get_event_options(int section_iteration, char * section_id) {
    event_t event;

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

    char * json_val_str = uci_get_config_entry(json_command_buffer);
    char * val_type = uci_get_config_entry(type_command_buffer);
    char * operator = uci_get_config_entry(operator_command_buffer);
    char * comparison_val = uci_get_config_entry(comparison_val_command_buffer);
    char * email_group = uci_get_config_entry(email_group_buffer);
    char * recip_email = uci_get_config_entry(recip_email_buffer);

    if(json_val_str == NULL || val_type == NULL || operator == NULL || comparison_val == NULL ||
        email_group == NULL || recip_email == NULL) {
        return event;
    }

    int user_group_iteration = uci_get_user_group_iteration(email_group);
    if (user_group_iteration == -1) {
        fprintf(stderr, "Error: %s email group was not found\n", email_group);
        return event;
    }

    char secure_conn_command_buffer[64];
    char smtp_ip_command_buffer[64];
    char smtp_port_command_buffer[64];
    char username_command_buffer[64];
    char password_command_buffer[64];
    char senderemail_command_buffer[64];

    snprintf(secure_conn_command_buffer, sizeof(secure_conn_command_buffer), "user_groups.@email[%d].secure_conn", user_group_iteration);
    snprintf(smtp_ip_command_buffer, sizeof(smtp_ip_command_buffer), "user_groups.@email[%d].smtp_ip", user_group_iteration);
    snprintf(smtp_port_command_buffer, sizeof(smtp_port_command_buffer), "user_groups.@email[%d].smtp_port", user_group_iteration);
    snprintf(username_command_buffer, sizeof(username_command_buffer), "user_groups.@email[%d].username", user_group_iteration);
    snprintf(password_command_buffer, sizeof(password_command_buffer), "user_groups.@email[%d].password", user_group_iteration);
    snprintf(senderemail_command_buffer, sizeof(senderemail_command_buffer), "user_groups.@email[%d].senderemail", user_group_iteration);

    char * secure_conn = uci_get_config_entry(secure_conn_command_buffer);
    char * smtp_ip = uci_get_config_entry(smtp_ip_command_buffer);
    char * smtp_port = uci_get_config_entry(smtp_port_command_buffer);
    char * username = uci_get_config_entry(username_command_buffer);
    char * password = uci_get_config_entry(password_command_buffer);
    char * senderemail = uci_get_config_entry(senderemail_command_buffer);

    if(smtp_ip == NULL || smtp_port == NULL || secure_conn == NULL || senderemail == NULL) {
        return event;
    } else if(json_val_str == NULL || val_type == NULL || operator == NULL || comparison_val == NULL
        || email_group == NULL || recip_email == NULL) {
        return event;
    }

    event.json_val = json_val_str;
    event.val_type = val_type;
    event.comparison_val = comparison_val;
    event.email_group = email_group;
    event.recip_email = recip_email;
    event.secure_conn = secure_conn;
    event.smtp_ip = smtp_ip;
    event.smtp_port = smtp_port;
    event.username = username;
    event.password = password;
    event.senderemail = senderemail;
    event.operator_str = operator;

    if(strcmp(val_type, "string") == 0) {
        event.val_type_enum = STRING;
    } else if (strcmp(val_type, "int") == 0) {
        event.val_type_enum = INT;
    }
    if(strcmp(operator, "=") == 0) {
        event.operator = EQUAL;
    } else if(strcmp(operator, "!=") == 0) {
        event.operator = NOT_EQUAL;
    } else if(strcmp(operator, ">") == 0) {
        event.operator = MORE;
    } else if(strcmp(operator, ">=") == 0) {
        event.operator = MORE_EQ;
    } else if(strcmp(operator, "<") == 0) {
        event.operator = LESS;
    } else if(strcmp(operator, "<=") == 0) {
        event.operator = LESS_EQ;
    }

    return event;
}

topic_node_t * uci_get_topic_list() {
    topic_node_t *topic_head = NULL;
    topic_node_t *topic_tmp;

    int topic_iteration = 0;
    int rc = 0;

    while (topic_iteration < 128) {
        int section_iteration = 0;
        topic_t topic = uci_get_topic_options(topic_iteration);
        event_node_t *event_head = NULL;
        event_node_t *event_tmp;

        if (topic.topic_name == NULL) {
            return topic_head;
        }
        while (section_iteration < 128) {
            event_t event = uci_get_event_options(section_iteration, topic.section_id);
            if (event_empty(event) == 1) {
                section_iteration = 128;
            } else {
                event_tmp = event_create_new_node(event);
                event_insert_at_head(&event_head, event_tmp);
                section_iteration++;
            }
        }
        topic.events = event_head;
        topic_tmp = topic_create_new_node(topic);
        topic_insert_at_head(&topic_head, topic_tmp);
        topic_iteration++;
    }
    return topic_head;
}