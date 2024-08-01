cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(text_analyzer)
enable_testing()

add_subdirectory(TextAnalyzer)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} TextAnalyzer)