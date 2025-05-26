#ifndef QUERIES_H
#define QUERIES_H

#include <sqlite3.h>


int compute_date_diff(const char* start_date, const char* end_date);


int query_total_received(sqlite3 *db, const char* start_date, const char* end_date);
int query_max_demand_composition(sqlite3 *db);
int query_order_count_by_urgency(sqlite3 *db);
int query_flowers_usage(sqlite3 *db, const char* start_date, const char* end_date);
int query_sales_by_composition(sqlite3 *db);
int insert_order_with_payment(sqlite3 *db, const char* order_date, const char* fulfillment_date, int composition_id, int quantity, int customer_id);
int update_flower_price(sqlite3 *db, int flower_id, double new_price);
int display_orders_by_date(sqlite3 *db, const char* date);

#endif // QUERIES_H
