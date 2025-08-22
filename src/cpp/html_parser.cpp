#include "html_parser.h"

HtmlParser::HtmlParser() {
}

HtmlParser::~HtmlParser() {
}

bool HtmlParser::parse(const std::string& html) {
    content = html;
    return true;
}

std::string HtmlParser::get_text_content() const {
    return content;
}
