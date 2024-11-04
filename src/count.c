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

void orphaned_lock_thread(void *params) {
    orphaned_lock_data_t *orphaned_lock_data = (orphaned_lock_data_t*)params;

    orphaned_lock_data->output = orphaned_lock_data->input;

    for (int i = 0; i < 5; i++) {
        xSemaphoreTake(orphaned_lock_data->a, portMAX_DELAY);
        orphaned_lock_data->output++;
        if (orphaned_lock_data->output % 2) {
            continue;
        }
        printf("Count %d\n", orphaned_lock_data->output);
        xSemaphoreGive(orphaned_lock_data->a);
    }
}

void unorphaned_lock_thread(void *params) {
    orphaned_lock_data_t *orphaned_lock_data = (orphaned_lock_data_t*)params;

    orphaned_lock_data->output = orphaned_lock_data->input;

    for (int i = 0; i < 5; i++) {
        xSemaphoreTake(orphaned_lock_data->a, portMAX_DELAY);
        orphaned_lock_data->output++;
        if (orphaned_lock_data->output % 2) {
            xSemaphoreGive(orphaned_lock_data->a);
            continue;
        }
        printf("Count %d\n", orphaned_lock_data->output);
        xSemaphoreGive(orphaned_lock_data->a);
    }

    vTaskDelete(NULL);
}
