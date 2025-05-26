#include <stdio.h>
#include "db.h"
#include <sqlite3.h>


sqlite3* open_database(const char *db_name) {
    sqlite3 *db;
    int rc = sqlite3_open(db_name, &db);
    if(rc) {
        fprintf(stderr, "Ошибка открытия базы данных: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}


void close_database(sqlite3 *db) {
    sqlite3_close(db);
}
