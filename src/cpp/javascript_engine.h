#pragma once

#include <string>

class JavaScriptEngine {
public:
    JavaScriptEngine();
    ~JavaScriptEngine();
    
    bool execute(const std::string& code);
    
private:
    std::string last_code;
};
