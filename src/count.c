#include <stdio.h>
#include <FreeRTOS.h>

#include "count.h"

bool print_and_increment(int *counter, SemaphoreHandle_t *semaphore)
{
    int temp;
    if (!xSemaphoreTake((*semaphore), portTICK_PERIOD_MS * 50))
        return false;
    {
        temp = (*counter);
        (*counter)++; // Original code prints the pre-increment value
    }
    xSemaphoreGive((*semaphore));

    printf("hello world from %s! Count %d\n", "main", temp);

    return true;
}

void deadlock_thread(void *params)
{
    deadlock_info_t *deadlock_info = (deadlock_info_t*)params;
    
    xSemaphoreTake(deadlock_info->a, portMAX_DELAY);
	{
        vTaskDelay(100); // Allow other thread to take semaphore

        xSemaphoreTake(deadlock_info->b, portMAX_DELAY);
        {
            printf("Deadlocked section, shouldn't be here.\n");
            deadlock_info->critical_section_hit = true;
        }
        xSemaphoreGive(deadlock_info->b);
    }
    xSemaphoreGive(deadlock_info->a);
}
