cmake_minimum_required(VERSION 3.13)

include(GoogleTest)

set(This tests)

add_executable(${This} TextAnalyzer_test.cpp )
target_link_libraries(${This} PRIVATE TextAnalyzer gtest gtest_main gmock gmock_main)

add_test (
    NAME ${This}
    COMMAND ${This}
)

gtest_discover_tests(${This})