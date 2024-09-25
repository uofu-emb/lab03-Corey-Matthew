#include <stdio.h>
#include <FreeRTOS.h>

#include "count.h"

bool double_plus1_and_print(int *counter, SemaphoreHandle_t *semaphore)
{
    int temp;
    if (!xSemaphoreTake((*semaphore), portTICK_PERIOD_MS * 50))
        return false;
    {
        (*counter) += (*counter) + 1;
        temp = (*counter);
    }
    xSemaphoreGive((*semaphore));

    printf("hello world from %s! Count %d\n", "thread", temp);

    return true;
}

bool print_and_increment(int *counter, SemaphoreHandle_t *semaphore)
{
    int temp;
    if (!xSemaphoreTake((*semaphore), portTICK_PERIOD_MS * 50))
        return false;
    {
        temp = (*counter);
        (*counter)++; // We want to print the pre-increment value
    }
    xSemaphoreGive((*semaphore));

    printf("hello world from %s! Count %d\n", "main", temp);

    return true;
}
