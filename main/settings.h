#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

#define DEFAULT_SAMPLE_RATE         (48000)
#define DEFAULT_VOLUME              (99)
// Currently the player and recorder should use the same channel and width
#define DEFAULT_RECORDER_CHANNEL    (1)
#define DEFAULT_RECORDER_WIDTH      (16)
#define DEFAULT_PLAYER_CHANNEL      (1)
#define DEFAULT_PLAYER_WIDTH        (16)
#define DEBUG_USB_HEADSET           (0)
#define DEBUG_SYSTEM_VIEW           (0)

#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS                               1
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE                         DEFAULT_SAMPLE_RATE   // 24bit/48kHz is the best quality for full-speed, high-speed is needed beyond this
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX                           DEFAULT_RECORDER_CHANNEL
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX                           DEFAULT_PLAYER_CHANNEL

// 16bit in 16bit slots
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX          (DEFAULT_RECORDER_WIDTH/8)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX                  DEFAULT_RECORDER_WIDTH
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX          (DEFAULT_PLAYER_WIDTH/8)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX                  DEFAULT_PLAYER_WIDTH

// EP and buffer size - for isochronous EP´s, the buffer and EP size are equal (different sizes would not make sense)
#define CFG_TUD_AUDIO_ENABLE_EP_IN                1
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_MS      11  // using a bigger buffer size than the packet size is recommended to avoid packet loss
// #define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN    TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)
// #define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ      (CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN * CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_MS)
// #define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX         CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN // Maximum EP IN size for all AS alternate settings used
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN    512
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ      (CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN * CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_MS)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX         CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN // Maximum EP IN size for all AS alternate settings used


#ifdef __cplusplus
}
#endif