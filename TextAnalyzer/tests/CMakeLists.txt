add_executable(
  basic_test
  TextAnalyzer_test.cpp
)
target_link_libraries(
    basic_test
  GTest::gtest_main
  TextAnalyzer
)

include(GoogleTest)
gtest_discover_tests(basic_test)