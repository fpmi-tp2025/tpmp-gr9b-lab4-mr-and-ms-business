#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

int register_user(sqlite3 *db) {
    char username[256];
    char password[256];

    while (1) {
        printf("\nРегистрация нового пользователя:\n");
        printf("Введите желаемое имя пользователя: ");
        if (scanf("%255s", username) != 1) {
            fprintf(stderr, "Ошибка чтения имени пользователя\n");
            return 0;
        }
        
        const char *check_sql = "SELECT user_id FROM ORANGERIE_USERS WHERE username = ?;";
        sqlite3_stmt *check_stmt = NULL;
        if (sqlite3_prepare_v2(db, check_sql, -1, &check_stmt, NULL) != SQLITE_OK) {
            fprintf(stderr, "Ошибка подготовки запроса для проверки пользователя: %s\n", sqlite3_errmsg(db));
            return 0;
        }
        sqlite3_bind_text(check_stmt, 1, username, -1, SQLITE_STATIC);
        int rc = sqlite3_step(check_stmt);
        int user_exists = (rc == SQLITE_ROW);
        sqlite3_finalize(check_stmt);
        
        if (user_exists) {
            printf("Пользователь с именем '%s' уже существует. Попробуйте ввести другое имя.\n", username);
            continue;
        }
        break;
    }

    printf("Введите желаемый пароль: ");
    if (scanf("%255s", password) != 1) {
        fprintf(stderr, "Ошибка чтения пароля\n");
        return 0;
    }

    const char *sql = "INSERT INTO ORANGERIE_USERS (username, password) VALUES (?, ?);";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса для регистрации: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    int rc_insert = sqlite3_step(stmt);
    if (rc_insert != SQLITE_DONE) {
        fprintf(stderr, "Ошибка регистрации: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    printf("Регистрация прошла успешно! Теперь вы можете авторизоваться.\n");
    return 1;
}


int authenticate_user(sqlite3 *db) {
    char username[256];
    char password[256];

    printf("Введите имя пользователя: ");
    if (scanf("%255s", username) != 1) {
        fprintf(stderr, "Ошибка чтения имени пользователя\n");
        return 0;
    }
    printf("Введите пароль: ");
    if (scanf("%255s", password) != 1) {
        fprintf(stderr, "Ошибка чтения пароля\n");
        return 0;
    }

    const char *sql = "SELECT user_id FROM ORANGERIE_USERS WHERE username = ? AND password = ?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса для аутентификации: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    int authenticated = 0;
    if (rc == SQLITE_ROW) {
        authenticated = 1;
        printf("Аутентификация успешна! Добро пожаловать, %s.\n", username);
    } else {
        printf("Неверное имя пользователя или пароль.\n");
    }
    
    sqlite3_finalize(stmt);
    return authenticated;
}

int perform_authentication(sqlite3 *db) {
    int choice = 0;
    printf("\nВыберите действие:\n");
    printf("1. Войти\n");
    printf("2. Зарегистрироваться\n");
    printf("Ваш выбор: ");
    if (scanf("%d", &choice) != 1) {
        fprintf(stderr, "Ошибка ввода\n");
        return 0;
    }
    
    if (choice == 1) {
        return authenticate_user(db);
    } else if (choice == 2) {
        if (register_user(db)) {
            return authenticate_user(db);
        } else {
            return 0;
        }
    } else {
        printf("Неверный выбор, попробуйте снова.\n");
        return perform_authentication(db);
    }
}
