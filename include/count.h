#ifndef COUNT_H
#define COUNT_H
#include <semphr.h>

bool double_plus1_and_print(int *counter, SemaphoreHandle_t *semaphore);
bool print_and_increment(int *counter, SemaphoreHandle_t *semaphore);

#endif