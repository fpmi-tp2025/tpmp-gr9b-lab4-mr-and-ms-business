#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queries.h"
#include "db.h"

int compute_date_diff(const char* start_date, const char* end_date) {
    struct tm tm_start = {0}, tm_end = {0};
    sscanf(start_date, "%4d-%2d-%2d", &tm_start.tm_year, &tm_start.tm_mon, &tm_start.tm_mday);
    sscanf(end_date, "%4d-%2d-%2d", &tm_end.tm_year, &tm_end.tm_mon, &tm_end.tm_mday);
    tm_start.tm_year -= 1900;
    tm_end.tm_year -= 1900;
    tm_start.tm_mon -= 1;
    tm_end.tm_mon -= 1;
    time_t t_start = mktime(&tm_start);
    time_t t_end = mktime(&tm_end);
    double diff_seconds = difftime(t_end, t_start);
    int diff_days = diff_seconds / (60 * 60 * 24);
    return diff_days;
}

int query_total_received(sqlite3 *db, const char* start_date, const char* end_date) {
    const char *sql = "SELECT SUM(order_cost) FROM ORANGERIE_ORDER_PAYMENTS WHERE order_id IN (SELECT order_id FROM ORANGERIE_ORDERS WHERE order_date BETWEEN ? AND ?);";
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_text(stmt, 1, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end_date, -1, SQLITE_STATIC);
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        double total = sqlite3_column_double(stmt, 0);
        printf("Сумма полученных денег за период с %s по %s: %.2f\n", start_date, end_date, total);
    } else {
        printf("Нет данных за указанный период.\n");
    }
    sqlite3_finalize(stmt);
    return 0;
}

int query_max_demand_composition(sqlite3 *db) {
    const char *sql = "SELECT composition_id, SUM(quantity) as total_qty FROM ORANGERIE_ORDERS GROUP BY composition_id ORDER BY total_qty DESC LIMIT 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    int composition_id = -1;
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        composition_id = sqlite3_column_int(stmt, 0);
        int total_qty = sqlite3_column_int(stmt, 1);
        printf("Композиция с максимальным спросом (ID: %d), продано: %d\n", composition_id, total_qty);
    } else {
        printf("Нет данных по заказам.\n");
        sqlite3_finalize(stmt);
        return 1;
    }
    sqlite3_finalize(stmt);


    const char *sql_comp = "SELECT composition_name FROM ORANGERIE_COMPOSITIONS WHERE composition_id = ?;";
    rc = sqlite3_prepare_v2(db, sql_comp, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса композиции: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(stmt, 1, composition_id);
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *composition_name = sqlite3_column_text(stmt, 0);
        printf("Название композиции: %s\n", composition_name);
    }
    sqlite3_finalize(stmt);


    const char *sql_details = "SELECT f.flower_name, f.flower_sort, cf.quantity FROM ORANGERIE_COMPOSITION_FLOWERS cf JOIN ORANGERIE_FLOWERS f ON cf.flower_id = f.flower_id WHERE cf.composition_id = ?;";
    rc = sqlite3_prepare_v2(db, sql_details, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса состава композиции: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(stmt, 1, composition_id);
    printf("Состав композиции:\n");
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *flower_name = sqlite3_column_text(stmt, 0);
        const unsigned char *flower_sort = sqlite3_column_text(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        printf(" - %s (%s): %d шт.\n", flower_name, flower_sort, quantity);
    }
    sqlite3_finalize(stmt);
    return 0;
}


int query_order_count_by_urgency(sqlite3 *db) {
    const char *sql =
    "SELECT CASE "
    "    WHEN (julianday(fulfillment_date) - julianday(order_date)) <= 1 THEN 'Сверхсрочный' "
    "    WHEN (julianday(fulfillment_date) - julianday(order_date)) <= 2 THEN 'Срочный' "
    "    ELSE 'Обычный' END as urgency, COUNT(*) as count_orders "
    "FROM ORANGERIE_ORDERS "
    "GROUP BY urgency;";
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    printf("Заказы по срочности:\n");
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *urgency = sqlite3_column_text(stmt, 0);
        int count = sqlite3_column_int(stmt, 1);
        printf("%s: %d заказ(ов)\n", urgency, count);
    }
    sqlite3_finalize(stmt);
    return 0;
}

int query_flowers_usage(sqlite3 *db, const char* start_date, const char* end_date) {
    const char *sql =
    "SELECT f.flower_name, f.flower_sort, SUM(cf.quantity * o.quantity) as total_used "
    "FROM ORANGERIE_ORDERS o "
    "JOIN ORANGERIE_COMPOSITION_FLOWERS cf ON o.composition_id = cf.composition_id "
    "JOIN ORANGERIE_FLOWERS f ON cf.flower_id = f.flower_id "
    "WHERE o.order_date BETWEEN ? AND ? "
    "GROUP BY f.flower_name, f.flower_sort;";
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_text(stmt, 1, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end_date, -1, SQLITE_STATIC);
    printf("Использованные цветы с %s по %s:\n", start_date, end_date);
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *flower_name = sqlite3_column_text(stmt, 0);
        const unsigned char *flower_sort = sqlite3_column_text(stmt, 1);
        int total_used = sqlite3_column_int(stmt, 2);
        printf("%s (%s): %d шт.\n", flower_name, flower_sort, total_used);
    }
    sqlite3_finalize(stmt);
    return 0;
}

int query_sales_by_composition(sqlite3 *db) {
    const char *sql =
    "SELECT c.composition_name, SUM(o.quantity) as sold_count, SUM(p.order_cost) as total_received "
    "FROM ORANGERIE_ORDERS o "
    "JOIN ORANGERIE_COMPOSITIONS c ON o.composition_id = c.composition_id "
    "JOIN ORANGERIE_ORDER_PAYMENTS p ON o.order_id = p.order_id "
    "GROUP BY c.composition_name;";
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    printf("Продажи по композициям:\n");
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *composition_name = sqlite3_column_text(stmt, 0);
        int sold_count = sqlite3_column_int(stmt, 1);
        double total_received = sqlite3_column_double(stmt, 2);
        printf("Композиция: %s, Продано: %d, Сумма: %.2f\n", composition_name, sold_count, total_received);
    }
    sqlite3_finalize(stmt);
    return 0;
}

int insert_order_with_payment(sqlite3 *db, const char* order_date, const char* fulfillment_date, int composition_id, int quantity, int customer_id) {
    int rc;
    sqlite3_stmt *stmt;
    const char *sql_insert = "INSERT INTO ORANGERIE_ORDERS (order_date, fulfillment_date, composition_id, quantity, customer_id) VALUES (?, ?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса вставки заказа: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_text(stmt, 1, order_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, fulfillment_date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, composition_id);
    sqlite3_bind_int(stmt, 4, quantity);
    sqlite3_bind_int(stmt, 5, customer_id);
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) {
        fprintf(stderr, "Ошибка вставки заказа: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    sqlite3_finalize(stmt);
    int order_id = (int) sqlite3_last_insert_rowid(db);
    printf("Заказ добавлен с order_id: %d\n", order_id);

    const char *sql_cost = "SELECT SUM(f.price * cf.quantity) FROM ORANGERIE_COMPOSITION_FLOWERS cf JOIN ORANGERIE_FLOWERS f ON cf.flower_id = f.flower_id WHERE cf.composition_id = ?;";
    rc = sqlite3_prepare_v2(db, sql_cost, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса расчета стоимости: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(stmt, 1, composition_id);
    double base_cost = 0.0;
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        base_cost = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    base_cost *= quantity;

    int diff_days = compute_date_diff(order_date, fulfillment_date);
    double final_cost = base_cost;
    if(diff_days <= 1) {
        final_cost *= 1.25;
    } else if(diff_days <= 2) {
        final_cost *= 1.15;
    }
    const char *sql_payment = "INSERT INTO ORANGERIE_ORDER_PAYMENTS (order_id, order_cost) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(db, sql_payment, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса вставки платежа: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(stmt, 1, order_id);
    sqlite3_bind_double(stmt, 2, final_cost);
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) {
        fprintf(stderr, "Ошибка вставки платежа: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    sqlite3_finalize(stmt);
    printf("Стоимость заказа рассчитана: %.2f\n", final_cost);
    return 0;
}

int update_flower_price(sqlite3 *db, int flower_id, double new_price) {
    sqlite3_stmt *stmt;
    int rc;
    const char *sql_old_price = "SELECT price FROM ORANGERIE_FLOWERS WHERE flower_id = ?;";
    rc = sqlite3_prepare_v2(db, sql_old_price, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса для получения старой цены: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(stmt, 1, flower_id);
    double old_price = 0.0;
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        old_price = sqlite3_column_double(stmt, 0);
    } else {
        fprintf(stderr, "Цветок с ID %d не найден.\n", flower_id);
        sqlite3_finalize(stmt);
        return 1;
    }
    sqlite3_finalize(stmt);

    const char *sql_compositions = "SELECT composition_id, quantity FROM ORANGERIE_COMPOSITION_FLOWERS WHERE flower_id = ?;";
    rc = sqlite3_prepare_v2(db, sql_compositions, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса для получения композиций: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(stmt, 1, flower_id);
    int violation = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        int composition_id = sqlite3_column_int(stmt, 0);
        int flower_quantity = sqlite3_column_int(stmt, 1);
        const char *sql_old_cost = "SELECT SUM(f.price * cf.quantity) FROM ORANGERIE_COMPOSITION_FLOWERS cf JOIN ORANGERIE_FLOWERS f ON cf.flower_id = f.flower_id WHERE cf.composition_id = ?;";
        sqlite3_stmt *stmt2;
        rc = sqlite3_prepare_v2(db, sql_old_cost, -1, &stmt2, NULL);
        if(rc != SQLITE_OK) {
            fprintf(stderr, "Ошибка подготовки запроса для расчета старой стоимости композиции: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return 1;
        }
        sqlite3_bind_int(stmt2, 1, composition_id);
        double old_cost = 0.0;
        if(sqlite3_step(stmt2) == SQLITE_ROW) {
            old_cost = sqlite3_column_double(stmt2, 0);
        }
        sqlite3_finalize(stmt2);

        double new_cost = old_cost - (old_price * flower_quantity) + (new_price * flower_quantity);
        if(new_cost > old_cost * 1.10) {
            printf("Обновление цены нарушает правило для композиции ID %d: старая стоимость %.2f, новая %.2f\n", composition_id, old_cost, new_cost);
            violation = 1;
        }
    }
    sqlite3_finalize(stmt);
    if(violation) {
        printf("Обновление цены не выполнено из-за превышения лимита увеличения стоимости композиции.\n");
        return 1;
    }

    const char *sql_update = "UPDATE ORANGERIE_FLOWERS SET price = ? WHERE flower_id = ?;";
    rc = sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса обновления цены: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_double(stmt, 1, new_price);
    sqlite3_bind_int(stmt, 2, flower_id);
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) {
        fprintf(stderr, "Ошибка обновления цены: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    sqlite3_finalize(stmt);
    printf("Цена для цветка ID %d успешно обновлена до %.2f\n", flower_id, new_price);
    return 0;
}

int display_orders_by_date(sqlite3 *db, const char* date) {
    const char *sql =
    "SELECT o.order_id, o.order_date, o.fulfillment_date, c.composition_name, o.quantity, cust.name, p.order_cost "
    "FROM ORANGERIE_ORDERS o "
    "JOIN ORANGERIE_COMPOSITIONS c ON o.composition_id = c.composition_id "
    "JOIN ORANGERIE_CUSTOMERS cust ON o.customer_id = cust.customer_id "
    "JOIN ORANGERIE_ORDER_PAYMENTS p ON o.order_id = p.order_id "
    "WHERE o.order_date = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_text(stmt, 1, date, -1, SQLITE_STATIC);
    printf("Заказы на дату %s:\n", date);
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        int order_id = sqlite3_column_int(stmt, 0);
        const unsigned char *order_date = sqlite3_column_text(stmt, 1);
        const unsigned char *fulfillment_date = sqlite3_column_text(stmt, 2);
        const unsigned char *composition_name = sqlite3_column_text(stmt, 3);
        int quantity = sqlite3_column_int(stmt, 4);
        const unsigned char *customer_name = sqlite3_column_text(stmt, 5);
        double order_cost = sqlite3_column_double(stmt, 6);
        printf("Заказ ID: %d, Дата: %s, Выполнение: %s, Композиция: %s, Кол-во: %d, Покупатель: %s, Стоимость: %.2f\n",
               order_id, order_date, fulfillment_date, composition_name, quantity, customer_name, order_cost);
    }
    sqlite3_finalize(stmt);
    return 0;
}
