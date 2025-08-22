#pragma once

#include <gtk/gtk.h>
#include <string>

class BrowserStyles {
public:
    // Применяет CSS стили к виджету
    static void apply_styles(GtkWidget* widget);
    
    // Возвращает CSS строку со стилями
    static std::string get_css_string();
    
private:
    // Инициализирует CSS провайдер
    static void init_css_provider();
    
    static GtkCssProvider* css_provider;
    static bool css_initialized;
};
