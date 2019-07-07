#include "nrf_battery_monitor.h"
#include "nrf_delay.h"
#include "app_error.h"

void battery_monitor_init(uint32_t period_ms, void (*handler)(m_batt_meas_event_t const *), battery_t* h_battery){
    ret_code_t err_code;
    static m_ble_service_handle_t ble_bat_handle;
    batt_meas_init_t battery_cfg = {
         .evt_handler = handler,
         .batt_meas_param = BATT_MEAS_PARAM_CFG,
    };

    err_code = m_batt_meas_init(&ble_bat_handle, &battery_cfg);
    APP_ERROR_CHECK(err_code);

    err_code = m_batt_meas_enable(period_ms); // 60000
    APP_ERROR_CHECK(err_code);
}
