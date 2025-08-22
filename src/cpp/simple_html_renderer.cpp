#include "simple_html_renderer.h"
#include <iostream>
#include <algorithm>
#include <regex>

SimpleHtmlRenderer::SimpleHtmlRenderer() {
}

SimpleHtmlRenderer::~SimpleHtmlRenderer() {
}

bool SimpleHtmlRenderer::parse_html(const std::string& html) {
    clear();
    
    std::cout << "Парсим HTML напрямую, длина: " << html.length() << std::endl;
    
    // Простой, но эффективный парсер
    size_t pos = 0;
    size_t element_count = 0;
    const size_t MAX_ELEMENTS = 200; // Увеличиваем лимит
    
    while (pos < html.length() && element_count < MAX_ELEMENTS) {
        // Ищем открывающий тег
        size_t tag_start = html.find('<', pos);
        if (tag_start == std::string::npos) break;
        
        // Ищем закрывающий тег
        size_t tag_end = html.find('>', tag_start);
        if (tag_end == std::string::npos) break;
        
        // Извлекаем тег
        std::string tag = html.substr(tag_start + 1, tag_end - tag_start - 1);
        
        // Пропускаем комментарии и закрывающие теги
        if (tag.empty() || tag[0] == '!' || tag[0] == '/') {
            pos = tag_end + 1;
            continue;
        }
        
        // Извлекаем имя тега
        std::string tag_name;
        size_t space_pos = tag.find(' ');
        if (space_pos != std::string::npos) {
            tag_name = tag.substr(0, space_pos);
        } else {
            tag_name = tag;
        }
        
        // Пропускаем технические теги
        if (tag_name == "head" || tag_name == "script" || tag_name == "style" || 
            tag_name == "meta" || tag_name == "link" || tag_name == "noscript" ||
            tag_name == "!--") {
            pos = tag_end + 1;
            continue;
        }
        
        // Создаем элемент
        SimpleHtmlElement element;
        element.tag_name = tag_name;
        
        // Извлекаем атрибуты
        if (space_pos != std::string::npos) {
            std::string attr_part = tag.substr(space_pos + 1);
            parse_attributes(attr_part, element.attributes);
        }
        
        // Ищем текст после тега
        size_t text_start = tag_end + 1;
        size_t next_tag = html.find('<', text_start);
        if (next_tag != std::string::npos) {
            std::string text = html.substr(text_start, next_tag - text_start);
            // Очищаем текст от лишних пробелов
            text.erase(0, text.find_first_not_of(" \n\r\t"));
            text.erase(text.find_last_not_of(" \n\r\t") + 1);
            if (!text.empty() && text.length() > 1) {
                element.text_content = text;
            }
        }
        
        // Добавляем элемент только если у него есть контент или это важный тег
        if (!element.text_content.empty() || 
            tag_name == "a" || tag_name == "img" || tag_name == "button" || 
            tag_name == "input" || tag_name == "h1" || tag_name == "h2" || 
            tag_name == "h3" || tag_name == "p" || tag_name == "div" ||
            tag_name == "span" || tag_name == "li" || tag_name == "td" || 
            tag_name == "th" || tag_name == "title") {
            
            elements.push_back(element);
            element_count++;
            
            if (element_count % 20 == 0) {
                std::cout << "Найдено элементов: " << element_count << std::endl;
            }
        }
        
        pos = tag_end + 1;
    }
    
    std::cout << "Всего найдено элементов: " << elements.size() << std::endl;
    return !elements.empty();
}

void SimpleHtmlRenderer::parse_attributes(const std::string& attr_string, std::map<std::string, std::string>& attributes) {
    // Простой парсер атрибутов
    std::regex attr_regex(R"((\w+)\s*=\s*["']([^"']*)["'])");
    std::sregex_iterator iter(attr_string.begin(), attr_string.end(), attr_regex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::string key = (*iter)[1];
        std::string value = (*iter)[2];
        attributes[key] = value;
    }
}

void SimpleHtmlRenderer::parse_html_recursive(const std::string& html, size_t& pos, SimpleHtmlElement& element) {
    // Этот метод больше не используется
}

GtkWidget* SimpleHtmlRenderer::render_to_widget() {
    if (elements.empty()) {
        return gtk_label_new("Нет контента для отображения");
    }
    
    std::cout << "Рендерим " << elements.size() << " элементов" << std::endl;
    
    // Создаем основной контейнер с прокруткой
    GtkWidget* scrolled_window = gtk_scrolled_window_new(nullptr, nullptr);
    GtkWidget* main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Добавляем заголовок
    GtkWidget* title_label = gtk_label_new("HTML Рендеринг");
    gtk_widget_set_name(title_label, "title");
    gtk_box_pack_start(GTK_BOX(main_container), title_label, FALSE, FALSE, 10);
    
    // Добавляем информацию о количестве элементов
    std::string info_text = "Найдено элементов: " + std::to_string(elements.size());
    GtkWidget* info_label = gtk_label_new(info_text.c_str());
    gtk_box_pack_start(GTK_BOX(main_container), info_label, FALSE, FALSE, 5);
    
    // Рендерим элементы
    size_t rendered_count = 0;
    
    for (const auto& element : elements) {
        GtkWidget* widget = create_element_widget(element);
        if (widget) {
            gtk_box_pack_start(GTK_BOX(main_container), widget, FALSE, FALSE, 2);
            rendered_count++;
        }
    }
    
    gtk_container_add(GTK_CONTAINER(scrolled_window), main_container);
    gtk_widget_show_all(scrolled_window);
    
    std::cout << "Отрендерено " << rendered_count << " элементов" << std::endl;
    return scrolled_window;
}

GtkWidget* SimpleHtmlRenderer::create_element_widget(const SimpleHtmlElement& element) {
    GtkWidget* widget = nullptr;
    
    // Создаем виджет в зависимости от тега
    if (element.tag_name == "html" || element.tag_name == "body" || element.tag_name == "div") {
        // Контейнеры
        widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_name(widget, "container");
    }
    else if (element.tag_name == "title" || element.tag_name == "h1" || element.tag_name == "h2" || 
             element.tag_name == "h3" || element.tag_name == "h4" || element.tag_name == "h5" || element.tag_name == "h6") {
        // Заголовки
        std::string text = element.text_content;
        if (text.empty()) text = "[" + element.tag_name + "]";
        widget = gtk_label_new(text.c_str());
        gtk_widget_set_name(widget, "heading");
        
        // Делаем заголовки жирными
        PangoAttrList* attr_list = pango_attr_list_new();
        pango_attr_list_insert(attr_list, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
        gtk_label_set_attributes(GTK_LABEL(widget), attr_list);
        pango_attr_list_unref(attr_list);
    }
    else if (element.tag_name == "p") {
        // Параграфы
        std::string text = element.text_content;
        if (text.empty()) text = "[Параграф]";
        widget = gtk_label_new(text.c_str());
        gtk_widget_set_name(widget, "paragraph");
        gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(widget), PANGO_WRAP_WORD_CHAR);
    }
    else if (element.tag_name == "a") {
        // Ссылки
        std::string text = element.text_content;
        if (text.empty()) text = "[Ссылка]";
        
        // Проверяем href атрибут
        auto href_it = element.attributes.find("href");
        if (href_it != element.attributes.end()) {
            std::string url = href_it->second;
            if (!url.empty()) {
                widget = gtk_link_button_new_with_label(url.c_str(), text.c_str());
            } else {
                widget = gtk_link_button_new_with_label("#", text.c_str());
            }
        } else {
            widget = gtk_link_button_new_with_label("#", text.c_str());
        }
        gtk_widget_set_name(widget, "link");
    }
    else if (element.tag_name == "img") {
        // Изображения
        std::string alt_text = element.text_content;
        if (alt_text.empty()) alt_text = "[Изображение]";
        
        // Проверяем src атрибут
        auto src_it = element.attributes.find("src");
        if (src_it != element.attributes.end()) {
            std::string src = src_it->second;
            if (!src.empty()) {
                // Пытаемся загрузить изображение
                widget = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
                // TODO: Реализовать загрузку реальных изображений
            } else {
                widget = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
            }
        } else {
            widget = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
        }
        gtk_widget_set_name(widget, "image");
        
        // Добавляем подпись с alt текстом
        if (!alt_text.empty()) {
            GtkWidget* alt_label = gtk_label_new(alt_text.c_str());
            gtk_widget_set_name(alt_label, "image-alt");
            gtk_widget_set_margin_start(alt_label, 5);
        }
    }
    else if (element.tag_name == "span" || element.tag_name == "strong" || element.tag_name == "b" ||
             element.tag_name == "em" || element.tag_name == "i" || element.tag_name == "u") {
        // Текстовые элементы
        std::string text = element.text_content;
        if (text.empty()) text = "[" + element.tag_name + "]";
        widget = gtk_label_new(text.c_str());
        gtk_widget_set_name(widget, "text");
    }
    else if (element.tag_name == "ul" || element.tag_name == "ol") {
        // Списки
        widget = gtk_list_box_new();
        gtk_widget_set_name(widget, "list");
    }
    else if (element.tag_name == "li") {
        // Элементы списка
        std::string text = element.text_content;
        if (text.empty()) text = "[Элемент списка]";
        widget = gtk_label_new(text.c_str());
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
        std::string text = element.text_content;
        if (text.empty()) text = "[Ячейка]";
        widget = gtk_label_new(text.c_str());
        gtk_widget_set_name(widget, element.tag_name == "th" ? "table-header" : "table-cell");
    }
    else if (element.tag_name == "form") {
        // Формы
        widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_widget_set_name(widget, "form");
    }
    else if (element.tag_name == "input") {
        // Поля ввода
        std::string placeholder = element.text_content;
        if (placeholder.empty()) placeholder = "[Поле ввода]";
        
        // Проверяем type атрибут
        auto type_it = element.attributes.find("type");
        if (type_it != element.attributes.end()) {
            std::string type = type_it->second;
            if (type == "button" || type == "submit") {
                widget = gtk_button_new_with_label(placeholder.c_str());
            } else if (type == "checkbox") {
                widget = gtk_check_button_new_with_label(placeholder.c_str());
            } else if (type == "radio") {
                widget = gtk_radio_button_new_with_label(nullptr, placeholder.c_str());
            } else {
                widget = gtk_entry_new();
                gtk_entry_set_placeholder_text(GTK_ENTRY(widget), placeholder.c_str());
            }
        } else {
            widget = gtk_entry_new();
            gtk_entry_set_placeholder_text(GTK_ENTRY(widget), placeholder.c_str());
        }
        gtk_widget_set_name(widget, "input");
    }
    else if (element.tag_name == "button") {
        // Кнопки
        std::string text = element.text_content;
        if (text.empty()) text = "[Кнопка]";
        widget = gtk_button_new_with_label(text.c_str());
        gtk_widget_set_name(widget, "button");
    }
    else {
        // Неизвестный тег - отображаем как текст
        std::string text = element.text_content;
        if (text.empty()) text = "[" + element.tag_name + "]";
        widget = gtk_label_new(text.c_str());
        gtk_widget_set_name(widget, "unknown");
    }
    
    if (widget) {
        apply_basic_styles(widget, element.tag_name);
    }
    
    return widget;
}

void SimpleHtmlRenderer::apply_basic_styles(GtkWidget* widget, const std::string& tag_name) {
    if (!widget) return;
    
    // Применяем базовые стили
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
    else if (tag_name == "input" || tag_name == "button") {
        gtk_widget_set_margin_start(widget, 20);
        gtk_widget_set_margin_end(widget, 20);
        gtk_widget_set_margin_top(widget, 10);
        gtk_widget_set_margin_bottom(widget, 10);
    }
}

void SimpleHtmlRenderer::clear() {
    elements.clear();
}
