cmake_minimum_required(VERSION 3.10)
project(OrangerieApp)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(${PROJECT_SOURCE_DIR}/include)

find_library(SQLITE3_LIBRARY sqlite3)
if(NOT SQLITE3_LIBRARY)
  message(FATAL_ERROR "Не удалось найти библиотеку sqlite3")
endif()
message(STATUS "Найдена библиотека sqlite3: ${SQLITE3_LIBRARY}")

set(LIB_SOURCES
    src/db.c
    src/queries.c
    src/auth.c
)
add_library(OrangerieLib ${LIB_SOURCES})
target_link_libraries(OrangerieLib PUBLIC ${SQLITE3_LIBRARY})

add_executable(orangerie_app src/main.c)
target_link_libraries(orangerie_app PRIVATE OrangerieLib)

enable_testing()

set(TEST_SOURCES
    tests/test_db.cpp
    tests/test_queries.cpp
    tests/test_auth.cpp
)
add_executable(runTests ${TEST_SOURCES})
target_link_libraries(runTests PRIVATE OrangerieLib gtest_main pthread)

add_test(NAME runTests COMMAND runTests)
