#ifndef NRF_MOTOR_DRIVER_H__
#define NRF_MOTOR_DRIVER_H__

#include <stdint.h>
#include "pca20020.h"
#include "low_power_pwm.h"

#define PIN_MASK(_pin) (1u << (uint32_t)((_pin) & (~P0_PIN_NUM)))

static const uint8_t MOTOR_DIRECTION_FORWARD = 0;
static const uint8_t MOTOR_DIRECTION_BACKWARD = 1;
static const uint8_t MOTOR_DIRECTION_STOP = 2;

typedef struct{
     uint8_t pin;
     uint8_t level;
} dir_pin_t;

typedef struct{
     low_power_pwm_t pwm;
     dir_pin_t dir;
     float speed;
} motor_t;


void pwm_init(uint8_t pwm_gpio, uint8_t dir_gpio, motor_t* h_motor);
void set_motor_direction(uint8_t direction, motor_t* h_motor);
void set_speed(float spd_ratio, motor_t* h_motor);

#endif // NRF_MOTOR_DRIVER_H__