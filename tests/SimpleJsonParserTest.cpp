#include <gtest/gtest.h>
#include "../client/SimpleJsonParser.h"

TEST(SimpleJsonParserTest, TestParsesValidJson) {
    SimpleJsonParser parser;
    const std::string json = R"({"type":"greeting","message":"Welcome"})";
    std::map<std::string, std::string> result;

    ASSERT_TRUE(parser.parse(json, result));
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result["type"], "greeting");
    ASSERT_EQ(result["message"], "Welcome");
}

TEST(SimpleJsonParserTest, TestReturnsFalseForInvalidJson) {
    SimpleJsonParser parser;
    const std::string json = "invalid{json";
    std::map<std::string, std::string> result;

    ASSERT_FALSE(parser.parse(json, result));
    ASSERT_TRUE(result.empty());
}

TEST(SimpleJsonParserTest, TestHandlesEmptyInput) {
    SimpleJsonParser parser;
    const std::string json = "";
    std::map<std::string, std::string> result;

    ASSERT_FALSE(parser.parse(json, result));
    ASSERT_TRUE(result.empty());
}