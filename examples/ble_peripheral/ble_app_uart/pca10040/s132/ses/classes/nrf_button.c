#include "nrf_button.h"
#include "nrf_delay.h"

//static void button_evt_handler(uint8_t pin_no, uint8_t button_action)
//{
//    uint32_t err_code;
//    static uint8_t led_state = 0;
//    if(button_action == 1)
//    {
//        led_state ^= 1;
//    }
////    ble_uis_led_t led_on = {.mode = BLE_UIS_LED_MODE_CONST,
////                            .data.mode_const.r = 0xff,
////                            .data.mode_const.g = 0xff,
////                            .data.mode_const.b = 0xff};
////    ble_uis_led_t led_off = {.mode = BLE_UIS_LED_MODE_CONST,
////                            .data.mode_const.r = 0x00,
////                            .data.mode_const.g = 0x00,
////                            .data.mode_const.b = 0x00};
//}
//static ret_code_t button_init(void)
//{
//    ret_code_t err_code;
//
//    /* Configure gpiote for the sensors data ready interrupt. */
//    if (!nrf_drv_gpiote_is_init())
//    {
//        err_code = nrf_drv_gpiote_init();
//        RETURN_IF_ERROR(err_code);
//    }
//
//    static const app_button_cfg_t button_cfg =
//    {
//        .pin_no         = BUTTON,
//        .active_state   = APP_BUTTON_ACTIVE_LOW,
//        .pull_cfg       = NRF_GPIO_PIN_PULLUP,
//        .button_handler = button_evt_handler
//    };
//
//    err_code = app_button_init(&button_cfg, 1, APP_TIMER_TICKS(50));
//    RETURN_IF_ERROR(err_code);
//
//    return app_button_enable();
//}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
//static void buttons_leds_init(bool * p_erase_bonds)
//{
//    bsp_event_t startup_event;
//
//    uint32_t err_code = bsp_btn_ble_init(NULL, &startup_event);
//    APP_ERROR_CHECK(err_code);
//
//    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
//}
