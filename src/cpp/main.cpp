#include "browser.h"
#include <iostream>
#include <gtk/gtk.h>

int main(int argc, char* argv[]) {
    std::cout << "Запуск HeavenlyWebGu..." << std::endl;
    
    try {
        // Создаем экземпляр браузера
        Browser browser;
        
        // Инициализируем браузер
        if (!browser.initialize()) {
            std::cerr << "Ошибка инициализации браузера" << std::endl;
            return 1;
        }
        
        std::cout << "Браузер успешно инициализирован" << std::endl;
        
        // Загружаем стартовую страницу
        browser.navigate("https://www.google.com");
        
        // Запускаем главный цикл GTK
        gtk_main();
        
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Неизвестная ошибка" << std::endl;
        return 1;
    }
    
    std::cout << "Браузер завершил работу" << std::endl;
    return 0;
}
