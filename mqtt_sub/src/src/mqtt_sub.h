#ifndef MQTT_SUB_H
#define MQTT_SUB_H

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
#include <curl/curl.h>
#include <ctype.h>

#include "uci_helper.h"
#include "curl_helper.h"
#include "mqtt_sub_sql.h"
#include "uci_option_list.h"

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
 * @brief Process individual event information and send search for a match with received message
 * 
 * @param events Event list
 * @param topic Received message topic name
 * @param payload Received message contents
 * @return int 
 */
int process_events(event_node_t *events, char * topic, char * payload);

/**
 * @brief Compare all string options against the event's tracked JSON value. If a condition is met, send an email
 * 
 * @param event Event struct containing all information about a specified event
 * @param json_item Json item to be compared against
 * @param topic Received message topic name
 * @return int 
 */
int compare_strings(event_t event, char *json_item, char *topic);

/**
 * @brief Compare all integer options against the event's tracked JSON value. If a condition is met, send an email
 * 
 * @param event Event struct containing all information about a specified event
 * @param json_item Json item to be compared against
 * @param topic Received message topic name
 * @return int 
 */
int compare_integers(event_t event, int json_item, char *topic);

/**
 * @brief Configure MQTT subscriber options (check for password/TLS settings)
 * 
 * @param mosq Mosquitto subscriber object
 * @return int 
 */
int mosq_setup(struct mosquitto * mosq);

/**
 * @brief Check if a string contains only numbers
 * @return 1 - true; 0 - false
 * 
 */
int digits_only(char * s);

#endif