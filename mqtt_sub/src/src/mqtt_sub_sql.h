#ifndef MQTT_SUB_SQL_H
#define MQTT_SUB_SQL_H

#include <sqlite3.h>
#include <stdio.h>

#define DATABASE_PATH "/log/mqtt_sub.db"

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
extern int sqlite3_insert(sqlite3 * db, char * topic, char * message);

/**
 * @brief Open a database for logging events
 * 
 * @return sqlite3* 
 */
extern sqlite3 *sqlite3_open_database();

#endif