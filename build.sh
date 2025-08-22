#!/bin/bash

echo "Сборка HeavenlyWebGu..."

# Проверяем наличие необходимых инструментов
if ! command -v cmake &> /dev/null; then
    echo "Ошибка: cmake не найден. Установите зависимости: ./install_dependencies.sh"
    exit 1
fi

if ! command -v cargo &> /dev/null; then
    echo "Ошибка: cargo не найден. Установите Rust: ./install_dependencies.sh"
    exit 1
fi

# Создаем директорию для сборки
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Очищаем предыдущую сборку
echo "Очистка предыдущей сборки..."
rm -rf *

# Конфигурируем CMake
echo "Конфигурация CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "Ошибка конфигурации CMake"
    exit 1
fi

# Собираем проект
echo "Сборка проекта..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Ошибка сборки"
    exit 1
fi

echo ""
echo "Сборка завершена успешно!"
echo "Исполняемый файл: build/HeavenlyWebGu"
echo ""
echo "Для запуска выполните:"
echo "cd build && ./HeavenlyWebGu"
