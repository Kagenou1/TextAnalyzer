cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

project(project)

set(SOURCE_FILES TextAnalyzer/Source.cpp)

add_executable(project ${SOURCE_FILES})
