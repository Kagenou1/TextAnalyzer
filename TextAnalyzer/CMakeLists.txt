cmake_minimum_required(VERSION 3.14)

set(SOURCES 
    src/TextAnalyzer.cpp
)
add_library(TextAnalyzer SHARED ${SOURCES})
target_include_directories(TextAnalyzer INTERFACE include)