#include "browser_styles.h"
#include <iostream>

GtkCssProvider* BrowserStyles::css_provider = nullptr;
bool BrowserStyles::css_initialized = false;

void BrowserStyles::init_css_provider() {
    if (css_initialized) return;
    
    css_provider = gtk_css_provider_new();
    
    // Загружаем CSS стили
    std::string css = get_css_string();
    
    GError* error = nullptr;
    gtk_css_provider_load_from_data(css_provider, css.c_str(), css.length(), &error);
    
    if (error) {
        std::cerr << "Ошибка загрузки CSS: " << error->message << std::endl;
        g_error_free(error);
    } else {
        // Применяем стили глобально
        GtkStyleContext* context = gtk_style_context_new();
        gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref(context);
        
        css_initialized = true;
    }
}

void BrowserStyles::apply_styles(GtkWidget* widget) {
    if (!css_initialized) {
        init_css_provider();
    }
    
    if (css_provider && widget) {
        GtkStyleContext* context = gtk_widget_get_style_context(widget);
        gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

std::string BrowserStyles::get_css_string() {
    return R"(
        /* Основные стили браузера */
        window {
            background-color: #f8f9fa;
            color: #212529;
        }
        
        /* Панель инструментов */
        toolbar {
            background-color: #ffffff;
            border-bottom: 1px solid #dee2e6;
            padding: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        /* Кнопки навигации */
        button {
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: 500;
            transition: all 0.2s ease;
        }
        
        button:hover {
            background-color: #0056b3;
            transform: translateY(-1px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.2);
        }
        
        button:active {
            transform: translateY(0);
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        /* Адресная строка */
        entry {
            background-color: #ffffff;
            border: 2px solid #e9ecef;
            border-radius: 8px;
            padding: 10px 15px;
            font-size: 14px;
            transition: border-color 0.2s ease;
        }
        
        entry:focus {
            border-color: #007bff;
            box-shadow: 0 0 0 3px rgba(0,123,255,0.1);
        }
        
        /* Notebook (вкладки) */
        notebook {
            background-color: #ffffff;
            border-radius: 8px;
            margin: 10px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        
        notebook tab {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-bottom: none;
            border-radius: 6px 6px 0 0;
            padding: 8px 16px;
            margin-right: 2px;
            transition: all 0.2s ease;
        }
        
        notebook tab:checked {
            background-color: #ffffff;
            border-color: #007bff;
            color: #007bff;
            font-weight: 600;
        }
        
        notebook tab:hover:not(:checked) {
            background-color: #e9ecef;
        }
        
        /* Статус бар */
        statusbar {
            background-color: #ffffff;
            border-top: 1px solid #dee2e6;
            padding: 8px 15px;
            font-size: 12px;
            color: #6c757d;
        }
        
        /* Контент страницы */
        .content-view {
            background-color: #ffffff;
            padding: 20px;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            line-height: 1.6;
        }
        
        /* HTML элементы */
        .title {
            font-size: 24px;
            font-weight: 700;
            color: #212529;
            margin-bottom: 20px;
            border-bottom: 2px solid #007bff;
            padding-bottom: 10px;
        }
        
        .heading {
            font-size: 20px;
            font-weight: 600;
            color: #343a40;
            margin-top: 25px;
            margin-bottom: 15px;
        }
        
        .paragraph {
            font-size: 16px;
            color: #495057;
            margin-bottom: 15px;
            text-align: justify;
        }
        
        .link {
            color: #007bff;
            text-decoration: none;
            font-weight: 500;
            transition: color 0.2s ease;
        }
        
        .link:hover {
            color: #0056b3;
            text-decoration: underline;
        }
        
        .link:visited {
            color: #6f42c1;
        }
        
        .image {
            border-radius: 8px;
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
            transition: transform 0.2s ease;
        }
        
        .image:hover {
            transform: scale(1.02);
        }
        
        .list {
            background-color: #f8f9fa;
            border-radius: 8px;
            padding: 15px;
            margin: 15px 0;
        }
        
        .list-item {
            padding: 8px 0;
            border-bottom: 1px solid #e9ecef;
        }
        
        .list-item:last-child {
            border-bottom: none;
        }
        
        .table {
            border-collapse: collapse;
            width: 100%;
            margin: 20px 0;
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        
        .table-header {
            background-color: #007bff;
            color: white;
            font-weight: 600;
            padding: 12px;
            text-align: left;
        }
        
        .table-cell {
            padding: 12px;
            border-bottom: 1px solid #e9ecef;
            background-color: #ffffff;
        }
        
        .table-row:nth-child(even) .table-cell {
            background-color: #f8f9fa;
        }
        
        .form {
            background-color: #f8f9fa;
            border-radius: 8px;
            padding: 20px;
            margin: 20px 0;
        }
        
        .input, .textarea, .select {
            background-color: #ffffff;
            border: 2px solid #e9ecef;
            border-radius: 6px;
            padding: 10px;
            font-size: 14px;
            transition: border-color 0.2s ease;
            width: 100%;
            margin-bottom: 15px;
        }
        
        .input:focus, .textarea:focus, .select:focus {
            border-color: #007bff;
            box-shadow: 0 0 0 3px rgba(0,123,255,0.1);
            outline: none;
        }
        
        .button {
            background-color: #28a745;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 12px 24px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
        }
        
        .button:hover {
            background-color: #218838;
            transform: translateY(-1px);
            box-shadow: 0 4px 12px rgba(0,0,0,0.2);
        }
        
        .code {
            background-color: #f8f9fa;
            border: 1px solid #e9ecef;
            border-radius: 6px;
            padding: 15px;
            font-family: "SF Mono", Monaco, "Cascadia Code", "Roboto Mono", Consolas, "Courier New", monospace;
            font-size: 14px;
            line-height: 1.5;
            overflow-x: auto;
        }
        
        .blockquote {
            background-color: #f8f9fa;
            border-left: 4px solid #007bff;
            padding: 15px 20px;
            margin: 20px 0;
            font-style: italic;
            color: #6c757d;
        }
        
        .bold {
            font-weight: 700;
            color: #212529;
        }
        
        .italic {
            font-style: italic;
            color: #495057;
        }
        
        .underline {
            text-decoration: underline;
            color: #495057;
        }
        
        /* Анимации */
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .content-view > * {
            animation: fadeIn 0.3s ease-out;
        }
        
        /* Адаптивность */
        @media (max-width: 768px) {
            .content-view {
                padding: 15px;
            }
            
            .title {
                font-size: 20px;
            }
            
            .heading {
                font-size: 18px;
            }
            
            .paragraph {
                font-size: 14px;
            }
        }
    )";
}
