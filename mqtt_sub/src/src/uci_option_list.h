#ifndef UCI_OPTION_LIST_H
#define UCI_OPTION_LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>

struct event {
	char * json_val;
	char * val_type;
	char * comparison_val;
	char * email_group;
	char * recip_email;
    char * operator_str;
	enum operator {
        EQUAL, NOT_EQUAL, MORE, LESS, MORE_EQ, LESS_EQ,
    } operator;

    enum val_type_enum {
        STRING, INT,
    } val_type_enum;
    
	char * secure_conn;
    char * smtp_ip;
    char * smtp_port;
    char * username;
    char * password;
    char * senderemail;
};

struct topic {
    char * topic_name;
    char * section_id;
    int qos;
    struct event_node *events;
};

struct topic_node {
    struct topic topic;
    struct topic_node *next;
};

struct event_node {
    struct event event;
    struct event_node *next;
};

struct mqtt_options {
    sqlite3 *sqlite3_db;
    struct topic_node *topics;
};

typedef struct event event_t;
typedef struct topic topic_t;
typedef struct topic_node topic_node_t;
typedef struct event_node event_node_t;
typedef struct mqtt_options mqtt_options_t;

/**
 * @brief Create new linked list node
 * 
 * @param topic
 * @return topic_node_t* 
 */
topic_node_t *topic_create_new_node(topic_t topic);

/**
 * @brief Insert a new node at the start of the list
 * 
 * @param head 
 * @param node_to_insert 
 * @return topic_node_t* 
 */
topic_node_t *topic_insert_at_head(topic_node_t **head, topic_node_t *node_to_insert);

/**
 * @brief Create new linked list node
 * 
 * @param event 
 * @return event_node_t* 
 */
event_node_t *event_create_new_node(event_t event);

/**
 * @brief Insert a new node at the start of the list
 * 
 * @param head 
 * @param node_to_insert 
 * @return event_node_t* 
 */
event_node_t *event_insert_at_head(event_node_t **head, event_node_t *node_to_insert);

/**
 * @brief Check if an event is empty
 * 
 * @param event 
 * @return int 
 */
int event_empty(event_t event);

/**
 * @brief Read UCI values of a topic and return a struct
 * 
 * @param topic_iteration 
 * @return topic_t 
 */
topic_t uci_get_topic_options(int topic_iteration);

/**
 * @brief Read UCI value of an event and user group options (for sending email)
 * 
 * @param section_iteration 
 * @param section_id 
 * @return event_t 
 */
event_t uci_get_event_options(int section_iteration, char * section_id);

/**
 * @brief Get a list of all topics and corresponding events
 * 
 * @return topic_node_t* 
 */
topic_node_t * uci_get_topic_list();

#endif