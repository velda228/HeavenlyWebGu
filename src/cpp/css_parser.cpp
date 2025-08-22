#include "css_parser.h"

CssParser::CssParser() {
}

CssParser::~CssParser() {
}

bool CssParser::parse(const std::string& css) {
    content = css;
    return true;
}
