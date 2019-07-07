#include "nrf_buzzer.h"
#include "nrf_delay.h"

void initiate_alarm_sequence(uint16_t freq, uint8_t volume, uint8_t nBeeps, uint32_t delay, uint32_t dt_ms){
     uint32_t err_code;
     for(int i = 0; i < nBeeps; i++){
          err_code = drv_speaker_tone_start(freq, dt_ms, volume);
          APP_ERROR_CHECK(err_code);
          nrf_delay_ms(delay);
          err_code = drv_speaker_tone_start(100, dt_ms, 0);
          APP_ERROR_CHECK(err_code);
          nrf_delay_ms(delay);
     }
}

// void buzzer_init(void* handler, drv_speaker_init_t* cfg){
void buzzer_init(void (*handler)(drv_speaker_evt_t)){
     drv_speaker_init_t buzzer_cfg = {.evt_handler = handler,};
     ret_code_t err_code = drv_speaker_init(&buzzer_cfg);
     APP_ERROR_CHECK(err_code);
}
