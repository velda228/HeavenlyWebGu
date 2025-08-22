#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef __linux__
#include <sys/sysinfo.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#endif
#include <sys/utsname.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

SystemInfo get_system_info(void) {
    SystemInfo info = {0};
    
    // Получаем информацию об ОС
    struct utsname uts;
    if (uname(&uts) == 0) {
        strncpy(info.os_name, uts.sysname, sizeof(info.os_name) - 1);
        strncpy(info.os_version, uts.release, sizeof(info.os_version) - 1);
        strncpy(info.architecture, uts.machine, sizeof(info.architecture) - 1);
    }
    
    // Получаем информацию о памяти
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        info.total_memory = (size_t)si.totalram * si.mem_unit;
        info.available_memory = (size_t)si.freeram * si.mem_unit;
    }
#elif defined(__APPLE__)
    // macOS использует sysctl
    uint64_t total_mem;
    size_t len = sizeof(total_mem);
    if (sysctlbyname("hw.memsize", &total_mem, &len, NULL, 0) == 0) {
        info.total_memory = total_mem;
        info.available_memory = total_mem; // Упрощенно
    }
#endif
    
    // Получаем количество CPU
    info.cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    
    return info;
}

void print_system_info(const SystemInfo* info) {
    printf("=== Системная информация ===\n");
    printf("ОС: %s %s\n", info->os_name, info->os_version);
    printf("Архитектура: %s\n", info->architecture);
    printf("Память: %zu MB / %zu MB\n", 
           info->available_memory / (1024 * 1024),
           info->total_memory / (1024 * 1024));
    printf("CPU: %d ядер\n", info->cpu_count);
}

void* safe_malloc(size_t size) {
    if (size == 0) return NULL;
    
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Ошибка выделения памяти: %zu байт\n", size);
        exit(1);
    }
    return ptr;
}

void* safe_calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) return NULL;
    
    void* ptr = calloc(nmemb, size);
    if (ptr == NULL) {
        fprintf(stderr, "Ошибка выделения памяти: %zu x %zu байт\n", nmemb, size);
        exit(1);
    }
    return ptr;
}

void* safe_realloc(void* ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL) {
        fprintf(stderr, "Ошибка изменения размера памяти: %zu байт\n", size);
        exit(1);
    }
    return new_ptr;
}

void safe_free(void* ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
}

int file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

size_t file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

int read_file(const char* path, char** buffer, size_t* size) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return -1;
    }
    
    // Получаем размер файла
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Выделяем память
    *buffer = (char*)safe_malloc(*size + 1);
    
    // Читаем файл
    size_t bytes_read = fread(*buffer, 1, *size, file);
    fclose(file);
    
    if (bytes_read != *size) {
        safe_free(*buffer);
        return -1;
    }
    
    (*buffer)[*size] = '\0';
    return 0;
}

int write_file(const char* path, const char* data, size_t size) {
    FILE* file = fopen(path, "wb");
    if (!file) {
        return -1;
    }
    
    size_t bytes_written = fwrite(data, 1, size, file);
    fclose(file);
    
    return (bytes_written == size) ? 0 : -1;
}

int get_process_id(void) {
    return getpid();
}

int get_thread_id(void) {
    return (int)pthread_self();
}

void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}

int is_network_available(void) {
    // Простая проверка - пытаемся разрешить localhost
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo("localhost", NULL, &hints, &result);
    if (status == 0) {
        freeaddrinfo(result);
        return 1;
    }
    return 0;
}

int get_local_ip(char* buffer, size_t buffer_size) {
    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) {
        return -1;
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
            
            if (inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN) != NULL) {
                // Пропускаем localhost
                if (strcmp(ip, "127.0.0.1") != 0) {
                    strncpy(buffer, ip, buffer_size - 1);
                    buffer[buffer_size - 1] = '\0';
                    freeifaddrs(ifaddr);
                    return 0;
                }
            }
        }
    }
    
    freeifaddrs(ifaddr);
    return -1;
}

int resolve_hostname(const char* hostname, char* ip, size_t ip_size) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(hostname, NULL, &hints, &result);
    if (status != 0) {
        return -1;
    }
    
    struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;
    if (inet_ntop(AF_INET, &(addr->sin_addr), ip, ip_size) == NULL) {
        freeaddrinfo(result);
        return -1;
    }
    
    freeaddrinfo(result);
    return 0;
}
