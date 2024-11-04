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

void test_orphaned_lock() {
    SemaphoreHandle_t a = xSemaphoreCreateCounting(1, 1);

    TaskHandle_t orphaned_lock;

    orphaned_lock_data_t orphaned_lock_data = { a, 1, 50 };

    xTaskCreate(orphaned_lock_thread, "Orphaned Lock", configMINIMAL_STACK_SIZE, &orphaned_lock_data, TEST_RUNNER_PRIORITY - 1, &orphaned_lock);

    vTaskDelay(500);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, uxSemaphoreGetCount(a), "Semaphore should be unavailable.");

    TEST_ASSERT_EQUAL_INT_MESSAGE(3, orphaned_lock_data.output, "Counting should have stalled during deadlock.");

    vTaskDelete(orphaned_lock);
}

void test_unorphaned_lock() {
    SemaphoreHandle_t a = xSemaphoreCreateCounting(1, 1);

    TaskHandle_t unorphaned_lock;

    orphaned_lock_data_t unorphaned_lock_data = { a, 1, 50 };

    xTaskCreate(unorphaned_lock_thread, "Orphaned Lock", configMINIMAL_STACK_SIZE, &unorphaned_lock_data, TEST_RUNNER_PRIORITY - 1, &unorphaned_lock);

    vTaskDelay(500);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, uxSemaphoreGetCount(a), "Semaphore should be available.");

    TEST_ASSERT_EQUAL_INT_MESSAGE(6, unorphaned_lock_data.output, "Counting should have completed.");
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
        RUN_TEST(test_orphaned_lock);
        RUN_TEST(test_unorphaned_lock);
        UNITY_END();
        sleep_ms(10000);
    }
}

int main (void)
{
    stdio_init_all();
    sleep_ms(5000); // Give time for TTY to attach.

    xTaskCreate(runner_thread, "TestRunner", configMINIMAL_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);
    vTaskStartScheduler();
    
    return 0;
}
