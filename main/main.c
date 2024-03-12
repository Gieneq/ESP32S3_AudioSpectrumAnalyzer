#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char TAG[] = "MainBox";



void app_main(void) {
    ESP_LOGI(TAG, "Starting!");


    while(1) {
        vTaskDelay(1);
    }
}
