#include "mqtt_sub_sql.h"

extern sqlite3 *sqlite3_open_database() {
    int rc = 0;
    sqlite3 *db;
    rc = sqlite3_open(DATABASE_PATH, &db);
    if(rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    } 
    return db;
}

static int sqlite_callback(void *data, int argc, char **argv, char **azColName) {
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   
   for(i = 0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   
   printf("\n");
   return 0;
}

extern int sqlite3_insert(sqlite3 * db, char * topic, char * message) {
    char *zErrMsg = 0;
    int rc = 0;
    char *sql;

    /* Create SQL statement */
    sql = sqlite3_mprintf("INSERT INTO RECEIVED_MESSAGES (date, topic, message) VALUES (CURRENT_TIMESTAMP, '%s', '%s');", topic, message);

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, sqlite_callback, 0, &zErrMsg);
    
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return rc;
    } 
    return 0;
}