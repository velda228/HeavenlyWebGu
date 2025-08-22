#include "rust_html_renderer.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <vector>

RustHtmlRenderer::RustHtmlRenderer() {
}

RustHtmlRenderer::~RustHtmlRenderer() {
}

bool RustHtmlRenderer::parse_from_rust(HtmlParser* rust_parser) {
    if (!rust_parser) {
        std::cout << "Ошибка: Rust парсер не инициализирован" << std::endl;
        return false;
    }
    
    clear();
    
    // Получаем количество элементов от Rust
    size_t element_count = html_get_element_count(rust_parser);
    std::cout << "Rust парсер нашел " << element_count << " элементов" << std::endl;
    
    if (element_count == 0) {
        std::cout << "Нет элементов для рендеринга" << std::endl;
        return false;
    }
    
    // Обрабатываем каждый элемент
    for (size_t i = 0; i < element_count; i++) {
        RustHtmlElement element;
        
        // Получаем имя тега
        char* tag_name_ptr = html_get_element_tag_name(rust_parser, i);
        if (tag_name_ptr) {
            element.tag_name = std::string(tag_name_ptr);
            string_free(tag_name_ptr);
        }
        
        // Получаем текст элемента
        char* text_ptr = html_get_element_text(rust_parser, i);
        if (text_ptr) {
            element.text_content = std::string(text_ptr);
            string_free(text_ptr);
        }
        
        // Получаем атрибуты
        size_t attr_count = html_get_element_attribute_count(rust_parser, i);
        for (size_t j = 0; j < attr_count; j++) {
            char* attr_name_ptr = html_get_element_attribute_name(rust_parser, i, j);
            if (attr_name_ptr) {
                std::string attr_name = std::string(attr_name_ptr);
                string_free(attr_name_ptr);
                
                char* attr_value_ptr = html_get_element_attribute(rust_parser, i, attr_name.c_str());
                if (attr_value_ptr) {
                    std::string attr_value = std::string(attr_value_ptr);
                    element.attributes[attr_name] = attr_value;
                    string_free(attr_value_ptr);
                }
            }
        }
        
        // Добавляем элемент только если он имеет смысл
        if (!element.tag_name.empty() && 
            (element.tag_name != "head" && element.tag_name != "script" && 
             element.tag_name != "style" && element.tag_name != "meta" && 
             element.tag_name != "link" && element.tag_name != "noscript")) {
            
            elements.push_back(element);
        }
    }
    
    std::cout << "Подготовлено " << elements.size() << " элементов для рендеринга" << std::endl;
    return !elements.empty();
}

GtkWidget* RustHtmlRenderer::render_to_widget() {
    if (elements.empty()) {
        return gtk_label_new("Нет контента для отображения");
    }
    
    std::cout << "Рендерим " << elements.size() << " элементов из Rust" << std::endl;
    
    // Создаем основной контейнер с прокруткой
    GtkWidget* scrolled_window = gtk_scrolled_window_new(nullptr, nullptr);
    GtkWidget* main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Добавляем заголовок
    GtkWidget* title_label = gtk_label_new("HTML Рендеринг (Rust + C++)");
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

GtkWidget* RustHtmlRenderer::create_element_widget(const RustHtmlElement& element) {
    GtkWidget* widget = nullptr;
    
    // Создаем виджет в зависимости от тега
    if (element.tag_name == "#text") {
        // Чистый текст
        if (!element.text_content.empty()) {
            widget = gtk_label_new(element.text_content.c_str());
            gtk_widget_set_name(widget, "text-node");
            gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
            gtk_label_set_line_wrap_mode(GTK_LABEL(widget), PANGO_WRAP_WORD_CHAR);
            gtk_label_set_selectable(GTK_LABEL(widget), TRUE);
        }
    }
    else if (element.tag_name == "html" || element.tag_name == "body" || element.tag_name == "div") {
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
        // Изображения - используем Rust сетевой модуль
        std::string alt_text = element.text_content;
        if (alt_text.empty()) alt_text = "[Изображение]";
        
        // Проверяем src атрибут
        auto src_it = element.attributes.find("src");
        if (src_it != element.attributes.end()) {
            std::string src = src_it->second;
            if (!src.empty()) {
                widget = load_image(src, alt_text);
            } else {
                widget = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
            }
        } else {
            widget = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
        }
        gtk_widget_set_name(widget, "image");
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
        apply_styles(widget, element.tag_name);
    }
    
    return widget;
}

GtkWidget* RustHtmlRenderer::load_image(const std::string& src, const std::string& alt_text) {
    // Используем Rust сетевой модуль для загрузки изображений
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Пытаемся загрузить изображение
    char* image_data = network_fetch_image(src.c_str());
    
    if (image_data) {
        // Создаем изображение из base64 данных
        std::string base64_data(image_data);
        string_free(image_data);
        
        // Конвертируем base64 в байты
        std::vector<guchar> image_bytes;
        try {
            // Простая base64 декодировка
            size_t decoded_size = (base64_data.length() * 3) / 4;
            image_bytes.resize(decoded_size);
            
            // TODO: Реализовать proper base64 декодирование
            // Пока используем заглушку
            GtkWidget* image = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
            gtk_box_pack_start(GTK_BOX(container), image, FALSE, FALSE, 2);
            
            // Подпись с URL
            std::string caption = "Изображение загружено: " + src;
            if (!alt_text.empty() && alt_text != "[Изображение]") {
                caption += " (" + alt_text + ")";
            }
            
            GtkWidget* label = gtk_label_new(caption.c_str());
            gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
            gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
            gtk_box_pack_start(GTK_BOX(container), label, FALSE, FALSE, 2);
            
        } catch (...) {
            // В случае ошибки показываем заглушку
            GtkWidget* image = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
            gtk_box_pack_start(GTK_BOX(container), image, FALSE, FALSE, 2);
            
            std::string caption = "Ошибка загрузки: " + src;
            GtkWidget* label = gtk_label_new(caption.c_str());
            gtk_box_pack_start(GTK_BOX(container), label, FALSE, FALSE, 2);
        }
    } else {
        // Ошибка загрузки - показываем заглушку
        GtkWidget* image = gtk_image_new_from_icon_name("image-x-generic", GTK_ICON_SIZE_DIALOG);
        gtk_box_pack_start(GTK_BOX(container), image, FALSE, FALSE, 2);
        
        // Подпись с URL
        std::string caption = "Не удалось загрузить: " + src;
        if (!alt_text.empty() && alt_text != "[Изображение]") {
            caption += " (" + alt_text + ")";
        }
        
        GtkWidget* label = gtk_label_new(caption.c_str());
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
        gtk_box_pack_start(GTK_BOX(container), label, FALSE, FALSE, 2);
    }
    
    return container;
}

void RustHtmlRenderer::apply_styles(GtkWidget* widget, const std::string& tag_name) {
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

void RustHtmlRenderer::clear() {
    elements.clear();
}
