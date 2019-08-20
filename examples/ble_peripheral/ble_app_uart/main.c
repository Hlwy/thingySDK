#pragma GCC diagnostic ignored "-Wint-conversion"

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "nrf.h"
#include "nordic_common.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "addon_defines.h"
#include "nrf_motor_driver.h"
#include "nrf_lights.h"
#include "nrf_buzzer.h"
#include "nrf_battery_monitor.h"
#include "nrf_ble_main.h"
#include "dd_cmd_relay.h"

//#define DEVICE_NAME                     "Nordic_UART"                               /**< Name of device. Will be included in the advertising data. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define SCHED_MAX_EVENT_DATA_SIZE   MAX(APP_TIMER_SCHED_EVENT_DATA_SIZE, BLE_STACK_HANDLER_SCHED_EVT_SIZE) /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE            60  /**< Maximum number of events in the scheduler queue. */

static const uint8_t pwm_pin            = 2;
static const uint8_t dir_pin            = 3;
static const uint8_t up_limit_switch    = SX_IOEXT_0;
static const uint8_t low_limit_switch   = SX_IOEXT_1;
static const uint8_t aux_switch         = BUTTON;

static motor_t motor;
static battery_t battery;

static uint32_t lower_limit_reached;
static uint32_t upper_limit_reached;

uint32_t pwroff_t0 = -1;
static ble_bas_t m_bas;                                   /**< Structure used to identify the battery service. */
static bool toggler = false;

static int nResetToggles = 0;
static time_t reset_begin;
static struct tm* tmpResetT;
static time_t reset_now;

/** *********************************   ********************************************* */
static void reset_switch_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
     if((nResetToggles == 0) || (nResetToggles>2)){
          reset_begin = mktime(nrf_cal_get_time_calibrated());
          nResetToggles = 0;
     }
     nResetToggles += 1;
     tmpResetT = nrf_cal_get_time_calibrated();
     reset_now = mktime(tmpResetT);
     float reset_dt = (reset_now - reset_begin)/3925.0;
     if(reset_dt >= 5.0){
          drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_LIGHTWELL,&color_white);
          nrf_delay_ms(1000);
          sd_nvic_SystemReset();
     }     
//     uint32_t now = millis();
//     uint32_t tmpdt = compareMillis(myTimeStamp, now);
//     float dt = tmpdt/(float)1000.0;
}

static void gpio_init(void){
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    
    // Upper Limit Switch
    nrf_drv_gpiote_in_config_t reset_cfg = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    reset_cfg.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(aux_switch, &reset_cfg, reset_switch_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(aux_switch, true);
}


#define FPU_EXCEPTION_MASK 0x0000009F
static void power_manage(void){
     __set_FPSCR(__get_FPSCR()  & ~(FPU_EXCEPTION_MASK));
    (void) __get_FPSCR();
    NVIC_ClearPendingIRQ(FPU_IRQn);
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name){
    drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_LIGHTWELL,&color_red);
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for putting the chip into sleep mode. */
static void sleep_mode_enter(void){
     uint32_t err_code;

    // Prepare wakeup buttons.
//    err_code = bsp_btn_ble_sleep_mode_prepare();
//    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the nrf log module.*/
static void log_init(void){
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}

void close_door(){
     uint32_t tmpLevel;
     static bool last_lower_switch_value;
     static bool last_upper_switch_value;
     tmpLevel = drv_ext_gpio_pin_read(low_limit_switch,&lower_limit_reached);
     if(last_lower_switch_value != lower_limit_reached){
          report_limit_switch_state(&m_nus, "lower",!lower_limit_reached);
          last_lower_switch_value = lower_limit_reached;
     }
     tmpLevel = drv_ext_gpio_pin_read(up_limit_switch,&upper_limit_reached);
     if(last_upper_switch_value != upper_limit_reached){
          report_limit_switch_state(&m_nus, "upper",!upper_limit_reached);
          last_upper_switch_value = upper_limit_reached;
     }
     
     if(!lower_limit_reached){
          // drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_black);
     }else{
          // drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_red);
          // printf("[DoggyDoor] Closing door...\r\n");
          float doorVel = motor_speed * 1.0;
          set_speed(doorVel,&motor);
          while(lower_limit_reached){
               if(flag_force_door_open || flag_force_door_stop) break;
               tmpLevel = drv_ext_gpio_pin_read(low_limit_switch,&lower_limit_reached);
               app_sched_execute();
          }
          set_speed(0.0,&motor);
          // printf("[DoggyDoor] Finished close door sequence.\r\n");
     }     
}

void open_door(){
     uint32_t tmpLevel;
     static bool last_lower_switch_value;
     static bool last_upper_switch_value;
     tmpLevel = drv_ext_gpio_pin_read(up_limit_switch,&upper_limit_reached);
     if(last_upper_switch_value != upper_limit_reached){
          report_limit_switch_state(&m_nus, "upper",!upper_limit_reached);
          last_upper_switch_value = upper_limit_reached;
     }
     tmpLevel = drv_ext_gpio_pin_read(low_limit_switch,&lower_limit_reached);
     if(last_lower_switch_value != lower_limit_reached){
          report_limit_switch_state(&m_nus, "lower",!lower_limit_reached);
          last_lower_switch_value = lower_limit_reached;
     }
     if(!upper_limit_reached){
          // drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_black);
     }else{
          // drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_green);
          // printf("[DoggyDoor] Opening door...\r\n");
          float doorVel = motor_speed * -1.0;
          set_speed(doorVel,&motor);
          while(upper_limit_reached){
               tmpLevel = drv_ext_gpio_pin_read(up_limit_switch,&upper_limit_reached);
               if(flag_force_door_close || flag_force_door_stop) break;
               app_sched_execute();
          }
          set_speed(0.0,&motor);
          // printf("[DoggyDoor] Finished open door sequence.\r\n");
     }
}

/**@brief Application main function.*/
int main(void){
    uint32_t err_code;
    nBleUpdates = 0;
    debounceCounter = 0;
    vec_deinit(&m_tags); vec_init(&m_tags);

    // Initialize.
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
     
    struct tm* tmpT2 = nrf_cal_get_time_calibrated();
    begin = mktime(tmpT2);

    uart_init();
    log_init();
    init_ble();
    gpio_init();
    board_init();
    pwm_init(pwm_pin,dir_pin,&motor);
    buzzer_init(speaker_handler);
    battery_monitor_init(900000, battery_monitor_handler, &battery);
    NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"===== Thingy demo started! =====  \r\n"NRF_LOG_COLOR_CODE_DEFAULT);

    drv_ext_gpio_cfg_input(low_limit_switch,DRV_EXT_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(up_limit_switch,DRV_EXT_GPIO_PIN_PULLUP);

    // myTimeStamp = millis();
    set_speed(0.0,&motor);
    for(;;){
          app_sched_execute();
          if(tags_nearby){drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_green);}
          else{drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_black);}

          if(flag_force_door_open){
               // printf("[DoggyDoor] Forcing door open.\r\n");
               open_door();
          }else if(flag_force_door_close){
               // printf("[DoggyDoor] Forcing door closed.\r\n");
               close_door();
          }else if(flag_force_door_stop){
               // printf("[DoggyDoor] Forced stopping.\r\n");
               set_speed(0.0,&motor);
          }else{
               // printf("[DoggyDoor] Normal operation.\r\n");
               if(!tags_nearby) close_door();
               else open_door();
          }
        power_manage();
    }
}
