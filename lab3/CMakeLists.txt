cmake_minimum_required(VERSION 3.14)
project(ppois_lab3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LAB3_ENABLE_COVERAGE "Собирать проект с инструментацией покрытия (gcov)" OFF)

# Флаги покрытия подключаются только к нашим целям, но не к GoogleTest.
add_library(lab3_coverage INTERFACE)
if(LAB3_ENABLE_COVERAGE)
    target_compile_options(lab3_coverage INTERFACE --coverage -O0 -g)
    target_link_options(lab3_coverage INTERFACE --coverage)
endif()

# Предметная область: только заголовки (header-only).
add_library(studis_domain INTERFACE)
target_include_directories(studis_domain INTERFACE include)
target_compile_features(studis_domain INTERFACE cxx_std_17)
target_link_libraries(studis_domain INTERFACE lab3_coverage)

# Консольный интерфейс.
add_library(lab3_cli STATIC cli/Menu.cpp cli/DemoData.cpp cli/StudentSystemMenu.cpp)
target_include_directories(lab3_cli PUBLIC cli)
target_compile_options(lab3_cli PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(lab3_cli PUBLIC studis_domain)

add_executable(lab3_app cli/main.cpp)
target_link_libraries(lab3_app PRIVATE lab3_cli)

# Модульные тесты (GoogleTest скачивается при конфигурации).
enable_testing()
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lab3_tests
    tests/ExceptionsTests.cpp
    tests/CommonTests.cpp
    tests/PeopleTests.cpp
    tests/GradesTests.cpp
    tests/StructureTests.cpp
    tests/CoursesTests.cpp
    tests/AssessmentTests.cpp
    tests/AdmissionTests.cpp
    tests/FinanceTests.cpp
    tests/LifeTests.cpp
    tests/SystemTests.cpp
    tests/CliTests.cpp)
target_include_directories(lab3_tests PRIVATE tests)
target_compile_options(lab3_tests PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(lab3_tests PRIVATE lab3_cli GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lab3_tests)
