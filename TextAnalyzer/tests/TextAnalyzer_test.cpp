#include <TextAnalyzer.hpp>
#include <gtest/gtest.h>

TEST(TextAnalyzerTest, TestSimpleText)
{
    const auto expected = 11;
    //std::filesystem::path path_to_test_file = std::filesystem::current_path
    tean::text_analyzer text1("C:/Users/vasilyev.an/Desktop/file.txt");
    ASSERT_TRUE(text1.is_valid());
    for(int i = 1; i <=100; i++)
    {
        const auto actual = text1.find_substr("as", i);
        EXPECT_EQ(expected, actual);
    }
}

TEST(TextAnalyzerTest, TestHardText)
{
    const auto expected = 187;
    //std::filesystem::path path_to_test_file = std::filesystem::current_path
    tean::text_analyzer text1("C:/Users/vasilyev.an/Desktop/file1.txt");
    ASSERT_TRUE(text1.is_valid());
    for(int i = 1; i <=100; i++)
    {
        const auto actual = text1.find_substr("as", i);
        EXPECT_EQ(expected, actual);
    }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv); 
  return RUN_ALL_TESTS();
}