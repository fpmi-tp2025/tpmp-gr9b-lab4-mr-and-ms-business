#ifndef DB_H
#define DB_H

#include <sqlite3.h>

sqlite3* open_database(const char *db_name);

void close_database(sqlite3 *db);

#endif // DB_H
