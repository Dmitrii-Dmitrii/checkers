#ifndef SIMPLEJSONPARSER_H
#define SIMPLEJSONPARSER_H
#include <map>
#include <string>

class SimpleJsonParser {
public:
    static bool parse(const std::string& json, std::map<std::string, std::string>& result);
};

#endif //SIMPLEJSONPARSER_H
