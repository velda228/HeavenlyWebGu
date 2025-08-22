#pragma once

#include <string>

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool render(const std::string& content);
    
private:
    std::string last_content;
};
