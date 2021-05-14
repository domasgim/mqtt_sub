#include "uci_helper.h"

extern int uci_get_user_group_iteration(char * email_group) {
    int user_group_iteration = 0;
    while (user_group_iteration < 256) {
        char command_buffer[64];
        snprintf(command_buffer, 64, "user_groups.@email[%d].name", user_group_iteration);
        char * user_group_searched = uci_get_config_entry(command_buffer);
        if (user_group_searched == NULL) {
            return -1;
        } else if (strcmp(user_group_searched, email_group) == 0) {
            return user_group_iteration;
        }
        user_group_iteration++;
    }
    return -1;
}

extern char * uci_get_topic_section_id(char * topic) {
    int topics_iteration = 0;
    while (topics_iteration < 256) {
        char command_buffer[64];
        snprintf(command_buffer, 64, "mqtt_sub.@topic[%i].topic", topics_iteration);
        char * topic_searched = uci_get_config_entry(command_buffer);
        if (topic_searched == NULL) {
            return NULL;
        } else if (strcmp(topic_searched, topic) == 0) {
            char command_sid_buffer[64];
            snprintf(command_sid_buffer, 64, "mqtt_sub.@topic[%i].section_id", topics_iteration);
            char * section_id = uci_get_config_entry(command_sid_buffer);
            if (section_id != NULL) {
                return section_id;
            }
            // TODO
            return NULL;
        }
        topics_iteration++;
    }
    return NULL;
}

extern char * uci_get_config_entry (char *path) {
    char * result;
    struct uci_context *c;
    struct uci_ptr ptr;

    c = uci_alloc_context ();
    if (uci_lookup_ptr (c, &ptr, path, true) != UCI_OK) {
        uci_perror (c, "get_config_entry Error");
        goto error;
    } else if (ptr.flags & UCI_LOOKUP_COMPLETE) {
        struct uci_element *e;
        bool sep = false;

        switch(ptr.o->type) {
        case UCI_TYPE_STRING:
            result = strdup(ptr.o->v.string);
            uci_free_context (c);
            return result;
            break;
        case UCI_TYPE_LIST:
            goto error;
            break;
        default:
            goto error;
            break;
        }
    } else {
        goto error;
    }
        
    error:
        uci_free_context (c);
        return NULL;
}