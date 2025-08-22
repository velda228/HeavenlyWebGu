#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* custom_malloc(size_t size);
void custom_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_H
