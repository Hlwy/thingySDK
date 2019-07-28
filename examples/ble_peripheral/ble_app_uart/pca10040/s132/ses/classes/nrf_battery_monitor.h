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
static m_batt_meas_event_type_t m_bat_status;

typedef struct{
     uint16_t voltage;
     uint8_t remaining_percent;
     m_ble_service_handle_t handle;
} battery_t;

void battery_monitor_init(uint32_t period_ms, void (*handler)(m_batt_meas_event_t const *), battery_t* h_battery);

static void battery_monitor_handler(m_batt_meas_event_t const * p_event){
     m_batt_meas_event_type_t evt_type = p_event->type;
     m_bat_status = evt_type;

     switch(evt_type){
          case M_BATT_MEAS_EVENT_DATA:{
               bat_percent = p_event->level_percent;
               voltage = p_event->voltage_mv;
               printf("Battery Monitor (%d) --- Voltage (mV) = %d (%d%%)\r\n",M_BATT_MEAS_EVENT_DATA,voltage,bat_percent);
          }break; // M_BATT_MEAS_EVENT_DATA

          case M_BATT_MEAS_EVENT_LOW:{
               printf("Battery Monitor (%d): BATTERY LOW!\r\n",M_BATT_MEAS_EVENT_LOW);
          }break; // M_BATT_MEAS_EVENT_LOW

          case M_BATT_MEAS_EVENT_FULL:{
               printf("----- Battery Monitor (%d): Battery Full!\r\n",M_BATT_MEAS_EVENT_FULL);
          }break; // M_BATT_MEAS_EVENT_FULL

          case M_BATT_MEAS_EVENT_USB_CONN_CHARGING:{
               printf("----- Battery Monitor (%d): Charging...\r\n",M_BATT_MEAS_EVENT_USB_CONN_CHARGING);
          }break; // M_BATT_MEAS_EVENT_USB_CONN_CHARGING

          case M_BATT_MEAS_EVENT_USB_CONN_CHARGING_FINISHED:{
               printf("----- Battery Monitor (%d): Charging Finished!\r\n",M_BATT_MEAS_EVENT_USB_CONN_CHARGING_FINISHED);
          }break; // M_BATT_MEAS_EVENT_USB_CONN_CHARGING_FINISHED

          case M_BATT_MEAS_EVENT_USB_DISCONN:{
               printf("----- Battery Monitor (%d): Charging Stopped (USB Disconnected).\r\n",M_BATT_MEAS_EVENT_USB_DISCONN);
          }break; // M_BATT_MEAS_EVENT_USB_DISCONN

          case M_BATT_MEAS_EVENT_ERROR:{
               printf("----- Battery Monitor (%d): Error occurred!\r\n",M_BATT_MEAS_EVENT_ERROR);
          }break; // M_BATT_MEAS_EVENT_ERROR

          default:{
               NRF_LOG_INFO("----- Battery Monitor: Default Case!\r\n");
          }break;
     }
}

#endif // NRF_BATTERY_MONITOR_H__
