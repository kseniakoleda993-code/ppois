cmake_minimum_required(VERSION 3.14)
project(ppois_lab1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LAB1_ENABLE_COVERAGE "Собирать проект с инструментацией покрытия (gcov)" OFF)

# Флаги покрытия подключаются только к нашим целям, но не к GoogleTest.
add_library(lab1_coverage INTERFACE)
if(LAB1_ENABLE_COVERAGE)
    target_compile_options(lab1_coverage INTERFACE --coverage -O0 -g)
    target_link_options(lab1_coverage INTERFACE --coverage)
endif()

# Классы предметной области (ничего не знают о консоли).
add_library(lab1_core STATIC
    src/Point3D.cpp
    src/StreamParsing.cpp
    src/Vector3D.cpp
    src/BigInteger.cpp)
target_include_directories(lab1_core PUBLIC include)

# Консольный интерфейс (зависит от lab1_core, но не наоборот).
add_library(lab1_cli STATIC
    cli/Menu.cpp
    cli/VectorMenu.cpp
    cli/BigIntegerMenu.cpp)
target_include_directories(lab1_cli PUBLIC cli)

if(MSVC)
    target_compile_options(lab1_core PRIVATE /W4 /utf-8)
    target_compile_options(lab1_cli PRIVATE /W4 /utf-8)
else()
    target_compile_options(lab1_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(lab1_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(lab1_core PUBLIC lab1_coverage)
target_link_libraries(lab1_cli PUBLIC lab1_core)

add_executable(lab1_app cli/main.cpp)
target_link_libraries(lab1_app PRIVATE lab1_cli)

# Модульные тесты (GoogleTest скачивается при конфигурации).
enable_testing()
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lab1_tests
    tests/Point3DTests.cpp
    tests/Vector3DTests.cpp
    tests/BigIntegerTests.cpp
    tests/CliTests.cpp)
target_link_libraries(lab1_tests PRIVATE lab1_cli GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lab1_tests)
