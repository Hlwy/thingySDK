#include "nrf_lights.h"
#include "app_error.h"
#include "app_timer.h"
#include "support_func.h"
#include "twi_manager.h"


void board_init(void){
    uint32_t            err_code;
    m_ui_init_t         ui_params;
    drv_ext_gpio_init_t ext_gpio_init;
    drv_ext_light_init_t ext_light_init;

    #if defined(THINGY_HW_v0_7_0)
        #error   "HW version v0.7.0 not supported."
    #elif defined(THINGY_HW_v0_8_0)
        NRF_LOG_WARNING("FW compiled for depricated Thingy HW v0.8.0 \r\n");
    #elif defined(THINGY_HW_v0_9_0)
        NRF_LOG_WARNING("FW compiled for depricated Thingy HW v0.9.0 \r\n");
    #endif

    static const nrf_drv_twi_config_t twi_config =
    {
        .scl                = TWI_SCL,
        .sda                = TWI_SDA,
        .frequency          = NRF_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_LOW
    };

    static const drv_sx1509_cfg_t sx1509_cfg =
    {
        .twi_addr       = SX1509_ADDR,
        .p_twi_instance = &m_twi_sensors,
        .p_twi_cfg      = &twi_config
    };

    ext_gpio_init.p_cfg = &sx1509_cfg;
    err_code = support_func_configure_io_startup(&ext_gpio_init);
    APP_ERROR_CHECK(err_code);

    ext_light_init.p_twi_conf = &sx1509_cfg;
    ext_light_init.num_lights = 2;
    ext_light_init.resync_pin = DRV_EXT_LIGHT_INVALID_RESYNC_PIN;
    ext_light_init.clkx_div = DRV_EXT_LIGHT_CLKX_DIV_8;
    ext_light_init.p_light_conf = led_cfg;

    err_code = drv_ext_light_init(&ext_light_init,true);
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(100);

     /**@brief Initialize the TWI manager. */
//    err_code = twi_manager_init(APP_IRQ_PRIORITY_THREAD);
//    APP_ERROR_CHECK(err_code);
//
//    /**@brief Initialize LED and button UI module. */
//    ui_params.p_twi_instance = &m_twi_sensors;

    /** Begin Original*/
//    err_code = m_ui_init(&m_ble_service_handles[THINGY_SERVICE_UI],&ui_params);
//    APP_ERROR_CHECK(err_code);
//   led_set(&led_search, NULL);
}
