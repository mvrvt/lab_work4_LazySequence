cmake_minimum_required( VERSION 3.10 )
project( Lab4_FSM )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

add_executable( Lab4_FSM
    main.cpp    
    src/Generator.hpp
    src/InputStream.hpp
    src/FSM.hpp
    src/LazySequence
)

# Указание компилятору искать заголовочные файлы в папке src
target_include_directories( Lab4_FSM PRIVATE src )
