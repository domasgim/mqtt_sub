#ifndef CURL_HELPER_H
#define CURL_HELPER_H

#include <stdio.h>
#include <curl/curl.h>
#include "uci_option_list.h"

#define LOG_PATH "/etc/"
#define EMAIL_PATH "/tmp/email_text.txt"

/**
 * @brief Send out an email notifying that a MQTT event has matched
 * 
 * @param event Event struct containing all information about the specified event
 * @param json_item Json variable that was received
 * @param topic MQTT message topic
 * @return int 
 */
extern int curl_send_email(event_t event, char *json_item, char *topic);

/**
 * @brief Create a email file object
 * 
 * @param event Event struct containing all information about the specified event
 * @param json_item Json variable that was received
 * @param topic MQTT message topic 
 * @return int 
 */
static int create_email_file(event_t event, char *json_item, char *topic);

#endif