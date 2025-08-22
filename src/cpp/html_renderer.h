#pragma once

#include <string>
#include <vector>
#include <map>
#include <gtk/gtk.h>

struct HtmlElement {
    std::string tag_name;
    std::map<std::string, std::string> attributes;
    std::string text_content;
    std::vector<HtmlElement> children;
};

class HtmlRenderer {
public:
    HtmlRenderer();
    ~HtmlRenderer();
    
    // Парсит JSON и создает HTML элементы
    bool parse_json(const std::string& json);
    
    // Рендерит HTML в GTK виджет
    GtkWidget* render_to_widget();
    
    // Очищает все данные
    void clear();
    
private:
    std::vector<HtmlElement> elements;
    
    // Создает GTK виджет для элемента
    GtkWidget* create_element_widget(const HtmlElement& element);
    
    // Применяет CSS стили
    void apply_styles(GtkWidget* widget, const std::string& tag_name);
};
