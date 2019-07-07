#ifndef NRF_BUZZER_H__
#define NRF_BUZZER_H__

#include <stdint.h>
#include "drv_speaker.h"
#include "app_error.h"
#include "nrf_log.h"

void buzzer_init(void (*handler)(drv_speaker_evt_t));
void initiate_alarm_sequence(uint16_t freq, uint8_t volume, uint8_t nBeeps, uint32_t delay, uint32_t dt_ms);

static void speaker_handler(drv_speaker_evt_t _evt){
     switch(_evt){
          case DRV_SPEAKER_EVT_FINISHED:{
               NRF_LOG_DEBUG("----- Speaker Finished!\r\n");
          }break; // DRV_SPEAKER_EVT_FINISHED

          case DRV_SPEAKER_EVT_BUFFER_WARNING:{
               NRF_LOG_DEBUG("----- Speaker Buffer Warning!\r\n");
          }break; // DRV_SPEAKER_EVT_BUFFER_WARNING

          case DRV_SPEAKER_EVT_BUFFER_READY:{
               NRF_LOG_DEBUG("----- Speaker Buffer Ready!\r\n");
          }break; // DRV_SPEAKER_EVT_BUFFER_READY

          default:{
               NRF_LOG_DEBUG("----- Speaker Default Case!\r\n");
          }break;
     }
}

#endif // NRF_BUZZER_H__
