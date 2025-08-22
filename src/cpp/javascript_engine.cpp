#include "javascript_engine.h"

JavaScriptEngine::JavaScriptEngine() {
}

JavaScriptEngine::~JavaScriptEngine() {
}

bool JavaScriptEngine::execute(const std::string& code) {
    last_code = code;
    // TODO: Реализовать выполнение JavaScript
    return true;
}
