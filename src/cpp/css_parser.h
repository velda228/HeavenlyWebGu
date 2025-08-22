#pragma once

#include <string>

class CssParser {
public:
    CssParser();
    ~CssParser();
    
    bool parse(const std::string& css);
    
private:
    std::string content;
};
