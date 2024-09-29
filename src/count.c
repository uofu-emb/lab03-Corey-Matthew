#include <stdio.h>
#include <FreeRTOS.h>

#include "count.h"

bool isBlocked(TaskHandle_t task) {
    TaskStatus_t status;
    
    vTaskSuspend(task);
    vTaskGetInfo(task, &status, pdFALSE, eInvalid);
    vTaskResume(task);

    return status.eCurrentState == eBlocked;
}

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

void deadlock_thread(void *params)
{
    deadlock_info_t deadlock_info = (*(deadlock_info_t*)(params));
    printf("Thread started.\n");

    xSemaphoreTake(deadlock_info.a, portMAX_DELAY);
	{
        printf("1st Semaphore taken.\n");
        // Delay to ensure blocking
        while (!isBlocked(deadlock_info.otherTask)) {
            vTaskDelay(10);
            printf("Still waiting\n");
        }

        printf("Here\n");

        xSemaphoreTake(deadlock_info.b, portMAX_DELAY);
        {
            printf("Should not be here.\n");
            // Important stuff
        }
        xSemaphoreGive(deadlock_info.b);
    }
    xSemaphoreGive(deadlock_info.a);
}
