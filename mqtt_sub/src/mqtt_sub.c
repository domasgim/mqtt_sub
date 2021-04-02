#include <uci.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <syslog.h>
#include <time.h>
#include <stdatomic.h>
#include <sqlite3.h>
#include <errno.h>
#include <string.h>

#define LOG_PATH "/etc/"

atomic_int interrupt = 0;

void sigHandler(int signo) {
	interrupt = 1;
}

/**
 * @brief Get UCI option value (does not consider sections)
 * 
 * @param path UCI configuration name
 * @param option UCI configuration option
 * @return char* Option contents
 */
char * show_config_entry(char *path, char *option) {
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
        // char * value;
        struct uci_section * s = uci_to_section (e);
        value_temp = uci_lookup_option_string(ctx, s, option);
        if(value_temp != NULL) {
            value = strdup(value_temp);
            got_value = 1;
            //printf("Value: %s\n", value);
        }
    }
    // printf("Value: %s\n", value);
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

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    char * request = show_config_entry("mqtt_sub", "request");
	if(rc) {
		printf("Error - %s\n", mosquitto_strerror(errno));
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, request, 0);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
	printf("%s: %s\n", msg->topic, (char *) msg->payload);
    sqlite_insert(msg->topic, (char *) msg->payload);
}

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   
   for(i = 0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   
   printf("\n");
   return 0;
}

/**
 * @brief Write a new entry to a database
 * 
 * @param topic MQTT topic name
 * @param message MQTT message
 * @return int 
 */
int sqlite_insert(char * topic, char * message) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;

    /* Open database */
    rc = sqlite3_open("/log/mqtt_sub.db", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    sql = sqlite3_mprintf("INSERT INTO RECEIVED_MESSAGES (date, topic, message) VALUES (CURRENT_TIMESTAMP, '%s', '%s');", topic, message);
    // printf("%s\n", sql);

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Records created successfully\n");
    }
    
    sqlite3_close(db);
    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

    int rc, id=12;
    char * host = show_config_entry("mqtt_sub", "host");
    char * port_str = show_config_entry("mqtt_sub", "port");
    char * use_tls = show_config_entry("mqtt_sub", "use_tls_ssl");
    char * use_username_pw = show_config_entry("mqtt_sub", "use_username_pw");

    int port = (int) strtol(port_str, (char **)NULL, 10);
    int tls_flag = (int) strtol(use_tls, (char **)NULL, 10);
    int username_pw_flag = (int) strtol(use_username_pw, (char **)NULL, 10);

    /* Start initialising mosquitto objects */
	mosquitto_lib_init();
	struct mosquitto *mosq;

	mosq = mosquitto_new("mqtt_sub", true, &id);

    if(username_pw_flag == 0) {
        char * username = show_config_entry("mqtt_sub", "username");
        char * password = show_config_entry("mqtt_sub", "password");
        rc = mosquitto_username_pw_set(mosq, username, password);

        if(rc) {
            fprintf(stderr, "Error setting username and password - %s\n", mosquitto_strerror(errno));
            return -1;
        } 
    }

    if (tls_flag == 1) {
        char * tls_type = show_config_entry("mqtt_sub", "tls_type");

        if(strcmp(tls_type, "crt") == 0) {
            char * tls_insecure = show_config_entry("mqtt_sub", "tls_insecure");
            int tls_insecure_flag = (int) strtol(tls_insecure, (char **)NULL, 10);

            char * ca_file = show_config_entry("mqtt_sub", "ca_file");
            char * cert_file = show_config_entry("mqtt_sub", "cert_file");
            char * key_file = show_config_entry("mqtt_sub", "key_file");

            if(ca_file == NULL || cert_file == NULL || key_file == NULL) {
                fprintf(stderr, "Error getting certificate files\n");
                return -1;
            }
            
            rc = mosquitto_tls_set(mosq, ca_file, NULL, cert_file, key_file, NULL);
            if(rc) {
                printf("Error setting TLS - %s\n", mosquitto_strerror(errno));
                return -1;
            } 

            if (tls_insecure_flag == 1) {
                rc = mosquitto_tls_insecure_set(mosq, 1);
                if(rc) {
                    fprintf(stderr, "Error setting insecure connection - %s\n", mosquitto_strerror(errno));
                    return -1;
                } 
            }

        } else if (strcmp(tls_type, "psk") == 0) {
            char * psk = show_config_entry("mqtt_sub", "psk");
            char * identity = show_config_entry("mqtt_sub", "identity");

            if(psk == NULL || identity == NULL) {
                fprintf(stderr, "Error - empty PSK values\n");
                return -1;
            }

            rc = mosquitto_tls_psk_set(mosq, psk, identity, NULL);
            if(rc) {
                fprintf(stderr, "Error setting PSK values - %s\n", mosquitto_strerror(errno));
                return -1;
            } 
        } 
    } 
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	
	rc = mosquitto_connect(mosq, host, port, 10);
    if(rc) {
		printf("Could not connect to Broker - %s\n", mosquitto_strerror(errno));
		return -1;
	}
    mosquitto_loop_start(mosq);
    while (!interrupt) {

    }
	mosquitto_loop_stop(mosq, true);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
    return 0;
}