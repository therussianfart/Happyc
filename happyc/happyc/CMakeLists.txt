cmake_minimum_required(VERSION 3.10)
project(happyc)

set(CMAKE_CXX_STANDARD 17)

add_executable(hc
    main.cpp
    lexer.cpp
    parser.cpp
    codegen.cpp
)
