#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"

#include <FreeRTOS.h>
#include "count.h"

SemaphoreHandle_t semaphore;

void setUp(void) {
    semaphore = xSemaphoreCreateCounting(1, 1);
}

void tearDown(void) {}

void test_semaphore_timeout_double_plus1() {
    // Starve the test function so it times out (simulates another thread having it)
    xSemaphoreTake(semaphore, portMAX_DELAY);

    int count = 1;
    bool success = double_plus1_and_print(&count, &semaphore);
    xSemaphoreGive(semaphore);

    TEST_ASSERT_FALSE_MESSAGE(success, "Count was not properly locked, did not timeout.");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(1, count, "Count was modified even though it should have been locked.");
}

void test_semaphore_timeout_increment() {
    // Starve the test function so it times out (simulates another thread having it)
    xSemaphoreTake(semaphore, portMAX_DELAY);

    int count = 1;
    bool success = print_and_increment(&count, &semaphore);
    xSemaphoreGive(semaphore);

    TEST_ASSERT_FALSE_MESSAGE(success, "Count was not properly locked, did not timeout.");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(1, count, "Count was modified even though it should have been locked.");
}

void test_double_plus1() {
    int count = 2;

    bool success = double_plus1_and_print(&count, &semaphore);

    TEST_ASSERT_TRUE_MESSAGE(success, "Unsuccessful in taking semaphore.");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(5, count, "Count not modified properly");
}

void test_increment() {
    int count = 2;

    bool success = print_and_increment(&count, &semaphore);

    TEST_ASSERT_TRUE_MESSAGE(success, "Unsuccessful in taking semaphore.");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(3, count, "Count not modified properly");
}

void test_deadlock() {
    SemaphoreHandle_t a, b;
    a = xSemaphoreCreateCounting(1, 1);
    b = xSemaphoreCreateCounting(1, 1);

    TaskHandle_t deadlock1, deadlock2;

    deadlock_info_t deadlock_info1 = { a, b, deadlock1 };
    deadlock_info_t deadlock_info2 = { b, a, deadlock2 };

    printf("Scheduling threads.\n");

    xTaskCreate(deadlock_thread, "Deadlock 1",
                configMINIMAL_STACK_SIZE, &deadlock_info1, tskIDLE_PRIORITY + 1, &deadlock1);
    xTaskCreate(deadlock_thread, "Deadlock 2",
                configMINIMAL_STACK_SIZE, &deadlock_info2, tskIDLE_PRIORITY + 1, &deadlock2);
    vTaskStartScheduler();

    printf("Threads scheduled.\n");

    vTaskDelay(100);

    vTaskSuspend(deadlock1);
    vTaskSuspend(deadlock2);

    TEST_ASSERT_TRUE_MESSAGE(isBlocked(deadlock1), "Thread 1 failed to stay blocked.");
    TEST_ASSERT_TRUE_MESSAGE(isBlocked(deadlock2), "Thread 2 failed to stay blocked.");

    vTaskDelete(deadlock1);
    vTaskDelete(deadlock2);
}

int main (void)
{
    stdio_init_all();
    sleep_ms(5000); // Give time for TTY to attach.
    printf("Start tests\n");
    UNITY_BEGIN();
    RUN_TEST(test_semaphore_timeout_double_plus1);
    RUN_TEST(test_semaphore_timeout_increment);
    RUN_TEST(test_double_plus1);
    RUN_TEST(test_increment);
    RUN_TEST(test_deadlock);
    sleep_ms(5000);
    return UNITY_END();
}
