#include "memory.h"
#include <stdlib.h>

void* custom_malloc(size_t size) {
    return malloc(size);
}

void custom_free(void* ptr) {
    free(ptr);
}
