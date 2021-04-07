#include "mqtt_sub.h"

static int sqlite_callback(void *data, int argc, char **argv, char **azColName) {
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   
   for(i = 0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   
   printf("\n");
   return 0;
}

int sqlite3_insert(char * topic, char * message) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc = 0;
    char *sql;

    /* Open database */
    rc = sqlite3_open("/log/mqtt_sub.db", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    } 

    /* Create SQL statement */
    sql = sqlite3_mprintf("INSERT INTO RECEIVED_MESSAGES (date, topic, message) VALUES (CURRENT_TIMESTAMP, '%s', '%s');", topic, message);

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, sqlite_callback, 0, &zErrMsg);
    
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return rc;
    } 
    
    sqlite3_close(db);
    return 0;
}