cmake_minimum_required(VERSION 3.20)
project(HeavenlyWebGu VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Настройки компилятора
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3")
endif()

# Поиск зависимостей
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# GTK для UI
pkg_check_modules(GTK REQUIRED gtk+-3.0)

# Исходные файлы
set(CPP_SOURCES
    src/cpp/main.cpp
    src/cpp/browser.cpp
    src/cpp/html_parser.cpp
    src/cpp/css_parser.cpp
    src/cpp/javascript_engine.cpp
    src/cpp/renderer.cpp
    src/cpp/network.cpp
    src/cpp/html_renderer.cpp
    src/cpp/browser_styles.cpp
    src/cpp/simple_html_renderer.cpp
)

set(C_SOURCES
    src/c/system.c
    src/c/platform.c
    src/c/memory.c
)

# Создание исполняемого файла
add_executable(HeavenlyWebGu ${CPP_SOURCES} ${C_SOURCES})

# Подключение библиотек
target_link_libraries(HeavenlyWebGu
    OpenGL::GL
    glfw
    ${GTK_LIBRARIES}
)

# Подключение заголовочных файлов
target_include_directories(HeavenlyWebGu PRIVATE
    src/
    src/cpp/
    src/c/
    ${GTK_INCLUDE_DIRS}
)

# Компиляция Rust компонентов
add_custom_target(rust_components ALL
    COMMAND cargo build --release --manifest-path src/rust/Cargo.toml
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Сборка Rust компонентов"
)

# Зависимость от Rust
add_dependencies(HeavenlyWebGu rust_components)

# Линковка с Rust библиотекой и macOS фреймворками
target_link_libraries(HeavenlyWebGu
    ${CMAKE_SOURCE_DIR}/src/rust/target/release/libheavenly_webgu_rust.a
)

# Добавляем macOS фреймворки
if(APPLE)
    target_link_libraries(HeavenlyWebGu
        "-framework Security"
        "-framework CoreFoundation"
        "-framework SystemConfiguration"
    )
endif()
