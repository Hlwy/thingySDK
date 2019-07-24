#ifndef NRF_BATTERY_MONITOR_H__
#define NRF_BATTERY_MONITOR_H__

#include <stdint.h>
#include "m_batt_meas.h"
#include "app_error.h"
#include "pca20020.h"
#include "nrf_log.h"
#include "ble_bas.h"

static uint8_t bat_percent;
static uint16_t voltage;

typedef struct{
     uint16_t voltage;
     uint8_t remaining_percent;
     m_ble_service_handle_t handle;
} battery_t;

void battery_monitor_init(uint32_t period_ms, void (*handler)(m_batt_meas_event_t const *), battery_t* h_battery);

static void battery_monitor_handler(m_batt_meas_event_t const * p_event){
     m_batt_meas_event_type_t evt_type = p_event->type;

     switch(evt_type){
          case M_BATT_MEAS_EVENT_DATA:{
               bat_percent = p_event->level_percent;
               voltage = p_event->voltage_mv;
               NRF_LOG_INFO("----- Battery Monitor --- Event Data:\r\n\tVoltage (mV) = %d\r\n\tBattery Life (%%) = %d\r\n",voltage,bat_percent);
          }break; // M_BATT_MEAS_EVENT_DATA

          case M_BATT_MEAS_EVENT_LOW:{
               NRF_LOG_INFO("----- Battery Monitor: BATTERY LOW!\r\n");
          }break; // M_BATT_MEAS_EVENT_LOW

          case M_BATT_MEAS_EVENT_FULL:{
               NRF_LOG_INFO("----- Battery Monitor: Battery Full!\r\n");
          }break; // M_BATT_MEAS_EVENT_FULL

          case M_BATT_MEAS_EVENT_USB_CONN_CHARGING:{
               NRF_LOG_INFO("----- Battery Monitor: Charging...\r\n");
          }break; // M_BATT_MEAS_EVENT_USB_CONN_CHARGING

          case M_BATT_MEAS_EVENT_USB_CONN_CHARGING_FINISHED:{
               NRF_LOG_INFO("----- Battery Monitor: Charging Finished!\r\n");
          }break; // M_BATT_MEAS_EVENT_USB_CONN_CHARGING_FINISHED

          case M_BATT_MEAS_EVENT_USB_DISCONN:{
               NRF_LOG_INFO("----- Battery Monitor: Charging Stopped (USB Disconnected).\r\n");
          }break; // M_BATT_MEAS_EVENT_USB_DISCONN

          case M_BATT_MEAS_EVENT_ERROR:{
               NRF_LOG_INFO("----- Battery Monitor: Error occurred!\r\n");
          }break; // M_BATT_MEAS_EVENT_ERROR

          default:{
               NRF_LOG_INFO("----- Battery Monitor: Default Case!\r\n");
          }break;
     }
}

#endif // NRF_BATTERY_MONITOR_H__
