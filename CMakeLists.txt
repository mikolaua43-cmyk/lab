cmake_minimum_required(VERSION 3.10)

# Название проекта
project(ParentChildProcess C)

# Настройка стандарта C11 и флагов компиляции для Поденка Л.П.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

# Создаем основной исполняемый файл
add_executable(parent lab2.c)

# После сборки создаем копию файла под именем 'child', 
# чтобы execve мог его найти и запустить
add_custom_command(TARGET parent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:parent> ${CMAKE_CURRENT_BINARY_DIR}/child
    COMMENT "Создание копии исполняемого файла с именем 'child' для корректной работы execve"
)

# Вывод подсказки пользователю
message(STATUS "Для запуска программы не забудьте создать файл 'env' в папке сборки.")
