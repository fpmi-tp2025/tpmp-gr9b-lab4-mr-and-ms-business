#ifndef AUTH_H
#define AUTH_H

#include <sqlite3.h>

int authenticate_user(sqlite3 *db);

int register_user(sqlite3 *db);

int perform_authentication(sqlite3 *db);

#endif // AUTH_H
