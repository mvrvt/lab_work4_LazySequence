cmake_minimum_required(VERSION 3.14)
project(Lab4_FSM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем компилятору, где лежат .hpp файлы
include_directories(src src/lab2_files)

# 1. Основная программа
add_executable(app_main main.cpp)

# 2. Подключение Google Test (скачивается автоматически через CMake)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 3. Исполняемый файл для тестов
add_executable(app_tests 
    tests/lazy_sequence_tests.cpp
    tests/ordinal_tests.cpp
    tests/streams_tests.cpp
    tests/file_string_stream_tests.cpp
    tests/sliding_cache_tests.cpp
)

# Линкуем GTest с тестовым таргетом (автоматически дает функцию main для тестов)
target_link_libraries(app_tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(app_tests)
