#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST(JsonTests, l0_test_json_string_parsing)
{
    std::string jsonString = R"({
        "name": "John Doe",
        "age": 30,
        "isEmployed": true
    })";

    // Parse the JSON string into a json object
    json jsonData = json::parse(jsonString);

    EXPECT_EQ(std::string("John Doe"), jsonData["name"].get<std::string>());
}
