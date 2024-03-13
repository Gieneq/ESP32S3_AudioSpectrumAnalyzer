#include <stdio.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "bsp/esp-bsp.h"
#include "bsp_board.h"

#include "settings.h"

#include "tools/i2c_bus_scan.h"
#include "display.h"
#include "fft_convert.h"

static const char TAG[] = "MainBox";

// const uint32_t sample_rates[] = {DEFAULT_SAMPLE_RATE};
// static uint32_t s_sample_rate = DEFAULT_SAMPLE_RATE;
// #define N_SAMPLE_RATES  TU_ARRAY_SIZE(sample_rates)
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;
#define UAC_ENTER_CRITICAL()    portENTER_CRITICAL(&s_mux)
#define UAC_EXIT_CRITICAL()     portEXIT_CRITICAL(&s_mux)

// const uint8_t spk_resolutions_per_format[CFG_TUD_AUDIO_FUNC_1_N_FORMATS] = {CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX
//                                                                            };
const uint8_t mic_resolutions_per_format[CFG_TUD_AUDIO_FUNC_1_N_FORMATS] = 
{CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX};

// static uint8_t s_spk_resolution = spk_resolutions_per_format[0];
static uint8_t s_mic_resolution = CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX;
// static uint8_t s_mic_resolution = mic_resolutions_per_format[0];

static int16_t s_mic_buf1[CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ / 2] = {0};
static int16_t s_mic_buf2[CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ / 2] = {0};
static int16_t *s_mic_write_buf = s_mic_buf1;
static int16_t *s_mic_read_buf = s_mic_buf2;                                         

volatile static size_t s_mic_read_buf_len = 0;
static size_t s_spk_bytes_ms = 0;
static size_t s_mic_bytes_ms = 0;


volatile static bool s_mic_active;
static TaskHandle_t mic_task_handle;
static TaskHandle_t spk_task_handle;



static void usb_headset_spk(void *pvParam)
{
    while (1) {
        vTaskDelay(100);
        // SYSVIEW_SPK_WAIT_EVENT_START();
        // if (s_spk_active == false) {
        //     ulTaskNotifyTake(pdFAIL, portMAX_DELAY);
        //     continue;
        // }
        // // clear the notification
        // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // if (s_spk_buf_len == 0) {
        //     continue;
        // }
        // // playback the data from the ring buffer chunk by chunk
        // SYSVIEW_SPK_WAIT_EVENT_END();
        // SYSVIEW_SPK_SEND_EVENT_START();
        // size_t bytes_written = 0;
        // bsp_i2s_write(s_spk_buf, s_spk_buf_len, &bytes_written, 0);
        // for (int i = 0; i < bytes_written / 2; i ++) {
        //     rb_write(s_spk_buf + i, 2);
        // }
        // s_spk_buf_len = 0;
        // SYSVIEW_SPK_SEND_EVENT_END();
    }
}

static void usb_headset_mic(void *pvParam)
{
    while (1) {
        // if (s_mic_active == false) {
        //     ulTaskNotifyTake(pdFAIL, portMAX_DELAY);
        //     continue;
        // }
        // // clear the notification
        // ulTaskNotifyTake(pdTRUE, 0);
        // read data from the microphone chunk by chunk
        // SYSVIEW_MIC_READ_EVENT_START();
        size_t bytes_read = 0;
        size_t bytes_require = 1024;
        // size_t bytes_require = s_mic_bytes_ms * (CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_MS - 1);
        esp_err_t ret = bsp_i2s_read(s_mic_write_buf, bytes_require, &bytes_read, 0);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read data from I2S, ret = %d", ret);
            // SYSVIEW_MIC_READ_EVENT_END();
            continue;
        }
        ESP_LOGD(TAG, "Got %u / %u B", bytes_read, bytes_require);
        rb_write(s_mic_write_buf, bytes_read);
        // // swap the buffer
        // int16_t *tmp_buf = s_mic_read_buf;
        // UAC_ENTER_CRITICAL();
        // s_mic_read_buf = s_mic_write_buf;
        // s_mic_read_buf_len = bytes_read;
        // s_mic_write_buf = tmp_buf;
        // UAC_EXIT_CRITICAL();
        // // SYSVIEW_MIC_READ_EVENT_END();
        vTaskDelay(1);
    }
}




/* Can be used for i2s_std_gpio_config_t and/or i2s_std_config_t initialization */
#define BSP_I2S_GPIO_CFG       \
    {                          \
        .mclk = BSP_I2S_MCLK,  \
        .bclk = BSP_I2S_SCLK,  \
        .ws = BSP_I2S_LCLK,    \
        .dout = BSP_I2S_DOUT,  \
        .din = BSP_I2S_DSIN,   \
        .invert_flags = {      \
            .mclk_inv = false, \
            .bclk_inv = false, \
            .ws_inv = false,   \
        },                     \
    }

/* This configuration is used by default in bsp_audio_init() */
#define BSP_I2S_DUPLEX_MONO_CFG(_sample_rate)                                                         \
    {                                                                                                 \
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(_sample_rate),                                          \
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO), \
        .gpio_cfg = BSP_I2S_GPIO_CFG,                                                                 \
    }

void app_main(void) {
    ESP_LOGI(TAG, "Starting!");

#if CONFIG_SCAN_I2C_BUS_ON_STARTUP
    i2c_bus_scan();
#endif
    /* BSP related */

    /* Initialize I2C (for touch and audio) */
    bsp_i2c_init();

    /* Initialize display */
    display_lcd_init();

    /* Init fft */
    fft_convert_init();

    /* Initialize audio i2s */
    i2s_std_config_t i2s_config = BSP_I2S_DUPLEX_MONO_CFG(DEFAULT_SAMPLE_RATE);
    i2s_config.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_384;
    bsp_audio_init(&i2s_config);

    /* Initialize bsp board */
    bsp_board_init();

    /* Initialize codec with defaults */
    bsp_codec_set_fs(DEFAULT_SAMPLE_RATE, DEFAULT_PLAYER_WIDTH, DEFAULT_PLAYER_CHANNEL);
    bsp_codec_volume_set(DEFAULT_VOLUME, NULL);
    bsp_codec_mute_set(false);

    // int16_t buff[TMP_SMAPLES_COUNT] = {0};

    // for(int i=0; i<TMP_SMAPLES_COUNT; ++i) {
    //     buff[i] = i * 113;
    // }
    
    //48000 / 1000 * 1 * 1 / 8  --> 6
    // s_mic_bytes_ms = s_sample_rate / 1000 * s_mic_resolution * DEFAULT_RECORDER_CHANNEL / 8;
    // s_spk_bytes_ms = s_sample_rate / 1000 * s_spk_resolution * DEFAULT_PLAYER_CHANNEL / 8;
    s_mic_bytes_ms = 6;
    s_spk_bytes_ms = 6;
    // we give the higher priority to playback task, to avoid the data pending in the ring buffer
    BaseType_t ret_val = xTaskCreate(usb_headset_spk, "usb_headset_spk", 4 * 1024, NULL, 8, &spk_task_handle);
    if (ret_val != pdPASS) {
        ESP_LOGE(TAG, "Failed to create usb_headset_spk task");
        ESP_ERROR_CHECK(ESP_FAIL);
    }
    // we give the lower priority to record task, to avoid the data pending in the ring buffer
    ret_val = xTaskCreate(usb_headset_mic, "usb_headset_mic", 4 * 1024, NULL, 8, &mic_task_handle);
    if (ret_val != pdPASS) {
        ESP_LOGE(TAG, "Failed to create usb_headset_mic task");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    while(1) {
        // rb_write(buff, TMP_SMAPLES_COUNT);
        vTaskDelay(1);
    }
}