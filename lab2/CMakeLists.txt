cmake_minimum_required(VERSION 3.14)
project(ppois_lab2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LAB2_ENABLE_COVERAGE "Собирать проект с инструментацией покрытия (gcov)" OFF)

# Флаги покрытия подключаются только к нашим целям, но не к GoogleTest.
add_library(lab2_coverage INTERFACE)
if(LAB2_ENABLE_COVERAGE)
    target_compile_options(lab2_coverage INTERFACE --coverage -O0 -g)
    target_link_options(lab2_coverage INTERFACE --coverage)
endif()

# Предметная область: только заголовки (header-only).
add_library(autoshop_domain INTERFACE)
target_include_directories(autoshop_domain INTERFACE include)
target_compile_features(autoshop_domain INTERFACE cxx_std_17)
target_link_libraries(autoshop_domain INTERFACE lab2_coverage)

# Консольный интерфейс.
add_library(lab2_cli STATIC cli/Menu.cpp cli/DemoData.cpp cli/AutoShopMenu.cpp)
target_include_directories(lab2_cli PUBLIC cli)
target_compile_options(lab2_cli PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(lab2_cli PUBLIC autoshop_domain)

add_executable(lab2_app cli/main.cpp)
target_link_libraries(lab2_app PRIVATE lab2_cli)

# Модульные тесты (GoogleTest скачивается при конфигурации).
enable_testing()
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lab2_tests
    tests/ExceptionsTests.cpp
    tests/CommonTests.cpp
    tests/ServicesTests.cpp
    tests/VehicleTests.cpp
    tests/PartsTests.cpp
    tests/PeopleTests.cpp
    tests/OperationsTests.cpp
    tests/BillingTests.cpp
    tests/AutoShopTests.cpp
    tests/CliTests.cpp)
target_include_directories(lab2_tests PRIVATE tests)
target_compile_options(lab2_tests PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(lab2_tests PRIVATE lab2_cli GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lab2_tests)
