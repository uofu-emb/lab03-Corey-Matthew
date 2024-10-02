#ifndef COUNT_H
#define COUNT_H
#include <semphr.h>

typedef struct {
    SemaphoreHandle_t a;
    SemaphoreHandle_t b;
    bool critical_section_hit;
} deadlock_info_t;

typedef struct {
    SemaphoreHandle_t a;
    int32_t input;
    int32_t output;
} orphaned_lock_data_t;

bool print_and_increment(int *counter, SemaphoreHandle_t *semaphore);

void deadlock_thread(void *params);

void orphaned_lock_thread(void *params);

void unorphaned_lock_thread(void *params);

#endif