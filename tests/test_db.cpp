#include <gtest/gtest.h>
extern "C" {
#include "db.h"
}

TEST(DBTest, OpenDatabaseReturnsValidPointer) {
    sqlite3* db = open_database(":memory:");
    EXPECT_NE(db, nullptr);
    close_database(db);
}

TEST(DBTest, MultipleDatabasesOpenIndependently) {
    sqlite3* db1 = open_database(":memory:");
    sqlite3* db2 = open_database(":memory:");
    EXPECT_NE(db1, nullptr);
    EXPECT_NE(db2, nullptr);
    close_database(db1);
    close_database(db2);
}

TEST(DBTest, RepeatedOpenCloseCycle) {
    for (int i = 0; i < 3; i++) {
        sqlite3* db = open_database(":memory:");
        EXPECT_NE(db, nullptr);
        close_database(db);
    }
}
