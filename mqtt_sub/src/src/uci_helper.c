#include "mqtt_sub.h"

int uci_get_user_group_iteration(char * email_group) {
    int user_group_iteration = 0;
    while (user_group_iteration < 256) {
        char command_buffer[64];
        snprintf(command_buffer, 64, "user_groups.@email[%d].name", user_group_iteration);
        char * user_group_searched = uci_get_config_entry_V2(command_buffer);
        if (user_group_searched == NULL) {
            return -1;
        } else if (strcmp(user_group_searched, email_group) == 0) {
            return user_group_iteration;
        }
        user_group_iteration++;
    }
    return -1;
}

char * uci_get_topic_section_id(char * topic) {
    int topics_iteration = 0;
    while (topics_iteration < 256) {
        char command_buffer[64];
        snprintf(command_buffer, 64, "mqtt_sub.@topic[%i].topic", topics_iteration);
        char * topic_searched = uci_get_config_entry_V2(command_buffer);
        if (topic_searched == NULL) {
            return NULL;
        } else if (strcmp(topic_searched, topic) == 0) {
            char command_sid_buffer[64];
            snprintf(command_sid_buffer, 64, "mqtt_sub.@topic[%i].section_id", topics_iteration);
            char * section_id = uci_get_config_entry_V2(command_sid_buffer);
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

char * uci_get_config_entry_V2 (char *path) {
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
            // printf("%s\n", ptr.o->v.string);
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

char * uci_get_config_entry(char *path, char *option) {
    static  struct uci_context * ctx = NULL;
    struct uci_package * pkg = NULL;  
    struct uci_element * e; 
    char * value;
    int got_value = 0;

    ctx = uci_alloc_context();
    if  (uci_load(ctx, path, &pkg) != UCI_OK)  {
        printf("Error encountered getting uci path: %s\n", path);
        goto cleanup;
    }
    
    uci_foreach_element(&pkg->sections, e)  {
        char * value_temp;
        struct uci_section * s = uci_to_section (e);
        value_temp = uci_lookup_option_string(ctx, s, option);
        if(value_temp != NULL) {
            value = strdup(value_temp);
            got_value = 1;
        }
    }
    uci_unload(ctx, pkg);
    cleanup:  
        uci_free_context(ctx);
        ctx = NULL;
        if(got_value == 1) {
            return value;
        } else {
            return NULL;
        }
}

list_t * uci_get_config_entry_list(char *path, char *option) {
    list_t *entry_list = list_new();

    static  struct uci_context * ctx = NULL;
    struct uci_package * pkg = NULL;  
    struct uci_element * e; 
    char * value;
    int got_value = 0;

    ctx = uci_alloc_context();
    if  (uci_load(ctx, path, &pkg) != UCI_OK)  {
        printf("Error encountered getting uci path: %s\n", path);
        goto cleanup;
    }
    
    uci_foreach_element(&pkg->sections, e)  {
        char * value_temp;
        struct uci_section * s = uci_to_section (e);
        value_temp = uci_lookup_option_string(ctx, s, option);
        if(value_temp != NULL) {
            value = strdup(value_temp);
            got_value = 1;
            list_rpush(entry_list, list_node_new(value));
        }
    }
    uci_unload(ctx, pkg);
    cleanup:  
        uci_free_context(ctx);
        ctx = NULL;
        if(got_value == 1) {
            return entry_list;
        } else {
            return NULL;
        }
}