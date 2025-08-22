#pragma once

#include <string>
#include <vector>
#include <map>
#include <gtk/gtk.h>

// FFI интерфейсы для Rust
extern "C" {
    struct HtmlParser;
    size_t html_get_element_count(HtmlParser* parser);
    char* html_get_element_tag_name(HtmlParser* parser, size_t index);
    char* html_get_element_text(HtmlParser* parser, size_t index);
    char* html_get_element_attribute(HtmlParser* parser, size_t element_index, const char* attr_name);
    size_t html_get_element_attribute_count(HtmlParser* parser, size_t element_index);
    char* html_get_element_attribute_name(HtmlParser* parser, size_t element_index, size_t attr_index);
    
    // Сетевые функции
    char* network_fetch_image(const char* url);
    void string_free(char* ptr);
}

struct RustHtmlElement {
    std::string tag_name;
    std::map<std::string, std::string> attributes;
    std::string text_content;
};

class RustHtmlRenderer {
public:
    RustHtmlRenderer();
    ~RustHtmlRenderer();
    
    // Парсит HTML через Rust и создает элементы
    bool parse_from_rust(HtmlParser* rust_parser);
    
    // Рендерит HTML в GTK виджет
    GtkWidget* render_to_widget();
    
    // Очищает все данные
    void clear();
    
private:
    std::vector<RustHtmlElement> elements;
    
    // Создает GTK виджет для элемента
    GtkWidget* create_element_widget(const RustHtmlElement& element);
    
    // Применяет CSS стили
    void apply_styles(GtkWidget* widget, const std::string& tag_name);
    
    // Загружает изображение по URL
    GtkWidget* load_image(const std::string& src, const std::string& alt_text);
};
