#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "i2c_bus/i2c_bus.h"

static const char TAG[] = "MainBox";



void app_main(void) {
    ESP_LOGI(TAG, "Starting!");

    i2c_bus_init();

    while(1) {
        vTaskDelay(1);
    }
}
