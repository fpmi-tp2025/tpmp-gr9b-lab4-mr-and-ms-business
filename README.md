# Цветочная оранжерея

## Description
Проект представляет собой консольное приложение на языке C, демонстрирующее работу с базой данных «Цветочная оранжерея» через SQLite. Приложение умеет выполнять основные операции: создание, чтение, обновление и удаление данных (CRUD), а также содержит несколько аналитических запросов, связанных с заказами и цветочными композициями. В проекте реализованы юнит-тесты с помощью GoogleTest, которые проверяют корректность работы функций по взаимодействию с базой данных и бизнес-логики.

## Installation
1. **Склонируйте** репозиторий к себе локально:
   ```bash
   git clone https://github.com/fpmi-tp2025/tpmp-gr9b-lab4-mr-and-ms-business.git
   ```
2. **Установите** CMake (версии 3.10 или выше) и библиотеки GoogleTest и SQLite (если они не установлены в системе).  

3. **Соберите** и **запустите** проект с помощью CMake — каталог сборки создастся автоматически.

## Usage
- **Запуск приложения:**
  ```bash
  ./orangerie_app
  ```
   
- **Запуск тестов:**
  ```bash
  ./runTests
  ```
  Либо с помощью CTest:
  ```bash
  ctest --verbose
  ```
- **Оценка покрытия кода:**  
  При конфигурации добавьте флаг `-DCODE_COVERAGE=ON`, затем после запуска тестов используйте lcov/gcov:
  ```bash
  lcov --capture --directory . --output-file coverage.info
  lcov --list coverage.info
  ```

## Contributing
- **Авторы**: Беглецов Герман,Давидович Александр
- **Роли**:
Каждый участник команды принимали активное участие на всех этапах разработки: совместно реализовывали логику работы с базой данных в модуле db.c и обработку запросов в модуле queries.c. Кроме того, каждый вносил вклад в написание юнит-тестов, настройку интеграции с CMake и добавление инструментов покрытия кода. Такой подход позволил эффективно распределять задачи и обеспечил высокое качество итогового решения.