#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"

#include <FreeRTOS.h>
#include "count.h"

#define TEST_RUNNER_PRIORITY ( tskIDLE_PRIORITY + 2UL )

SemaphoreHandle_t semaphore;

void setUp(void) {
    semaphore = xSemaphoreCreateCounting(1, 1);
}

void tearDown(void) {}

void test_semaphore_timeout_increment() {
    // Starve the test function so it times out (simulates another thread having it)
    xSemaphoreTake(semaphore, portMAX_DELAY);

    int count = 1;
    bool success = print_and_increment(&count, &semaphore);
    xSemaphoreGive(semaphore);

    TEST_ASSERT_FALSE_MESSAGE(success, "Count was not properly locked, did not timeout.");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(1, count, "Count was modified even though it should have been locked.");
}

void test_increment() {
    int count = 1;

    bool success = print_and_increment(&count, &semaphore);

    TEST_ASSERT_TRUE_MESSAGE(success, "Unsuccessful in taking semaphore.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, count, "Count not modified properly");
}

void test_deadlock() {
    SemaphoreHandle_t a = xSemaphoreCreateCounting(1, 1);
    SemaphoreHandle_t b = xSemaphoreCreateCounting(1, 1);

    TaskHandle_t deadlock1, deadlock2;

    deadlock_info_t deadlock_info1 = { a, b, false };
    deadlock_info_t deadlock_info2 = { b, a, false };

    xTaskCreate(deadlock_thread, "Deadlock 1", configMINIMAL_STACK_SIZE, &deadlock_info1, TEST_RUNNER_PRIORITY - 1, &deadlock1);
    xTaskCreate(deadlock_thread, "Deadlock 2", configMINIMAL_STACK_SIZE, &deadlock_info2, TEST_RUNNER_PRIORITY - 1, &deadlock2);

    vTaskDelay(500);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, uxSemaphoreGetCount(a), "Semaphore should be unavailable.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, uxSemaphoreGetCount(b), "Semaphore should be unavailable.");

    TEST_ASSERT_FALSE_MESSAGE(deadlock_info1.critical_section_hit, "Thread 1 failed to stay blocked.");
    TEST_ASSERT_FALSE_MESSAGE(deadlock_info2.critical_section_hit, "Thread 2 failed to stay blocked.");

    vTaskDelete(deadlock1);
    vTaskDelete(deadlock2);
}

void runner_thread(__unused void *args)
{
    // Restarts the tests if we miss the window for TTY output
    for (;;) {
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_semaphore_timeout_increment);
        RUN_TEST(test_increment);
        RUN_TEST(test_deadlock);
        UNITY_END();
        sleep_ms(10000);
    }
}

int main (void)
{
    stdio_init_all();
    sleep_ms(5000); // Give time for TTY to attach.
    printf("Start tests\n");

    xTaskCreate(runner_thread, "TestRunner", configMINIMAL_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);
    vTaskStartScheduler();
    
    return 0;
}
