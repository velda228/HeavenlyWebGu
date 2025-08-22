#pragma once

#include <string>

class HtmlParser {
public:
    HtmlParser();
    ~HtmlParser();
    
    bool parse(const std::string& html);
    std::string get_text_content() const;
    
private:
    std::string content;
};
