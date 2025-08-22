#pragma once

#include <string>
#include <vector>
#include <map>
#include <gtk/gtk.h>

struct SimpleHtmlElement {
    std::string tag_name;
    std::map<std::string, std::string> attributes;
    std::string text_content;
    std::vector<SimpleHtmlElement> children;
};

class SimpleHtmlRenderer {
public:
    SimpleHtmlRenderer();
    ~SimpleHtmlRenderer();
    
    // Парсит HTML напрямую (без JSON)
    bool parse_html(const std::string& html);
    
    // Рендерит HTML в GTK виджет
    GtkWidget* render_to_widget();
    
    // Очищает все данные
    void clear();
    
private:
    std::vector<SimpleHtmlElement> elements;
    
    // Простой HTML парсер
    void parse_html_recursive(const std::string& html, size_t& pos, SimpleHtmlElement& element);
    void parse_attributes(const std::string& attr_string, std::map<std::string, std::string>& attributes);
    
    // Создает GTK виджет для элемента
    GtkWidget* create_element_widget(const SimpleHtmlElement& element);
    
    // Применяет базовые стили
    void apply_basic_styles(GtkWidget* widget, const std::string& tag_name);
};
