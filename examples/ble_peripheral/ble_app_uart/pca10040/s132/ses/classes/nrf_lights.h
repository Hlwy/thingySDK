#ifndef NRF_LIGHTS_H__
#define NRF_LIGHTS_H__

#include "sdk_config.h"
#include "pca20020.h"
#include "drv_ext_light.h"
#include "ble_uis.h"
//#include "m_ui_demo.h"
#include "m_ui.h"
#include "nrf_delay.h"

static const nrf_drv_twi_t     m_twi_sensors = NRF_DRV_TWI_INSTANCE(TWI_SENSOR_INSTANCE);
static m_ble_service_handle_t  m_ble_service_handles[THINGY_SERVICES_MAX];

//static const drv_ext_light_conf_t led_cfg = {
//    .type = DRV_EXT_LIGHT_TYPE_RGB,
//    .pin.rgb = {
//        .r = SX_LIGHTWELL_R,
//        .g = SX_LIGHTWELL_G,
//        .b = SX_LIGHTWELL_B },
//    .p_data = &my_led_1
//};

//static const drv_ext_light_conf_t led_cfg2 = {
//    .type = DRV_EXT_LIGHT_TYPE_RGB,
//    .pin.rgb = {
//        .r = SX_SENSE_LED_R,
//        .g = SX_SENSE_LED_G,
//        .b = SX_SENSE_LED_B },
//    .p_data = &my_led_0
//};

static const drv_ext_light_conf_t led_cfg[2] = DRV_EXT_LIGHT_CFG;

static const drv_ext_light_rgb_intensity_t color_white = {.r = 255, .g = 255, .b = 255};
static const drv_ext_light_rgb_intensity_t color_black = {.r = 0,.g = 0,.b = 0};

static const drv_ext_light_rgb_intensity_t color_red = {.r = 255,.g = 0,.b = 0};
static const drv_ext_light_rgb_intensity_t color_blue = {.r = 0,.g = 0,.b = 255};
static const drv_ext_light_rgb_intensity_t color_green = { .r = 0, .g = 255, .b = 0};

static const drv_ext_light_rgb_intensity_t color_purple = {.r = 255, .g = 0, .b = 255};
static const drv_ext_light_rgb_intensity_t color_yellow = {.r = 255, .g = 255, .b = 0};
static const drv_ext_light_rgb_intensity_t color_orange = {.r = 255, .g = 128, .b = 0};
static const drv_ext_light_rgb_intensity_t color_aqua = {.r = 0, .g = 255, .b = 255};
static const drv_ext_light_rgb_intensity_t color_darkpurple = {.r = 128, .g = 0, .b = 128};



static const ble_uis_led_t led_on = {
  .mode = BLE_UIS_LED_MODE_CONST,
  .data.mode_const.r = 0xff,
  .data.mode_const.g = 0xff,
  .data.mode_const.b = 0xff
};

static const ble_uis_led_t led_off = {
  .mode = BLE_UIS_LED_MODE_CONST,
  .data.mode_const.r = 0x00,
  .data.mode_const.g = 0x00,
  .data.mode_const.b = 0x00
};

static const ble_uis_led_t led_search = {
  .mode = BLE_UIS_LED_MODE_BREATHE,
  .data.mode_breathe.color_mix = DRV_EXT_LIGHT_COLOR_YELLOW,
  .data.mode_breathe.intensity  = DEFAULT_LED_INTENSITY_PERCENT,
  .data.mode_breathe.delay = DEFAULT_LED_OFF_TIME_MS
};

static const ble_uis_led_t led_found = {
  .mode = BLE_UIS_LED_MODE_CONST,
  .data.mode_const.r = 0x00,
  .data.mode_const.g = 0xff,
  .data.mode_const.b = 0x00
};

static const ble_uis_led_t led_breath_red = {
  .mode = BLE_UIS_LED_MODE_BREATHE,
  .data.mode_breathe.color_mix = DRV_EXT_LIGHT_COLOR_RED,
  .data.mode_breathe.intensity  = DEFAULT_LED_INTENSITY_PERCENT,
  .data.mode_breathe.delay = DEFAULT_LED_OFF_TIME_MS
};

static const ble_uis_led_t led_breath_blue = {
  .mode = BLE_UIS_LED_MODE_BREATHE,
  .data.mode_breathe.color_mix = DRV_EXT_LIGHT_COLOR_BLUE,
  .data.mode_breathe.intensity  = DEFAULT_LED_INTENSITY_PERCENT,
  .data.mode_breathe.delay = DEFAULT_LED_OFF_TIME_MS
};


void board_init(void);


// void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
// {
//     #if NRF_LOG_ENABLED
//         error_info_t * err_info = (error_info_t*)info;
//         NRF_LOG_ERROR(" id = %d, pc = %d, file = %s, line number: %d, error code = %d = %s \r\n", \
//         id, pc, nrf_log_push((char*)err_info->p_file_name), err_info->line_num, err_info->err_code, nrf_log_push((char*)nrf_strerror_find(err_info->err_code)));
//     #endif
//
//     NRF_LOG_FINAL_FLUSH();
//     nrf_delay_ms(5);
//
//     // On assert, the system can only recover with a reset.
//     #ifndef DEBUG
//         NVIC_SystemReset();
//     #endif
//
//     app_error_save_and_stop(id, pc, info);
// }


#endif // NRF_LIGHTS_H__
