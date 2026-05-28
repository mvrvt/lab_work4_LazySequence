cmake_minimum_required(VERSION 3.10)
project(Lab4_FSM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем компилятору, где лежат наши .hpp файлы
include_directories(src src/lab2_files)

# Основная программа
add_executable(app_main main.cpp)

# Программа для модульных тестов
#add_executable(app_tests tests/test_main.cpp)
