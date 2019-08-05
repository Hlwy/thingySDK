#ifndef NRF_BATTERY_MONITOR_H__
#define NRF_BATTERY_MONITOR_H__

#include <stdint.h>
#include "m_batt_meas.h"
#include "app_error.h"
#include "pca20020.h"
#include "nrf_log.h"
#include "ble_bas.h"
#include "nrf_buzzer.h"

//#define DEBUG_BATTERY

typedef enum{
     DD_BAT_NOT_CHARGING,
     DD_BAT_CHARGING,
     DD_BAT_CHARGING_FINISHED,
     DD_BAT_CHARGING_DISCONNECTED,
     DD_BAT_LOW,
     DD_BAT_FULL,
     DD_BAT_ERROR,
} bat_charge_status_t;

typedef struct{
     uint16_t voltage;
     uint8_t remaining_percent;
     bat_charge_status_t status;
     m_ble_service_handle_t handle;
} battery_t;

static uint8_t bat_percent;
static uint16_t voltage;
static m_batt_meas_event_type_t m_bat_status;
static bat_charge_status_t dd_bat_status;
static bool flag_low_battery = false;

void battery_monitor_init(uint32_t period_ms, void (*handler)(m_batt_meas_event_t const *), battery_t* h_battery);

static void battery_monitor_handler(m_batt_meas_event_t const * p_event){
     m_batt_meas_event_type_t evt_type = p_event->type;
     m_bat_status = evt_type;

     switch(evt_type){
          case M_BATT_MEAS_EVENT_DATA:{
               bat_percent = p_event->level_percent;
               voltage = p_event->voltage_mv;
               dd_bat_status = DD_BAT_NOT_CHARGING;
#ifdef DEBUG_BATTERY              
               printf("Battery Monitor (%d) --- Voltage (mV) = %d (%d%%)\r\n",M_BATT_MEAS_EVENT_DATA,voltage,bat_percent);
#endif          
          }break; // M_BATT_MEAS_EVENT_DATA

          case M_BATT_MEAS_EVENT_LOW:{
               // printf("Battery Monitor (%d): BATTERY LOW!\r\n",M_BATT_MEAS_EVENT_LOW);
               initiate_alarm_sequence(2500,100,5,500,1000);
               flag_low_battery = true;
               dd_bat_status = DD_BAT_LOW;
          }break; // M_BATT_MEAS_EVENT_LOW

          case M_BATT_MEAS_EVENT_FULL:{
               // printf("----- Battery Monitor (%d): Battery Full!\r\n",M_BATT_MEAS_EVENT_FULL);
               dd_bat_status = DD_BAT_FULL;
          }break; // M_BATT_MEAS_EVENT_FULL

          case M_BATT_MEAS_EVENT_USB_CONN_CHARGING:{
               // printf("----- Battery Monitor (%d): Charging...\r\n",M_BATT_MEAS_EVENT_USB_CONN_CHARGING);
               dd_bat_status = DD_BAT_CHARGING;
          }break; // M_BATT_MEAS_EVENT_USB_CONN_CHARGING

          case M_BATT_MEAS_EVENT_USB_CONN_CHARGING_FINISHED:{
               // printf("----- Battery Monitor (%d): Charging Finished!\r\n",M_BATT_MEAS_EVENT_USB_CONN_CHARGING_FINISHED);
               dd_bat_status = DD_BAT_CHARGING_FINISHED;
          }break; // M_BATT_MEAS_EVENT_USB_CONN_CHARGING_FINISHED

          case M_BATT_MEAS_EVENT_USB_DISCONN:{
               // printf("----- Battery Monitor (%d): Charging Stopped (USB Disconnected).\r\n",M_BATT_MEAS_EVENT_USB_DISCONN);
               dd_bat_status = DD_BAT_CHARGING_DISCONNECTED;
          }break; // M_BATT_MEAS_EVENT_USB_DISCONN

          case M_BATT_MEAS_EVENT_ERROR:{
               // printf("----- Battery Monitor (%d): Error occurred!\r\n",M_BATT_MEAS_EVENT_ERROR);
               dd_bat_status = DD_BAT_ERROR;
          }break; // M_BATT_MEAS_EVENT_ERROR

          default:{
               // NRF_LOG_INFO("----- Battery Monitor: Default Case!\r\n");
               dd_bat_status = DD_BAT_NOT_CHARGING;
          }break;
     }
}

#endif // NRF_BATTERY_MONITOR_H__
