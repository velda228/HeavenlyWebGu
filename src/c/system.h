#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// Системная информация
typedef struct {
    char os_name[64];
    char os_version[64];
    char architecture[32];
    size_t total_memory;
    size_t available_memory;
    int cpu_count;
} SystemInfo;

// Функции для работы с системой
SystemInfo get_system_info(void);
void print_system_info(const SystemInfo* info);

// Функции для работы с памятью
void* safe_malloc(size_t size);
void* safe_calloc(size_t nmemb, size_t size);
void* safe_realloc(void* ptr, size_t size);
void safe_free(void* ptr);

// Функции для работы с файлами
int file_exists(const char* path);
size_t file_size(const char* path);
int read_file(const char* path, char** buffer, size_t* size);
int write_file(const char* path, const char* data, size_t size);

// Функции для работы с процессами
int get_process_id(void);
int get_thread_id(void);
void sleep_ms(int milliseconds);

// Функции для работы с сетью
int is_network_available(void);
int get_local_ip(char* buffer, size_t buffer_size);
int resolve_hostname(const char* hostname, char* ip, size_t ip_size);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H
