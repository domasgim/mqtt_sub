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
#include <curl/curl.h>
#include <ctype.h>

#include "list.h"
#include "cJSON.h"

#define LOG_PATH "/etc/"
#define EMAIL_PATH "/tmp/email_text.txt"

/**
 * @brief Get UCI configuration entry object
 * 
 * @param path UCI configuration file name
 * @param option UCI configuration entry name
 * @return char* option
 */
char * get_config_entry(char *path, char *option);

/**
 * @brief Get UCI configuration option
 * 
 * @param path UCI configuration option path
 * @return char* option
 */
char * uci_get_config_entry_V2 (char *path);

/**
 * @brief Get UCI configuration entry object list
 * 
 * @param path UCI configuration file name
 * @param option UCI configuration entry name
 * @return list_t* entry list
 */
list_t * get_config_entry_list(char *path, char *option);

/**
 * @brief Find the user_group UCI configuration section iteration according to the specified email group. 
 * Used to later specify this number while getting user_group information
 * 
 * @param email_group Email group's name which will be used to search in user_group UCI configuration
 * @return int iteration number, -1 for error
 */
int uci_get_user_group_iteration(char * email_group);

/**
 * @brief Find the section ID of a specifed topic. Used to later read the topi's event information
 * 
 * @param topic topic name
 * @return char* section ID or NULL if nothing was found
 */
char * uci_get_topic_section_id(char * topic);

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
 * @brief Send an email notification when MQTT event occurs
 * 
 * @param email_group Email group specified by configuring MQTT subscriber event options. Linked to user_group UCI configuration
 * @param recipient_email Recipient's email address
 * @param topic MQTT subsciber topic name
 * @param json_name JSON variable name specified by configuring event's options
 * @param json_value JSON variable contents specified by configuring event's options
 * @param operator Operator specified by configuring event's options. Used to compare JSON value to another value
 * @param comparison_val The value used to compare JSON value to
 * @return int return code
 */
int curl_send_email(char * email_group, char * recipient_email, char * topic, 
    char * json_name, cJSON * json_value, char * operator, char * comparison_val);

/**
 * @brief Process event information of a specified topic. It's a bit of a sloppy function which could be improved on
 * but the premise of it is to read UCI configuration files of mqtt_sub which stores the information about events
 * and user_groups which stores the information about email settings. If everything checks out, we send out an email 
 * to the specifed user according to the UCI configuration.
 * 
 * @param section_id The section ID of the topic. Each topic has this information in the UCI configuration
 * @param topic MQTT topic name
 * @param payload MQTT payload (message contents)
 */
int process_events(char * section_id, char * topic, char * payload);

/**
 * @brief Create and write email body to a specified file
 * 
 * @param sender_address Sender's email address
 * @param recipient_address Recipient's email address
 * @param topic MQTT event's topic name
 * @param json_name MQTT event's tracked JSON variable name
 * @param json_value MQTT event's tracked JSON variable value
 * @param comparison_val Value used to compare JSON variable to
 */
int create_email_file(char * sender_address, char * recipient_address, char * topic, char * json_name, char * json_value, char * operator, char * comparison_val);

/**
 * @brief Check if a string contains only numbers
 * @return 1 - true; 0 - false
 * 
 */
int digits_only(char * s);

#endif