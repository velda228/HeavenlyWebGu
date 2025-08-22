#pragma once

#include <string>
#include <memory>
#include <vector>
#include <gtk/gtk.h>
#include "rust_html_renderer.h"

// FFI интерфейсы для Rust
extern "C" {
    struct HtmlParser;
    struct CssParser;
    
    HtmlParser* html_parse_new();
    void html_parse_free(HtmlParser* parser);
    char* html_parse_string(HtmlParser* parser, const char* html);
    
    // Новые функции для работы с элементами
    size_t html_get_element_count(HtmlParser* parser);
    char* html_get_element_tag_name(HtmlParser* parser, size_t index);
    char* html_get_element_text(HtmlParser* parser, size_t index);
    char* html_get_element_attribute(HtmlParser* parser, size_t element_index, const char* attr_name);
    size_t html_get_element_attribute_count(HtmlParser* parser, size_t element_index);
    char* html_get_element_attribute_name(HtmlParser* parser, size_t element_index, size_t attr_index);
    
    char* network_fetch_url(const char* url);
    char* network_fetch_image(const char* url);
    
    // Асинхронные сетевые функции
    struct AsyncFetchHandle;
    AsyncFetchHandle* network_fetch_url_async(const char* url);
    int network_fetch_url_check(AsyncFetchHandle* handle);
    char* network_fetch_url_result(AsyncFetchHandle* handle);
    
    void string_free(char* ptr);
}

class Browser {
public:
    Browser();
    ~Browser();
    
    // Основные функции браузера
    bool initialize();
    void navigate(const std::string& url);
    void reload();
    void stop();
    void go_back();
    void go_forward();
    
    // Управление вкладками
    void create_tab();
    void close_tab(int tab_index);
    void switch_tab(int tab_index);
    
    // Настройки
    void set_user_agent(const std::string& user_agent);
    void enable_javascript(bool enable);
    void enable_cookies(bool enable);
    
    // Безопасность
    void block_popups(bool block);
    void enable_https_only(bool enable);
    
    // UI
    GtkWidget* get_main_window() const { return main_window; }
    void show_developer_tools();
    
    // Прогресс загрузки
    void show_loading_progress(bool show);
    void update_loading_progress(double progress);
    
    // Кэширование
    bool is_cached(const std::string& url);
    void cache_page(const std::string& url, const std::string& content, const std::string& parsed);
    
private:
    // GTK виджеты
    GtkWidget* main_window;
    GtkWidget* notebook;
    GtkWidget* toolbar;
    GtkWidget* address_bar;
    GtkWidget* status_bar;
    GtkWidget* content_view;
    GtkWidget* progress_bar;
    
    // Rust компоненты
    HtmlParser* html_parser;
    CssParser* css_parser;
    
    // HTML рендерер
    RustHtmlRenderer* html_renderer;
    
    // Состояние браузера
    std::string current_url;
    std::string user_agent;
    bool javascript_enabled;
    bool cookies_enabled;
    bool popups_blocked;
    bool https_only;
    
    // Кэш для быстрой загрузки
    std::map<std::string, std::string> page_cache;
    std::map<std::string, std::string> parsed_cache;
    
    // Асинхронная загрузка
    AsyncFetchHandle* current_fetch_handle;
    std::string pending_url;
    guint loading_timer_id;
    
    // Приватные методы
    void setup_ui();
    void setup_signals();
    void create_web_view();
    void create_new_tab();
    void update_address_bar();
    void update_status_bar(const std::string& message);
    void display_content(const std::string& content);
    
    // Асинхронная загрузка
    void navigate_async(const std::string& url);
    void check_loading_progress();
    static gboolean on_loading_timer(gpointer data);
    
    // Обработчики событий
    static void on_back_clicked(GtkButton* button, Browser* browser);
    static void on_forward_clicked(GtkButton* button, Browser* browser);
    static void on_refresh_clicked(GtkButton* button, Browser* browser);
    static void on_new_tab_clicked(GtkButton* button, Browser* browser);
    static void on_close_tab_clicked(GtkButton* button, Browser* browser);
    static void on_address_bar_activate(GtkEntry* entry, Browser* browser);
    static void on_page_added(GtkNotebook* notebook, GtkWidget* child, guint page_num, Browser* browser);
    static void on_page_removed(GtkNotebook* notebook, GtkWidget* child, guint page_num, Browser* browser);
    static void on_switch_page(GtkNotebook* notebook, GtkWidget* page, guint page_num, Browser* browser);
    static void on_navigate_clicked(GtkButton* button, Browser* browser);
    static void on_address_bar_activated(GtkEntry* entry, Browser* browser);
    static gboolean on_window_key_press(GtkWidget* widget, GdkEventKey* event, Browser* browser);
    
    // Обработка сочетаний клавиш
    bool handle_key_shortcut(guint keyval, GdkModifierType state);
};
