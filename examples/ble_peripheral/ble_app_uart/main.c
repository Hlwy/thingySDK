#pragma GCC diagnostic ignored "-Wint-conversion"

//NRF_LOG_USES_RTT=1
//NRF_LOG_ENABLED=1
//DEBUG

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "nordic_common.h"
#include "nrf.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "addon_defines.h"
#include "nrf_motor_driver.h"
#include "nrf_lights.h"
#include "nrf_buzzer.h"

#include "nrf_battery_monitor.h"

#include "nrf_ble_main.h"
//#include "vector_c.h"

//#define DEVICE_NAME                     "Nordic_UART"                               /**< Name of device. Will be included in the advertising data. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define SCHED_MAX_EVENT_DATA_SIZE   MAX(APP_TIMER_SCHED_EVENT_DATA_SIZE, BLE_STACK_HANDLER_SCHED_EVT_SIZE) /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE            60  /**< Maximum number of events in the scheduler queue. */

static const uint8_t pwm_pin = 2;
static const uint8_t dir_pin = 3;

static motor_t motor;
static battery_t battery;

static uint32_t level;
static uint32_t level2;
static uint32_t level3;
static uint32_t level4;
static uint32_t level5;

static char const m_target_periph_name[] = "BlueCharm";
static char const m_target_phone_name[] = "Samsung Galaxy S7";
static uint8_t target_mac[] = {0xb0,0x91,0x22,0xf7,0x6d,0x55};
static uint8_t const target_mac_rvr[] = {0x55,0x6d,0xf7,0x22,0x91,0xb0};
static ble_bas_t m_bas;                                   /**< Structure used to identify the battery service. */

void print_current_time()
{
//    printf("Uncalibrated time:\t%s\r\n", nrf_cal_get_time_string(false));
//    printf("Calibrated time:\t%s (%d, %d, %d)\r\n", nrf_cal_get_time_string(true),MSEC_TO_UNITS(1000,UNIT_0_625_MS),MSEC_TO_UNITS(1000,UNIT_1_25_MS),MSEC_TO_UNITS(1000,UNIT_10_MS));
}

#define FPU_EXCEPTION_MASK 0x0000009F
static void power_manage(void)
{
     __set_FPSCR(__get_FPSCR()  & ~(FPU_EXCEPTION_MASK));
    (void) __get_FPSCR();
    NVIC_ClearPendingIRQ(FPU_IRQn);
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
static void battery_level_update(void)
{
    ret_code_t err_code;
    uint8_t  battery_level;

//    battery_level = (uint8_t)sensorsim_measure(&m_battery_sim_state, &m_battery_sim_cfg);

    err_code = ble_bas_battery_level_update(&m_bas, battery_level);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    battery_level_update();
}


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
     drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_LIGHTWELL,&color_red);
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
     uint32_t err_code;
//    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
//    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
//    err_code = bsp_btn_ble_sleep_mode_prepare();
//    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}

static uint8_t target_mac_rev[] = {0x55,0x6d,0xf7,0x22,0x91,0xb0};
static uint8_t target_mac2[] = {0xf2,0x9e,0x74,0x92,0xfb,0xe5};
static char* name2 = "add_dev tkr C3:CE:5E:26:AD:0A";
int tmpDuty;
uint8_t new_duty_cycle = 255;
uint32_t pwroff_t0;
/**@brief Application main function.*/
int main(void)
{
    uint32_t err_code;
    bool     erase_bonds;
    nTimeUpdates = 0;
    closeCounter = 0;
    vec_deinit(&addrs); vec_deinit(&names);
    vec_init(&addrs); vec_init(&names);

//    vec_byte_t testAddr;
//    vec_byte_t testAddr2;
//    vec_bytes_t testAddrs;
//    vec_init(&testAddr); vec_init(&testAddr2); vec_init(&testAddrs);
//    vec_push(&testAddr,0xb0); vec_push(&testAddr,0x91); vec_push(&testAddr,0x22);
//    vec_push(&testAddr,0xf7); vec_push(&testAddr,0x6d); vec_push(&testAddr,0x55);
//    vec_push(&testAddrs,testAddr);
//
//    vec_push(&testAddr2,0xf2); vec_push(&testAddr2,0x9e); vec_push(&testAddr2,0x74);
//    vec_push(&testAddr2,0x92); vec_push(&testAddr2,0xfb); vec_push(&testAddr2,0xe5);
//    vec_push(&testAddrs,testAddr2);
//    bool isMatch;
//    int fndIdx;
//    isMatch = check_addr_vecs(&testAddrs,target_mac,&fndIdx);
//    if (isMatch){printf("check_addr_vec: Addresses Match...\n");
//    }else{printf("check_addr_vec: Addresses Don't Match...\n");}
//
//    printf("\tvec_find returned = %d\n",fndIdx);
//    vec_splice(&testAddrs, fndIdx, 1);

    // Initialize.
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    
//    err_code = app_timer_create(&m_battery_timer_id,APP_TIMER_MODE_REPEATED,battery_level_meas_timeout_handler);
    
    uart_init();
    log_init();

    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();

    NRF_LOG_INFO("UART Start!\r\n");
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    scan_start();

    board_init();

    pwm_init(pwm_pin,dir_pin,&motor);
    buzzer_init(speaker_handler);
    battery_monitor_init(300000, battery_monitor_handler, &battery);
    new_duty_cycle = motor.pwm.period;

//     drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_green);

//    NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"===== Thingy demo started! =====  \r\n"NRF_LOG_COLOR_CODE_DEFAULT);

    nrf_gpio_cfg_input(BUTTON, NRF_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(SX_IOEXT_0,DRV_EXT_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(SX_IOEXT_1,DRV_EXT_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(SX_IOEXT_2,DRV_EXT_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(SX_IOEXT_3,DRV_EXT_GPIO_PIN_PULLUP);

    uint32_t tmpLevel;
    // Enter main loop.

    struct tm* tmpT2 = nrf_cal_get_time_calibrated();
    begin = mktime(tmpT2);
//    printf("Start Time = %d\n",begin);
//     led_set(&led_search, NULL);
    
    nrf_delay_ms(1000);
    myTimeStamp = millis();
    pwroff_t0 = -1;
    for (;;)
    {
        app_sched_execute();

        level = !nrf_gpio_pin_read(11);
        tmpLevel = drv_ext_gpio_pin_read(SX_IOEXT_0,&level2);
        tmpLevel = drv_ext_gpio_pin_read(SX_IOEXT_1,&level3);
        tmpLevel = drv_ext_gpio_pin_read(SX_IOEXT_2,&level4);
        tmpLevel = drv_ext_gpio_pin_read(SX_IOEXT_3,&level5);

        if((level == 1) && (level3 == 1)){
         tmpDuty = new_duty_cycle - 1;
         drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_LIGHTWELL,&color_purple);
          if(pwroff_t0 < 0){ pwroff_t0 = millis();}
          if(compareMillis(pwroff_t0, millis()) > 5000)
          {
            drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_LIGHTWELL,&color_white);
            nrf_delay_ms(1000);
            sd_nvic_SystemReset();
          }
       }else if((level == 0) && (level3 == 0)){
         tmpDuty = new_duty_cycle + 1;
         drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_LIGHTWELL,&color_darkpurple);
       }else if((level == 1) && (level3 == 0)){
          drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_LIGHTWELL,&color_aqua);
          initiate_alarm_sequence(2500,50,5,500,1000);
       }else{
          // Nothing
          drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_LIGHTWELL,&color_black);
       }
//          if((tags_detected) && (!tags_nearby)){
//               drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_blue);
//          }else if(tags_nearby){
//               drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_green);
//          }else{
////               drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_black);
//          }
          if(flag_close_door){
               drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_red);
          }else{
               drv_ext_light_rgb_intensity_set(DRV_EXT_RGB_LED_SENSE,&color_green);
          }

         if(tmpDuty >= 255){
         new_duty_cycle = 255;
       }else if(tmpDuty<=0){
         new_duty_cycle = 0;
       }else{
         new_duty_cycle = tmpDuty;
       }

       err_code = low_power_pwm_duty_set(&motor.pwm, new_duty_cycle);
        power_manage();
//        nrf_delay_ms(1000);
    }
}
