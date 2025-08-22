#!/bin/bash

echo "Установка зависимостей для HeavenlyWebGu..."

# Проверяем ОС
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Обнаружена Linux система"
    
    # Ubuntu/Debian
    if command -v apt-get &> /dev/null; then
        echo "Установка зависимостей для Ubuntu/Debian..."
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            pkg-config \
            libgtk-3-dev \
            libwebkit2gtk-4.0-dev \
            libglfw3-dev \
            libopengl-dev \
            libssl-dev \
            libcurl4-openssl-dev \
            git \
            curl
        
    # Fedora
    elif command -v dnf &> /dev/null; then
        echo "Установка зависимостей для Fedora..."
        sudo dnf install -y \
            gcc-c++ \
            cmake \
            pkg-config \
            gtk3-devel \
            webkit2gtk3-devel \
            glfw-devel \
            mesa-libGL-devel \
            openssl-devel \
            libcurl-devel \
            git \
            curl
    
    # Arch Linux
    elif command -v pacman &> /dev/null; then
        echo "Установка зависимостей для Arch Linux..."
        sudo pacman -S --needed \
            base-devel \
            cmake \
            pkg-config \
            gtk3 \
            webkit2gtk \
            glfw-x11 \
            mesa \
            openssl \
            curl \
            git
    
    else
        echo "Неизвестный дистрибутив Linux. Установите зависимости вручную."
        exit 1
    fi

elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Обнаружена macOS система"
    
    # Проверяем наличие Homebrew
    if ! command -v brew &> /dev/null; then
        echo "Установка Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    echo "Установка зависимостей для macOS..."
    brew install \
        cmake \
        pkg-config \
        gtk+3 \
        webkitgtk \
        glfw \
        openssl \
        curl \
        git

else
    echo "Неподдерживаемая ОС: $OSTYPE"
    exit 1
fi

# Установка Rust
if ! command -v cargo &> /dev/null; then
    echo "Установка Rust..."
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source ~/.cargo/env
else
    echo "Rust уже установлен"
fi

# Обновление Rust
echo "Обновление Rust..."
rustup update

# Установка дополнительных компонентов Rust
echo "Установка дополнительных компонентов Rust..."
rustup component add rustfmt clippy

echo "Все зависимости установлены!"
echo ""
echo "Для сборки проекта выполните:"
echo "mkdir build && cd build"
echo "cmake .."
echo "make -j\$(nproc)"
echo ""
echo "Для запуска:"
echo "./HeavenlyWebGu"
