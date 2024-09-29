#ifndef COUNT_H
#define COUNT_H
#include <semphr.h>

typedef struct {
    SemaphoreHandle_t a;
    SemaphoreHandle_t b;
    bool critical_section_hit;
} deadlock_info_t;

bool double_plus1_and_print(int *counter, SemaphoreHandle_t *semaphore);
bool print_and_increment(int *counter, SemaphoreHandle_t *semaphore);

void deadlock_thread(void *params);

#endif