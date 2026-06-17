cmake_minimum_required(VERSION 3.14)
project(Lab4_FSM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем компилятору, где лежат .hpp файлы
include_directories(src src/lab2_files)

# 1. Основная программа
add_executable(app_main main.cpp)

# 2. Подключение Google Test (скачивается автоматически)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# Для Windows/Mac это предотвращает конфликты рантайма
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 3. Исполняемый файл для тестов
# Сюда добавляем все .cpp файлы тестов, которые будем создавать
# Исполняемый файл для тестов (БЕЗ test_main.cpp)
add_executable( app_tests 
    tests/lazy_sequence_tests.cpp
    tests/ordinal_tests.cpp
    tests/streams_tests.cpp
)

# Линкуем GTest с нашим тестовым таргетом (это автоматически даст нам функцию main)
target_link_libraries(app_tests PRIVATE gtest_main)

# Включаем интеграцию тестов в CTest
include(GoogleTest)
gtest_discover_tests(app_tests)

