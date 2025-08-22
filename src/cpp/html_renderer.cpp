#include "html_renderer.h"
#include <iostream>
#include <sstream>
#include <cctype>

HtmlRenderer::HtmlRenderer() {
}

HtmlRenderer::~HtmlRenderer() {
}

bool HtmlRenderer::parse_json(const std::string& json) {
    // Упрощенный JSON парсер для HTML элементов
    clear();
    
    std::cout << "Начинаем парсинг JSON, длина: " << json.length() << std::endl;
    
    // Ограничиваем количество элементов для производительности
    const size_t MAX_ELEMENTS = 1000;
    size_t element_count = 0;
    
    // Ищем все вхождения "tag_name"
    size_t pos = 0;
    while (pos < json.length() && element_count < MAX_ELEMENTS) {
        size_t tag_pos = json.find("\"tag_name\"", pos);
        if (tag_pos == std::string::npos) break;
        
        HtmlElement element;
        
        // Ищем начало значения тега
        size_t value_start = json.find('"', tag_pos + 11); // 11 = длина "tag_name"
        if (value_start == std::string::npos) {
            pos = tag_pos + 11;
            continue;
        }
        
        size_t value_end = json.find('"', value_start + 1);
        if (value_end == std::string::npos) {
            pos = value_start + 1;
            continue;
        }
        
        // Извлекаем имя тега
        element.tag_name = json.substr(value_start + 1, value_end - value_start - 1);
        
        // Ищем атрибуты (упрощенно)
        size_t attr_start = json.find("\"attributes\"", tag_pos);
        if (attr_start != std::string::npos && attr_start < json.find("\"tag_name\"", tag_pos + 1)) {
            // TODO: Парсить атрибуты более детально
        }
        
        // Ищем текст (упрощенно)
        size_t text_start = json.find("\"text_content\"", tag_pos);
        if (text_start != std::string::npos && text_start < json.find("\"tag_name\"", tag_pos + 1)) {
            size_t text_value_start = json.find('"', text_start + 15); // 15 = длина "text_content"
            if (text_value_start != std::string::npos) {
                size_t text_value_end = json.find('"', text_value_start + 1);
                if (text_value_end != std::string::npos) {
                    element.text_content = json.substr(text_value_start + 1, text_value_end - text_value_start - 1);
                }
            }
        }
        
        if (!element.tag_name.empty()) {
            elements.push_back(element);
            element_count++;
            
            if (element_count % 100 == 0) {
                std::cout << "Найдено элементов: " << element_count << std::endl;
            }
        }
        
        pos = value_end + 1;
    }
    
    std::cout << "Всего найдено элементов: " << elements.size() << std::endl;
    return !elements.empty();
}

GtkWidget* HtmlRenderer::render_to_widget() {
    if (elements.empty()) {
        return gtk_label_new("Нет контента для отображения");
    }
    
    std::cout << "Рендерим " << elements.size() << " элементов" << std::endl;
    
    // Создаем основной контейнер
    GtkWidget* main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Добавляем заголовок с информацией
    GtkWidget* info_label = gtk_label_new(("Найдено элементов: " + std::to_string(elements.size())).c_str());
    gtk_box_pack_start(GTK_BOX(main_container), info_label, FALSE, FALSE, 10);
    
    // Ограничиваем количество рендеримых элементов для производительности
    const size_t MAX_RENDER_ELEMENTS = 100;
    size_t rendered_count = 0;
    
    // Рендерим только важные элементы
    for (const auto& element : elements) {
        if (rendered_count >= MAX_RENDER_ELEMENTS) {
            // Добавляем сообщение о том, что показаны не все элементы
            GtkWidget* more_label = gtk_label_new("... показаны только первые 100 элементов для производительности ...");
            gtk_box_pack_start(GTK_BOX(main_container), more_label, FALSE, FALSE, 10);
            break;
        }
        
        // Пропускаем технические теги
        if (element.tag_name == "head" || element.tag_name == "script" || 
            element.tag_name == "style" || element.tag_name == "meta" ||
            element.tag_name == "link") {
            continue;
        }
        
        // Создаем виджет для элемента
        GtkWidget* widget = create_element_widget(element);
        if (widget) {
            gtk_box_pack_start(GTK_BOX(main_container), widget, FALSE, FALSE, 2);
            rendered_count++;
        }
        
        // Добавляем отладочную информацию только для первых 10 элементов
        if (rendered_count <= 10) {
            std::string debug_text = "Тег: " + element.tag_name;
            if (!element.text_content.empty()) {
                debug_text += " | Текст: " + element.text_content.substr(0, 50);
                if (element.text_content.length() > 50) debug_text += "...";
            }
            
            GtkWidget* debug_label = gtk_label_new(debug_text.c_str());
            gtk_box_pack_start(GTK_BOX(main_container), debug_label, FALSE, FALSE, 2);
        }
    }
    
    std::cout << "Отрендерено " << rendered_count << " элементов" << std::endl;
    return main_container;
}

GtkWidget* HtmlRenderer::create_element_widget(const HtmlElement& element) {
    GtkWidget* widget = nullptr;
    
    if (element.tag_name == "html" || element.tag_name == "body") {
        // Контейнер для основного контента
        widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    }
    else if (element.tag_name == "head") {
        // Пропускаем head
        return nullptr;
    }
    else if (element.tag_name == "title") {
        // Заголовок страницы
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "title");
    }
    else if (element.tag_name == "h1" || element.tag_name == "h2" || 
             element.tag_name == "h3" || element.tag_name == "h4" || 
             element.tag_name == "h5" || element.tag_name == "h6") {
        // Заголовки
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "heading");
    }
    else if (element.tag_name == "p") {
        // Параграфы
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "paragraph");
        gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(widget), PANGO_WRAP_WORD_CHAR);
    }
    else if (element.tag_name == "a") {
        // Ссылки
        widget = gtk_link_button_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "link");
        
        // Получаем href атрибут
        auto it = element.attributes.find("href");
        if (it != element.attributes.end()) {
            gtk_link_button_set_uri(GTK_LINK_BUTTON(widget), it->second.c_str());
        }
    }
    else if (element.tag_name == "img") {
        // Изображения
        widget = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
        gtk_widget_set_name(widget, "image");
        
        // Получаем src атрибут
        auto it = element.attributes.find("src");
        if (it != element.attributes.end()) {
            // TODO: Загружать реальное изображение
            gtk_image_set_from_icon_name(GTK_IMAGE(widget), "image-x-generic", GTK_ICON_SIZE_DIALOG);
        }
    }
    else if (element.tag_name == "div" || element.tag_name == "span") {
        // Контейнеры
        widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
        gtk_widget_set_name(widget, "container");
    }
    else if (element.tag_name == "ul" || element.tag_name == "ol") {
        // Списки
        widget = gtk_list_box_new();
        gtk_widget_set_name(widget, "list");
    }
    else if (element.tag_name == "li") {
        // Элементы списка
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "list-item");
    }
    else if (element.tag_name == "table") {
        // Таблицы
        widget = gtk_grid_new();
        gtk_widget_set_name(widget, "table");
    }
    else if (element.tag_name == "tr") {
        // Строки таблицы
        widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
        gtk_widget_set_name(widget, "table-row");
    }
    else if (element.tag_name == "td" || element.tag_name == "th") {
        // Ячейки таблицы
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, element.tag_name == "th" ? "table-header" : "table-cell");
    }
    else if (element.tag_name == "form") {
        // Формы
        widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_widget_set_name(widget, "form");
    }
    else if (element.tag_name == "input") {
        // Поля ввода
        auto it = element.attributes.find("type");
        if (it != element.attributes.end()) {
            if (it->second == "text" || it->second == "email" || it->second == "password") {
                widget = gtk_entry_new();
                gtk_entry_set_placeholder_text(GTK_ENTRY(widget), element.text_content.c_str());
            } else if (it->second == "submit") {
                widget = gtk_button_new_with_label(element.text_content.c_str());
            } else if (it->second == "checkbox") {
                widget = gtk_check_button_new_with_label(element.text_content.c_str());
            } else if (it->second == "radio") {
                widget = gtk_radio_button_new_with_label(nullptr, element.text_content.c_str());
            }
        } else {
            widget = gtk_entry_new();
        }
        gtk_widget_set_name(widget, "input");
    }
    else if (element.tag_name == "textarea") {
        // Многострочные поля ввода
        widget = gtk_text_view_new();
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_WORD_CHAR);
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget)), 
                                element.text_content.c_str(), -1);
        gtk_widget_set_name(widget, "textarea");
    }
    else if (element.tag_name == "select") {
        // Выпадающие списки
        widget = gtk_combo_box_text_new();
        gtk_widget_set_name(widget, "select");
    }
    else if (element.tag_name == "option") {
        // Опции выпадающего списка
        // TODO: Добавлять в родительский select
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "option");
    }
    else if (element.tag_name == "button") {
        // Кнопки
        widget = gtk_button_new_with_label(element.text_content.c_str());
        gtk_widget_set_name(widget, "button");
    }
    else if (element.tag_name == "br") {
        // Перенос строки
        widget = gtk_label_new("");
        gtk_widget_set_name(widget, "line-break");
    }
    else if (element.tag_name == "hr") {
        // Горизонтальная линия
        widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_set_name(widget, "horizontal-rule");
    }
    else if (element.tag_name == "code" || element.tag_name == "pre") {
        // Код
        widget = gtk_text_view_new();
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_CHAR);
        gtk_text_view_set_monospace(GTK_TEXT_VIEW(widget), TRUE);
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget)), 
                                element.text_content.c_str(), -1);
        gtk_widget_set_name(widget, "code");
    }
    else if (element.tag_name == "strong" || element.tag_name == "b") {
        // Жирный текст
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "bold");
    }
    else if (element.tag_name == "em" || element.tag_name == "i") {
        // Курсив
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "italic");
    }
    else if (element.tag_name == "u") {
        // Подчеркнутый текст
        widget = gtk_label_new(element.text_content.c_str());
        gtk_widget_set_name(widget, "underline");
    }
    else if (element.tag_name == "blockquote") {
        // Цитата
        widget = gtk_frame_new(nullptr);
        GtkWidget* label = gtk_label_new(element.text_content.c_str());
        gtk_container_add(GTK_CONTAINER(widget), label);
        gtk_widget_set_name(widget, "blockquote");
    }
    else {
        // Неизвестный тег - отображаем как текст
        if (!element.text_content.empty()) {
            widget = gtk_label_new(element.text_content.c_str());
            gtk_widget_set_name(widget, "unknown");
        }
    }
    
    if (widget) {
        apply_styles(widget, element.tag_name);
    }
    
    return widget;
}

void HtmlRenderer::apply_styles(GtkWidget* widget, const std::string& tag_name) {
    if (!widget) return;
    
    // Применяем базовые стили в зависимости от тега
    if (tag_name == "h1") {
        gtk_widget_set_margin_start(widget, 10);
        gtk_widget_set_margin_end(widget, 10);
        gtk_widget_set_margin_top(widget, 15);
        gtk_widget_set_margin_bottom(widget, 10);
    }
    else if (tag_name == "h2" || tag_name == "h3") {
        gtk_widget_set_margin_start(widget, 15);
        gtk_widget_set_margin_end(widget, 15);
        gtk_widget_set_margin_top(widget, 10);
        gtk_widget_set_margin_bottom(widget, 5);
    }
    else if (tag_name == "p") {
        gtk_widget_set_margin_start(widget, 20);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 5);
        gtk_widget_set_margin_bottom(widget, 5);
    }
    else if (tag_name == "a") {
        gtk_widget_set_margin_start(widget, 20);
        gtk_widget_set_margin_end(widget, 20);
    }
    else if (tag_name == "img") {
        gtk_widget_set_margin_start(widget, 20);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 10);
        gtk_widget_set_margin_bottom(widget, 10);
    }
    else if (tag_name == "ul" || tag_name == "ol") {
        gtk_widget_set_margin_start(widget, 30);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 5);
        gtk_widget_set_margin_bottom(widget, 5);
    }
    else if (tag_name == "li") {
        gtk_widget_set_margin_start(widget, 10);
        gtk_widget_set_margin_end(widget, 10);
        gtk_widget_set_margin_top(widget, 2);
        gtk_widget_set_margin_bottom(widget, 2);
    }
    else if (tag_name == "table") {
        gtk_widget_set_margin_start(widget, 20);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 10);
        gtk_widget_set_margin_bottom(widget, 10);
    }
    else if (tag_name == "form") {
        gtk_widget_set_margin_start(widget, 20);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 15);
        gtk_widget_set_margin_bottom(widget, 15);
    }
    else if (tag_name == "input" || tag_name == "textarea" || tag_name == "select") {
        gtk_widget_set_margin_start(widget, 20);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 5);
        gtk_widget_set_margin_bottom(widget, 5);
    }
    else if (tag_name == "button") {
        gtk_widget_set_margin_start(widget, 20);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 10);
        gtk_widget_set_margin_bottom(widget, 10);
    }
    else if (tag_name == "blockquote") {
        gtk_widget_set_margin_start(widget, 30);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 10);
        gtk_widget_set_margin_bottom(widget, 10);
    }
}

void HtmlRenderer::clear() {
    elements.clear();
}
