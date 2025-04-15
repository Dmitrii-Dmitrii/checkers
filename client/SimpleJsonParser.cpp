#include "SimpleJsonParser.h"

#include <iostream>
#include <regex>

bool SimpleJsonParser::parse(const std::string& json, std::map<std::string, std::string>& result) {
    try {
        result.clear();

        std::regex pattern("\"([^\"]+)\"\\s*:\\s*\"([^\"]*)\"");

        auto words_begin = std::sregex_iterator(json.begin(), json.end(), pattern);
        auto words_end = std::sregex_iterator();

        for (auto i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            if (match.size() >= 3) {
                result[match[1].str()] = match[2].str();
            }
        }

        return !result.empty();
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
}
