#include "renderer.h"

Renderer::Renderer() {
}

Renderer::~Renderer() {
}

bool Renderer::render(const std::string& content) {
    last_content = content;
    // TODO: Реализовать рендеринг
    return true;
}
