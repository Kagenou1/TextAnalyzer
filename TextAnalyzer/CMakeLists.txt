cmake_minimum_required(VERSION 3.14)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_library(TextAnalyzer ${SOURCES})
target_include_directories(TextAnalyzer PUBLIC include)
add_subdirectory(tests)