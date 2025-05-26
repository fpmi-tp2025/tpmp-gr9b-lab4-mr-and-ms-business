#include <stdio.h>
#include <stdlib.h>
#include "db.h"
#include "queries.h"
#include "auth.h"

int main() {
    sqlite3 *db = open_database("orangerie.db");
    if (!db) {
        fprintf(stderr, "Не удалось открыть базу данных.\n");
        return 1;
    }
    
    if (!perform_authentication(db)) {
        printf("Доступ запрещен. Завершение работы.\n");
        close_database(db);
        return 1;
    }
    
    int choice;
    char start_date[11], end_date[11], date[11];
    while(1) {
        printf("\nМеню:\n");
        printf("1. Сумма полученных денег за период\n");
        printf("2. Композиция с максимальным спросом\n");
        printf("3. Количество заказов по срочности\n");
        printf("4. Количество использованных цветов по видам\n");
        printf("5. Проданные композиции и сумма по видам\n");
        printf("6. Внести новый заказ и рассчитать стоимость\n");
        printf("7. Обновить цену на цветок\n");
        printf("8. Вывести заказы на указанную дату\n");
        printf("0. Выход\n");
        printf("Выберите опцию: ");
        scanf("%d", &choice);

        if(choice == 0)
            break;

        switch(choice) {
            case 1:
                printf("Введите начальную дату (YYYY-MM-DD): ");
                scanf("%s", start_date);
                printf("Введите конечную дату (YYYY-MM-DD): ");
                scanf("%s", end_date);
                query_total_received(db, start_date, end_date);
                break;
            case 2:
                query_max_demand_composition(db);
                break;
            case 3:
                query_order_count_by_urgency(db);
                break;
            case 4:
                printf("Введите начальную дату (YYYY-MM-DD): ");
                scanf("%s", start_date);
                printf("Введите конечную дату (YYYY-MM-DD): ");
                scanf("%s", end_date);
                query_flowers_usage(db, start_date, end_date);
                break;
            case 5:
                query_sales_by_composition(db);
                break;
            case 6: {
                char order_date[11], fulfillment_date[11];
                int composition_id, quantity, customer_id;
                printf("Введите дату заказа (YYYY-MM-DD): ");
                scanf("%s", order_date);
                printf("Введите дату выполнения заказа (YYYY-MM-DD): ");
                scanf("%s", fulfillment_date);
                printf("Введите ID композиции: ");
                scanf("%d", &composition_id);
                printf("Введите количество: ");
                scanf("%d", &quantity);
                printf("Введите ID покупателя: ");
                scanf("%d", &customer_id);
                insert_order_with_payment(db, order_date, fulfillment_date, composition_id, quantity, customer_id);
                break;
            }
            case 7: {
                int flower_id;
                double new_price;
                printf("Введите ID цветка: ");
                scanf("%d", &flower_id);
                printf("Введите новую цену: ");
                scanf("%lf", &new_price);
                update_flower_price(db, flower_id, new_price);
                break;
            }
            case 8:
                printf("Введите дату (YYYY-MM-DD): ");
                scanf("%s", date);
                display_orders_by_date(db, date);
                break;
            default:
                printf("Неверная опция.\n");
        }
    }

    close_database(db);
    return 0;
}
