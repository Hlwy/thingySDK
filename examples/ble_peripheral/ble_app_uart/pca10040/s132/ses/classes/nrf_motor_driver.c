#include "nrf_motor_driver.h"
#include "nordic_common.h"
#include "app_error.h"
#include "app_timer.h"
#include "low_power_pwm.h"
#include <math.h>

static void pwm_handler(void * p_context){
    UNUSED_PARAMETER(p_context);
    low_power_pwm_t * pwm_instance = (low_power_pwm_t*)p_context;
}

void pwm_init(uint8_t pwm_gpio, uint8_t dir_gpio,motor_t* h_motor){
     uint32_t err_code;
     static low_power_pwm_t low_power_pwm;
     low_power_pwm_config_t low_power_pwm_config;

     APP_TIMER_DEF(lpp_timer);
     low_power_pwm_config.active_high    = true;
     low_power_pwm_config.period         = 255;
     low_power_pwm_config.bit_mask       = PIN_MASK(pwm_gpio);
     low_power_pwm_config.p_timer_id     = &lpp_timer;
     low_power_pwm_config.p_port         = NRF_GPIO;

     err_code = low_power_pwm_init((&h_motor->pwm), &low_power_pwm_config, pwm_handler);
     APP_ERROR_CHECK(err_code);
     err_code = low_power_pwm_duty_set(&h_motor->pwm, 20);
     APP_ERROR_CHECK(err_code);

     err_code = low_power_pwm_start((&h_motor->pwm), h_motor->pwm.bit_mask);
     APP_ERROR_CHECK(err_code);
    
     nrf_gpio_cfg_output(dir_gpio);
     h_motor->dir.pin = dir_gpio;
     h_motor->dir.level = 0;
}

void set_motor_direction(uint8_t direction, motor_t* h_motor){
     uint8_t level;

     if(direction == MOTOR_DIRECTION_FORWARD){
          level = direction;
     }else if(direction == MOTOR_DIRECTION_BACKWARD){
          level = direction;
     }else{
          level = 0;
     }
     
     h_motor->dir.level = level;
     nrf_gpio_pin_write(h_motor->dir.pin,level);
}

void set_speed(float spd_ratio, motor_t* h_motor){
     uint32_t err;
     uint8_t direction;

     if(spd_ratio > 1.0){
          spd_ratio = 1.0;
          direction = MOTOR_DIRECTION_FORWARD;
     }else if(spd_ratio < -1.0){
          spd_ratio = -1.0;
          direction = MOTOR_DIRECTION_BACKWARD;
     }else{
          spd_ratio = 0.0;
          direction = MOTOR_DIRECTION_STOP;
     }

     set_motor_direction(direction,h_motor);
     h_motor->speed = spd_ratio;
     
     // Calculate the PWM Duty value to writeOut
     uint8_t duty = fabs(spd_ratio) * 255;
     err = low_power_pwm_duty_set(&h_motor->pwm, duty);
}
