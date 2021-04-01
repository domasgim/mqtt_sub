#include <uci.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <syslog.h>
#include <time.h>
#include <stdatomic.h>
#include <sqlite3.h>

#define LOG_PATH "/etc/"

static const char *delimiter = " ";
atomic_int interrupt = 0;

void sigHandler(int signo) {
	interrupt = 1;
}

char * show_config_entry(char *path)
{
    char * path_processed = strdup(path); /* Reikes free */

    struct uci_context *c;
    struct uci_ptr ptr;

    c = uci_alloc_context();
    int rc = uci_lookup_ptr (c, &ptr, path_processed, true);
    //if (uci_lookup_ptr (c, &ptr, path_processed, true) != UCI_OK)
    if(rc != UCI_OK)
    {
        fprintf(stderr, "Error: could not read UCI value at %s\n", path);
        exit(1);
    }

    if(ptr.o->type == UCI_TYPE_STRING) {
        return ptr.o->v.string;
    } 
    // switch(ptr.o->type) {
    // case UCI_TYPE_STRING:
    //     return ptr.o->v.string;

    return NULL;
    //uci_free_context(c); /* Reiketu free sutvarkyt */
}

char * show_config_entry_V2(char *path, char *option) {
    char * path_processed = strdup(path);
    char *option_processed = strdup(option);

    struct uci_context* ctx = uci_alloc_context();

    if (!ctx) {
        printf("failed to alloc uci ctx\n");
        return NULL;
    }

    struct uci_ptr ptr;

    if (uci_lookup_ptr(ctx, &ptr, path_processed, true) != UCI_OK || !ptr.s) {
        printf("failed to find the specified section\n");
        return 1;
    }

    char * option_val = uci_lookup_option(ctx, ptr.s, option);

    // printf("Value: %s\n", ptr.o->v.string);
     printf("Value: %s\n", option_val);

    // char * result = ptr.value;

    return option_val;
}

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    char * request = show_config_entry("mqtt_sub.config.request");
	if(rc) {
		printf("Error - %d\n", rc);
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, request, 0);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
	printf("%s: %s\n", msg->topic, (char *) msg->payload);
    sqlite_insert(msg->topic, (char *) msg->payload);
    /* Write to logread but printf works too if we launch it with init.d */
    //openlog("mqtt_sub", LOG_PID, LOG_USER);
    //char * log_msg = "%s - %s\n", msg->topic, (char *) msg->payload;
    //syslog(LOG_INFO, "%s - %s\n", msg->topic, (char *) msg->payload);   /* Write to log */
    closelog();
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

int sqlite_insert(char * topic, char * message) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    sqlite3_stmt *stmt = NULL;

    /* Open database */
    rc = sqlite3_open("/log/mqtt_sub.db", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    //sql = "INSERT INTO RECEIVED_MESSAGES (date, topic, message) VALUES (CURRENT_TIMESTAMP, ?, ?);";
    sql = sqlite3_mprintf("INSERT INTO RECEIVED_MESSAGES (date, topic, message) VALUES (CURRENT_TIMESTAMP, '%s', '%s');", topic, message);
    printf("%s\n", sql);

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
    // char * host = show_config_entry("mqtt_sub.config.host");
    // printf("Host: %s\n", host);
    // char * port_str = show_config_entry("mqtt_sub.config.port");
    // printf("Port: %s\n", port_str);
    // char * topic = show_config_entry("mqtt_sub.config.request");
    // printf("Topic request: %s\n", topic);
    // char * use_tls = show_config_entry("mqtt_sub.config.use_tls_ssl");
    // printf("Use tls: %s\n", use_tls);
    // char * tls_version = show_config_entry("mqtt_sub.config.tls_version");
    // printf("TLS version: %s\n", tls_version);
    // char * ca_file = show_config_entry("mqtt_sub.config.ca_file");
    // if(ca_file == NULL) {
    //     fprintf(stderr, "Error\n");
    //     return -1;
    // }
    // printf("CA file: %s\n", ca_file);

    // char * cert_file = show_config_entry("mqtt_sub.config.cert_file");
    // if(cert_file == NULL) {
    //     fprintf(stderr, "Error\n");
    //     return -1;
    // }
    // printf("CERT file: %s\n", cert_file);
    // char * key_file = show_config_entry("mqtt_sub.config.key_file");
    // if(key_file == NULL) {
    //     fprintf(stderr, "Error\n");
    //     return -1;
    // }
    // printf("Key file: %s\n", key_file);


    char * host = show_config_entry_V2("mqtt_sub.config.host", "host");
    printf("Host: %s\n", host);

    char * psk = show_config_entry_V2("mqtt_sub.config.psppp", "psppp");
    printf("PSK: %s\n", psk);



    // signal(SIGINT, sigHandler);
	// signal(SIGTERM, sigHandler);

    // int rc, id=12;
    // char * host = show_config_entry("mqtt_sub.config.host");
    // char * port_str = show_config_entry("mqtt_sub.config.port");
    // char * use_tls = show_config_entry("mqtt_sub.config.use_tls_ssl");
    // char * use_username_pw = show_config_entry("mqtt_sub.config.use_username_pw");
    // printf("Use tls: %s\n", use_tls);

    // int port = (int) strtol(port_str, (char **)NULL, 10);
    // int tls_flag = (int) strtol(use_tls, (char **)NULL, 10);
    // int username_pw_flag = (int) strtol(use_username_pw, (char **)NULL, 10);

    // /* Start initialising mosquitto objects */
	// mosquitto_lib_init();
	// struct mosquitto *mosq;

	// mosq = mosquitto_new("mqtt_sub", true, &id);

    // if(username_pw_flag == 0) {
    //     char * username = show_config_entry("mqtt_sub.config.username");
    //     char * password = show_config_entry("mqtt_sub.config.password");
    //     rc = mosquitto_username_pw_set(mosq, username, password);

    //     if(rc) {
    //         fprintf(stderr, "Error setting username and password - %d\n", rc);
    //         exit(-1);
    //     } else {
    //         printf("Username and password set!\n");
    //     }
    // }

    // if (tls_flag == 1) {
    //     char * tls_type = show_config_entry("mqtt_sub.config.tls_type");

    //     if(tls_type == "cert") {
    //         printf("TLS cert mode\n");
    //         char * tls_version = show_config_entry("mqtt_sub.config.tls_version");
    //         char * ca_file = show_config_entry("mqtt_sub.config.ca_file");
    //         char * cert_file = show_config_entry("mqtt_sub.config.cert_file");
    //         char * key_file = show_config_entry("mqtt_sub.config.key_file");

    //         if(ca_file == NULL || cert_file == NULL || key_file == NULL) {
    //             fprintf(stderr, "Error getting certificate files\n");
    //             exit(-1);
    //         }
            
    //         printf("Setting TLS options\n");
    //         rc = mosquitto_tls_set(mosq, ca_file, NULL, cert_file, key_file, NULL);
    //         if(rc) {
    //             printf("Error setting TLS - %d\n", rc);
    //             return -1;
    //         } else {
    //             printf("TLS set!\n");
    //         }
    //     } else {
    //         printf("TLS psk mode\n");

    //     }
        
    // } else {
    //     printf("NO TLS mode\n");
    // }

	// mosquitto_connect_callback_set(mosq, on_connect);
	// mosquitto_message_callback_set(mosq, on_message);
	
	// rc = mosquitto_connect(mosq, host, port, 10);

    // if(rc) {
	// 	printf("Could not connect to Broker with return code %d\n", rc);
	// 	return -1;
	// }
    // mosquitto_loop_start(mosq);
    // while (!interrupt) {

    // }
	// mosquitto_loop_stop(mosq, true);

	// mosquitto_disconnect(mosq);
	// mosquitto_destroy(mosq);
	// mosquitto_lib_cleanup();
    return 0;
}