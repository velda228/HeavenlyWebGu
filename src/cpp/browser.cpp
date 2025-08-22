#include "browser.h"
#include "browser_styles.h"
#include <iostream>
#include <gtk/gtk.h>

Browser::Browser() 
    : main_window(nullptr)
    , notebook(nullptr)
    , toolbar(nullptr)
    , address_bar(nullptr)
    , status_bar(nullptr)
    , content_view(nullptr)
    , html_parser(nullptr)
    , css_parser(nullptr)
    , html_renderer(nullptr)
    , javascript_enabled(true)
    , cookies_enabled(true)
    , popups_blocked(true)
    , https_only(false)
{
    // Инициализируем Rust парсеры
    html_parser = html_parse_new();
    // css_parser = css_parse_new(); // TODO: Добавить CSS парсер
    
    // Инициализируем HTML рендерер
               html_renderer = new SimpleHtmlRenderer();
    
    user_agent = "HeavenlyWebGu/1.0 (X11; Linux x86_64) AppleWebKit/537.36";
}

Browser::~Browser() {
    if (html_parser) {
        html_parse_free(html_parser);
    }
    
    if (html_renderer) {
        delete html_renderer;
    }
    
    // Очищаем GTK виджеты
    if (main_window) {
        gtk_widget_destroy(main_window);
    }
}

bool Browser::initialize() {
    // Инициализируем GTK
    if (!gtk_init_check(nullptr, nullptr)) {
        std::cerr << "Ошибка инициализации GTK" << std::endl;
        return false;
    }
    
    setup_ui();
    setup_signals();
    
    // Создаем первую вкладку
    create_tab();
    
    // Применяем стили
    BrowserStyles::apply_styles(main_window);
    
    // Показываем окно
    gtk_widget_show_all(main_window);
    
    return true;
}

void Browser::setup_ui() {
    // Создаем главное окно
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "HeavenlyWebGu");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1200, 800);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);
    
    // Создаем вертикальный контейнер
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), vbox);
    
    // Создаем панель инструментов
    toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 5);
    
    // Кнопки навигации
    GtkWidget* back_button = gtk_button_new_from_icon_name("go-previous", GTK_ICON_SIZE_BUTTON);
    GtkWidget* forward_button = gtk_button_new_from_icon_name("go-next", GTK_ICON_SIZE_BUTTON);
    GtkWidget* reload_button = gtk_button_new_from_icon_name("view-refresh", GTK_ICON_SIZE_BUTTON);
    GtkWidget* stop_button = gtk_button_new_from_icon_name("process-stop", GTK_ICON_SIZE_BUTTON);
    
    gtk_box_pack_start(GTK_BOX(toolbar), back_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(toolbar), forward_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(toolbar), reload_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(toolbar), stop_button, FALSE, FALSE, 2);
    
    // Адресная строка
    address_bar = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(address_bar), "Введите URL...");
    gtk_box_pack_start(GTK_BOX(toolbar), address_bar, TRUE, TRUE, 5);
    
    // Кнопка перехода
    GtkWidget* navigate_button = gtk_button_new_from_icon_name("go-jump", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(toolbar), navigate_button, FALSE, FALSE, 2);
    
    // Кнопка "Новая вкладка"
    GtkWidget* new_tab_button = gtk_button_new_from_icon_name("tab-new", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(new_tab_button, "Новая вкладка");
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_clicked), this);
    gtk_box_pack_start(GTK_BOX(toolbar), new_tab_button, FALSE, FALSE, 2);
    
    // Кнопка "Закрыть вкладку"
    GtkWidget* close_tab_button = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(close_tab_button, "Закрыть вкладку");
    g_signal_connect(close_tab_button, "clicked", G_CALLBACK(on_close_tab_clicked), this);
    gtk_box_pack_start(GTK_BOX(toolbar), close_tab_button, FALSE, FALSE, 2);
    
    // Создаем notebook для вкладок
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
    
    // Подключаем сигналы для вкладок
    g_signal_connect(notebook, "page-added", G_CALLBACK(on_page_added), this);
    g_signal_connect(notebook, "page-removed", G_CALLBACK(on_page_removed), this);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_switch_page), this);
    
    // Создаем прогресс бар
    progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
    gtk_widget_set_visible(progress_bar, FALSE);
    gtk_box_pack_end(GTK_BOX(vbox), progress_bar, FALSE, FALSE, 0);
    
    // Создаем статус бар
    status_bar = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(vbox), status_bar, FALSE, FALSE, 0);
    
    // Подключаем сигналы кнопок
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), this);
    g_signal_connect(forward_button, "clicked", G_CALLBACK(on_forward_clicked), this);
    g_signal_connect(reload_button, "clicked", G_CALLBACK(on_refresh_clicked), this);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_navigate_clicked), this);
    g_signal_connect(navigate_button, "clicked", G_CALLBACK(on_navigate_clicked), this);
    
    // Подключаем сигнал адресной строки
    g_signal_connect(address_bar, "activate", G_CALLBACK(on_address_bar_activate), this);
    
    // Подключаем обработчик клавиш для главного окна
    g_signal_connect(main_window, "key-press-event", G_CALLBACK(on_window_key_press), this);
}

void Browser::setup_signals() {
    // TODO: Подключить сигналы notebook
}

void Browser::create_web_view() {
    // Создаем простой текстовый вид для отображения контента
    content_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(content_view), GTK_WRAP_WORD_CHAR);
    
    // Добавляем в notebook
    GtkWidget* scrolled_window = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(scrolled_window), content_view);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, gtk_label_new("Новая вкладка"));
}

void Browser::create_new_tab() {
    // Создаем контейнер для вкладки
    GtkWidget* tab_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Создаем контейнер для контента
    GtkWidget* content_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Добавляем контент в контейнер вкладки
    gtk_box_pack_start(GTK_BOX(tab_container), content_container, TRUE, TRUE, 0);
    
    // Создаем заголовок вкладки
    GtkWidget* tab_label = gtk_label_new("Новая вкладка");
    
    // Добавляем вкладку в notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_container, tab_label);
    
    // Показываем все
    gtk_widget_show_all(tab_container);
    
    // Переключаемся на новую вкладку
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), -1);
}

void Browser::navigate(const std::string& url) {
    if (!content_view) {
        create_tab();
    }
    
    current_url = url;
    update_address_bar();
    
    // Проверяем кэш
    if (is_cached(url)) {
        update_status_bar("Загружаем из кэша: " + url);
        
        // Показываем прогресс загрузки
        show_loading_progress(true);
        update_loading_progress(0.5);
        
        // Используем кэшированные данные
        std::string cached_content = page_cache[url];
        std::string cached_parsed = parsed_cache[url];
        
        update_loading_progress(0.8);
        update_status_bar("Рендерим из кэша: " + url);
        
        // Рендерим кэшированную страницу
        if (html_renderer->parse_html(cached_parsed)) {
            GtkWidget* scrolled_window = gtk_widget_get_parent(content_view);
            if (scrolled_window) {
                gtk_container_remove(GTK_CONTAINER(scrolled_window), content_view);
                
                GtkWidget* rendered_content = html_renderer->render_to_widget();
                if (rendered_content) {
                    content_view = rendered_content;
                    gtk_container_add(GTK_CONTAINER(scrolled_window), content_view);
                    gtk_widget_show_all(scrolled_window);
                }
            }
        }
        
        update_loading_progress(1.0);
        update_status_bar("Загрузка из кэша завершена: " + url);
        
        // Скрываем прогресс бар
        g_timeout_add(1000, [](gpointer data) -> gboolean {
            Browser* browser = static_cast<Browser*>(data);
            browser->show_loading_progress(false);
            return G_SOURCE_REMOVE;
        }, this);
        
        return;
    }
    
    // Показываем прогресс загрузки
    show_loading_progress(true);
    update_loading_progress(0.1);
    update_status_bar("Начинаем загрузку: " + url);
    
    // Обновляем UI
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
    
    // Используем Rust сетевой модуль для получения HTML
    update_loading_progress(0.3);
    update_status_bar("Загружаем HTML: " + url);
    
    char* html_content = network_fetch_url(url.c_str());
    if (html_content) {
        update_loading_progress(0.6);
        update_status_bar("Парсим HTML: " + url);
        
        // Парсим HTML через Rust
        char* parsed_html = html_parse_string(html_parser, html_content);
        if (parsed_html) {
            update_loading_progress(0.8);
            update_status_bar("Рендерим страницу: " + url);
            
            // Используем HTML рендерер для отображения
            std::cout << "Пытаемся распарсить HTML напрямую, длина: " << strlen(html_content) << std::endl;
            
            if (html_renderer->parse_html(html_content)) {
                std::cout << "HTML успешно распарсен, рендерим..." << std::endl;
                
                // Очищаем текущий контент
                GtkWidget* scrolled_window = gtk_widget_get_parent(content_view);
                if (scrolled_window) {
                    gtk_container_remove(GTK_CONTAINER(scrolled_window), content_view);
                    
                    // Создаем новый рендеринг
                    GtkWidget* rendered_content = html_renderer->render_to_widget();
                    if (rendered_content) {
                        std::cout << "Контент отрендерен, добавляем в окно..." << std::endl;
                        content_view = rendered_content;
                        gtk_container_add(GTK_CONTAINER(scrolled_window), content_view);
                        gtk_widget_show_all(scrolled_window);
                        
                        // Принудительно обновляем UI
                        gtk_widget_queue_draw(scrolled_window);
                        gtk_widget_queue_draw(content_view);
                    } else {
                        std::cout << "Ошибка: рендеринг вернул nullptr" << std::endl;
                        display_content("Ошибка рендеринга страницы");
                    }
                } else {
                    std::cout << "Ошибка: не найден scrolled_window" << std::endl;
                    display_content("Ошибка: не найден контейнер для контента");
                }
            } else {
                std::cout << "Ошибка парсинга HTML, показываем исходный HTML" << std::endl;
                // Если рендеринг не удался, показываем исходный HTML
                std::string html_content = "Ошибка парсинга HTML. Исходный HTML:\n\n";
                html_content += html_content;
                display_content(html_content);
            }
            
            // Кэшируем страницу
            std::string html_str(html_content);
            std::string parsed_str(parsed_html);
            cache_page(url, html_str, parsed_str);
            
            string_free(parsed_html);
        }
        string_free(html_content);
        
        update_loading_progress(1.0);
        update_status_bar("Загрузка завершена: " + url);
    } else {
        // Fallback - показываем ошибку
        display_content("Ошибка загрузки страницы: " + url);
        update_status_bar("Ошибка загрузки: " + url);
    }
    
    // Скрываем прогресс бар через небольшую задержку
    g_timeout_add(1000, [](gpointer data) -> gboolean {
        Browser* browser = static_cast<Browser*>(data);
        browser->show_loading_progress(false);
        return G_SOURCE_REMOVE;
    }, this);
}

void Browser::reload() {
    if (!current_url.empty()) {
        navigate(current_url);
    }
}

void Browser::stop() {
    update_status_bar("Остановлено");
}

void Browser::go_back() {
    // TODO: Реализовать историю
    update_status_bar("Назад");
}

void Browser::go_forward() {
    // TODO: Реализовать историю
    update_status_bar("Вперед");
}

void Browser::create_tab() {
    create_web_view();
}

void Browser::close_tab(int tab_index) {
    if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) > 1) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), tab_index);
    }
}

void Browser::switch_tab(int tab_index) {
    update_status_bar("Вкладка " + std::to_string(tab_index + 1));
}

void Browser::set_user_agent(const std::string& user_agent) {
    this->user_agent = user_agent;
}

void Browser::enable_javascript(bool enable) {
    javascript_enabled = enable;
}

void Browser::enable_cookies(bool enable) {
    cookies_enabled = enable;
}

void Browser::block_popups(bool block) {
    popups_blocked = block;
}

void Browser::enable_https_only(bool enable) {
    https_only = enable;
}

void Browser::show_developer_tools() {
    update_status_bar("Инструменты разработчика");
}

void Browser::update_address_bar() {
    gtk_entry_set_text(GTK_ENTRY(address_bar), current_url.c_str());
}

void Browser::update_status_bar(const std::string& message) {
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, message.c_str());
}

void Browser::display_content(const std::string& content) {
    if (content_view) {
        // Проверяем, является ли content_view текстовым видом
        if (GTK_IS_TEXT_VIEW(content_view)) {
            GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(content_view));
            gtk_text_buffer_set_text(buffer, content.c_str(), -1);
        } else {
            // Если это не текстовый вид, создаем новый
            GtkWidget* scrolled_window = gtk_widget_get_parent(content_view);
            if (scrolled_window) {
                gtk_container_remove(GTK_CONTAINER(scrolled_window), content_view);
                
                // Создаем простой текстовый вид
                content_view = gtk_text_view_new();
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(content_view), GTK_WRAP_WORD_CHAR);
                gtk_text_view_set_editable(GTK_TEXT_VIEW(content_view), FALSE);
                
                GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(content_view));
                gtk_text_buffer_set_text(buffer, content.c_str(), -1);
                
                gtk_container_add(GTK_CONTAINER(scrolled_window), content_view);
                gtk_widget_show_all(scrolled_window);
            }
        }
    }
}

// Обработчик нажатий клавиш
gboolean Browser::on_window_key_press(GtkWidget* widget, GdkEventKey* event, Browser* browser) {
    if (!browser || !event) return FALSE;
    
    return browser->handle_key_shortcut(event->keyval, (GdkModifierType)event->state);
}

// Обработка сочетаний клавиш
bool Browser::handle_key_shortcut(guint keyval, GdkModifierType state) {
    // Ctrl+T - новая вкладка
    if ((state & GDK_CONTROL_MASK) && keyval == GDK_KEY_t) {
        create_tab();
        return TRUE;
    }
    
    // Ctrl+W - закрыть вкладку
    if ((state & GDK_CONTROL_MASK) && keyval == GDK_KEY_w) {
        // TODO: Закрыть текущую вкладку
        return TRUE;
    }
    
    // Ctrl+R или F5 - обновить
    if (((state & GDK_CONTROL_MASK) && keyval == GDK_KEY_r) || keyval == GDK_KEY_F5) {
        reload();
        return TRUE;
    }
    
    // Ctrl+L - фокус на адресную строку
    if ((state & GDK_CONTROL_MASK) && keyval == GDK_KEY_l) {
        gtk_widget_grab_focus(address_bar);
        gtk_editable_select_region(GTK_EDITABLE(address_bar), 0, -1);
        return TRUE;
    }
    
    // Ctrl+Tab - следующая вкладка
    if ((state & GDK_CONTROL_MASK) && keyval == GDK_KEY_Tab) {
        int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
        int num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
        if (num_pages > 1) {
            int next_page = (current_page + 1) % num_pages;
            gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), next_page);
        }
        return TRUE;
    }
    
    // Ctrl+Shift+Tab - предыдущая вкладка
    if ((state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK) && keyval == GDK_KEY_Tab) {
        int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
        int num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
        if (num_pages > 1) {
                    int prev_page = (current_page - 1 + num_pages) % num_pages;
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), prev_page);
        }
        return TRUE;
    }
    
    // F6 - фокус на контент
    if (keyval == GDK_KEY_F6) {
        if (content_view) {
            gtk_widget_grab_focus(content_view);
        }
        return TRUE;
    }
    
    // Escape - убрать фокус с адресной строки
    if (keyval == GDK_KEY_Escape) {
        if (gtk_widget_has_focus(address_bar)) {
            gtk_widget_grab_focus(main_window);
        }
        return TRUE;
    }
    
    return FALSE;
}

// Проверка кэша
bool Browser::is_cached(const std::string& url) {
    return page_cache.find(url) != page_cache.end();
}

// Кэширование страницы
void Browser::cache_page(const std::string& url, const std::string& content, const std::string& parsed) {
    // Ограничиваем размер кэша
    if (page_cache.size() > 50) {
        // Удаляем старые записи
        auto it = page_cache.begin();
        page_cache.erase(it);
        parsed_cache.erase(it->first);
    }
    
    page_cache[url] = content;
    parsed_cache[url] = parsed;
}

// Показать/скрыть прогресс загрузки
void Browser::show_loading_progress(bool show) {
    if (progress_bar) {
        gtk_widget_set_visible(progress_bar, show);
        if (show) {
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
        }
    }
}

// Обновить прогресс загрузки
void Browser::update_loading_progress(double progress) {
    if (progress_bar && gtk_widget_get_visible(progress_bar)) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), progress);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), 
                                 (std::to_string((int)(progress * 100)) + "%").c_str());
    }
}

// Статические обработчики событий
void Browser::on_navigate_clicked(GtkButton* button, Browser* browser) {
    const char* button_name = gtk_widget_get_name(GTK_WIDGET(button));
    
    if (strcmp(button_name, "go-previous") == 0) {
        browser->go_back();
    } else if (strcmp(button_name, "go-next") == 0) {
        browser->go_forward();
    } else if (strcmp(button_name, "view-refresh") == 0) {
        browser->reload();
    } else if (strcmp(button_name, "process-stop") == 0) {
        browser->stop();
    } else if (strcmp(button_name, "go-jump") == 0) {
        const char* url = gtk_entry_get_text(GTK_ENTRY(browser->address_bar));
        browser->navigate(url);
    }
}

void Browser::on_address_bar_activated(GtkEntry* entry, Browser* browser) {
    const char* url = gtk_entry_get_text(entry);
    browser->navigate(url);
}

// Обработчики событий
void Browser::on_back_clicked(GtkButton* button, Browser* browser) {
    // TODO: Реализовать навигацию назад
    std::cout << "Кнопка 'Назад' нажата" << std::endl;
}

void Browser::on_forward_clicked(GtkButton* button, Browser* browser) {
    // TODO: Реализовать навигацию вперед
    std::cout << "Кнопка 'Вперед' нажата" << std::endl;
}

void Browser::on_refresh_clicked(GtkButton* button, Browser* browser) {
    // Обновляем текущую страницу
    const char* url = gtk_entry_get_text(GTK_ENTRY(browser->address_bar));
    if (url && strlen(url) > 0) {
        browser->navigate(std::string(url));
    }
}

void Browser::on_new_tab_clicked(GtkButton* button, Browser* browser) {
    browser->create_new_tab();
}

void Browser::on_close_tab_clicked(GtkButton* button, Browser* browser) {
    int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(browser->notebook));
    if (current_page >= 0) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(browser->notebook), current_page);
    }
}

void Browser::on_address_bar_activate(GtkEntry* entry, Browser* browser) {
    const char* url = gtk_entry_get_text(entry);
    if (url && strlen(url) > 0) {
        browser->navigate(std::string(url));
    }
}

void Browser::on_page_added(GtkNotebook* notebook, GtkWidget* child, guint page_num, Browser* browser) {
    std::cout << "Добавлена вкладка " << page_num << std::endl;
}

void Browser::on_page_removed(GtkNotebook* notebook, GtkWidget* child, guint page_num, Browser* browser) {
    std::cout << "Удалена вкладка " << page_num << std::endl;
}

void Browser::on_switch_page(GtkNotebook* notebook, GtkWidget* page, guint page_num, Browser* browser) {
    std::cout << "Переключение на вкладку " << page_num << std::endl;
    // TODO: Обновить адресную строку и контент
}
