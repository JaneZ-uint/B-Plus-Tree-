cmake_minimum_required(VERSION 3.28)
project(BPlusTree)

set(CMAKE_CXX_STANDARD 20)

add_executable(code main.cpp
        BPT.h
        vector.h
        exceptions.h)
