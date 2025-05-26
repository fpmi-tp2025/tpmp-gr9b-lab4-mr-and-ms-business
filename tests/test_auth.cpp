#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
extern "C" {
    #include "auth.h"
    #include "db.h"
}


FILE* create_input_stream(const char* input) {
    size_t len = strlen(input);
    char* buffer = new char[len + 1];
    strcpy(buffer, input);
    FILE* stream = fmemopen(buffer, len + 1, "r");
    return stream;
}

TEST(AuthTest, RegisterNewUserWorks) {
    sqlite3* db = open_database(":memory:");
    ASSERT_NE(db, nullptr);
    char* errMsg = nullptr;
    const char* create_sql = "CREATE TABLE ORANGERIE_USERS ("
                             " user_id INTEGER PRIMARY KEY, "
                             " username TEXT, "
                             " password TEXT);";
    ASSERT_EQ(sqlite3_exec(db, create_sql, nullptr, nullptr, &errMsg), SQLITE_OK) << (errMsg ? errMsg : "");
    
    const char* input = "testuser testpass\n"; 
    FILE* inputStream = create_input_stream(input);
    FILE* old_stdin = stdin;
    stdin = inputStream;
    
    int reg_result = register_user(db);
    
    stdin = old_stdin;
    fclose(inputStream);
    
    EXPECT_EQ(reg_result, 1);
    
    const char* check_sql = "SELECT COUNT(*) FROM ORANGERIE_USERS WHERE username = 'testuser';";
    sqlite3_stmt* stmt = nullptr;
    ASSERT_EQ(sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr), SQLITE_OK);
    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    EXPECT_EQ(count, 1);
    
    close_database(db);
}

TEST(AuthTest, AuthenticateExistingUserWorks) {
    sqlite3* db = open_database(":memory:");
    ASSERT_NE(db, nullptr);
    char* errMsg = nullptr;
    const char* create_sql = "CREATE TABLE ORANGERIE_USERS ("
                             " user_id INTEGER PRIMARY KEY, "
                             " username TEXT, "
                             " password TEXT);";
    ASSERT_EQ(sqlite3_exec(db, create_sql, nullptr, nullptr, &errMsg), SQLITE_OK) << (errMsg ? errMsg : "");
    const char* insert_sql = "INSERT INTO ORANGERIE_USERS (username, password) VALUES ('existing', 'password123');";
    ASSERT_EQ(sqlite3_exec(db, insert_sql, nullptr, nullptr, &errMsg), SQLITE_OK) << (errMsg ? errMsg : "");
    
    const char* input = "existing password123\n";
    FILE* inputStream = create_input_stream(input);
    FILE* old_stdin = stdin;
    stdin = inputStream;
    
    int auth_result = authenticate_user(db);
    
    stdin = old_stdin;
    fclose(inputStream);
    
    EXPECT_EQ(auth_result, 1);
    
    close_database(db);
}

TEST(AuthTest, PerformAuthenticationRegisterFlowWorks) {
    sqlite3* db = open_database(":memory:");
    ASSERT_NE(db, nullptr);
    char* errMsg = nullptr;
    const char* create_sql = "CREATE TABLE ORANGERIE_USERS ("
                             " user_id INTEGER PRIMARY KEY, "
                             " username TEXT, "
                             " password TEXT);";
    ASSERT_EQ(sqlite3_exec(db, create_sql, nullptr, nullptr, &errMsg), SQLITE_OK) << (errMsg ? errMsg : "");
    
    const char* input = "2\nnewuser\nnewpass\nnewuser\nnewpass\n";
    FILE* inputStream = create_input_stream(input);
    FILE* old_stdin = stdin;
    stdin = inputStream;
    
    int result = perform_authentication(db);
    
    stdin = old_stdin;
    fclose(inputStream);
    
    EXPECT_EQ(result, 1);
    
    const char* check_sql = "SELECT COUNT(*) FROM ORANGERIE_USERS WHERE username = 'newuser';";
    sqlite3_stmt* stmt = nullptr;
    ASSERT_EQ(sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr), SQLITE_OK);
    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    EXPECT_EQ(count, 1);
    
    close_database(db);
}

