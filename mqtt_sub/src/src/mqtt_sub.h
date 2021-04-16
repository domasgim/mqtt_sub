#ifndef MQTT_SUB_H
#define MQTT_SUB_H

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

#include "list.h"

#define LOG_PATH "/etc/"

/**
 * @brief Get UCI configuration entry object
 * 
 * @param path UCI configuration file name
 * @param option UCI configuration entry name
 * @return char* entry contents
 */
char * get_config_entry(char *path, char *option);

/**
 * @brief Get UCI configuration entry object list
 * 
 * @param path UCI configuration file name
 * @param option UCI configuration entry name
 * @return list_t* entry list
 */
list_t * get_config_entry_list(char *path, char *option);

/**
 * @brief Callback called when the client receives a CONNACK message from the broker.
 * 
 * @param mosq
 * @param obj 
 * @param reason_code 
 */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code);

/**
 * @brief Callback called when the client receives a message.
 * 
 * @param mosq 
 * @param obj 
 * @param msg 
 */
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

/**
 * @brief Callback called when the broker sends a SUBACK in response to a SUBSCRIBE.
 * 
 * @param mosq 
 * @param obj 
 * @param mid 
 * @param qos_count 
 * @param granted_qos 
 */
void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);

/**
 * @brief Callback called during sql query execution
 * 
 * @param data 
 * @param argc 
 * @param argv 
 * @param azColName 
 * @return int 
 */
static int sqlite_callback(void *data, int argc, char **argv, char **azColName);

/**
 * @brief Insert mqtt message contents to sqlite3 database
 * 
 * @param topic received mqtt topic name 
 * @param message received mqtt message contents
 * @return int return code
 */
int sqlite3_insert(sqlite3 * db, char * topic, char * message);

/**
 * @brief Set mosquitto subscriber's username and password if appropriate UCI options are set
 * 
 * @param mosq mosquitto subscriber object
 * @return int return code
 */
int mosq_set_username_pw(struct mosquitto * mosq);

/**
 * @brief Set mosquitto subscriber's TLS certificate options if appropriate UCI options are set
 * 
 * @param mosq mosquitto subscriber object
 * @return int return code
 */
int mosq_set_tls(struct mosquitto * mosq);

/**
 * @brief Set mosquitto subscriber's password, TLS and callbacks
 * 
 * @param mosq mosquitto subscriber object
 * @return int return code
 */
int mosq_setup(struct mosquitto * mosq);

#endif