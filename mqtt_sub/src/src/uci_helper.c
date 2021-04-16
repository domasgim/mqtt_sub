#include "mqtt_sub.h"

char * get_config_entry(char *path, char *option) {
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

list_t * get_config_entry_list(char *path, char *option) {
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