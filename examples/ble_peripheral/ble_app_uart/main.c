/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "nrf_ble_gatt.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "addon_defines.h"
#include "nrf_motor_driver.h"
#include "nrf_lights.h"
#include "nrf_buzzer.h"
//#include "nrf_ble_main.h"
#include "nrf_battery_monitor.h"
#include "vector_c.h"

#define CONN_CFG_TAG                    1                                           /**< A tag that refers to the BLE stack configuration we set with @ref sd_ble_cfg_set. Default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

#define DEVICE_NAME                     "Nordic_UART"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout (in units of seconds). */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
#define SCHED_MAX_EVENT_DATA_SIZE   MAX(APP_TIMER_SCHED_EVENT_DATA_SIZE, BLE_STACK_HANDLER_SCHED_EVT_SIZE) /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE            60  /**< Maximum number of events in the scheduler queue. */
#define NRF_BLE_LINK_COUNT              (NRF_BLE_PERIPHERAL_LINK_COUNT + NRF_BLE_CENTRAL_LINK_COUNT)

typedef struct
{
    uint8_t  * p_data;    /**< Pointer to data. */
    uint16_t   data_len;  /**< Length of data. */
} data_t;

typedef struct{
    bool           is_connected;
    ble_gap_addr_t address;
} conn_peer_t;

static ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
static conn_peer_t                      m_connected_peers[NRF_BLE_LINK_COUNT];

static nrf_ble_gatt_t                   m_gatt;                                     /**< GATT module instance. */
static ble_uuid_t                       m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};  /**< Universally unique service identifier. */
static uint16_t                         m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;  /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

static const uint8_t pwm_pin = 2;
static const uint8_t dir_pin = 3;

static motor_t motor;
static battery_t battery;
static char _buffer[4096];

static uint32_t level;
static uint32_t level2;
static uint32_t level3;
static uint32_t level4;
static uint32_t level5;
static uint8_t const * altAddr;
static uint8_t const * tmpAddr;
static ble_gap_evt_adv_report_t const* temp_report;
static char const m_target_periph_name[] = "BlueCharm";
static char const m_target_phone_name[] = "Samsung Galaxy S7";
static uint8_t const target_mac[] = {0xb0,0x91,0x22,0xf7,0x6d,0x55};
static uint8_t const target_mac_rvr[] = {0x55,0x6d,0xf7,0x22,0x91,0xb0};
static const int rssiThresh = -40;
static char* tmpName;

static bool tags_nearby = false;
static bool tags_detected = false;
static int curRssi;
static bool were_tags_nearby = false;
static bool name_found = false;

typedef vec_t(char*) vec_string_t;
typedef vec_t(uint8_t) vec_byte_t;
typedef vec_t(vec_byte_t) vec_bytes_t;

static vec_bytes_t addrs;
static vec_string_t names;

static vec_byte_t testAddr;

static void check_addr_vec(vec_byte_t* addr, uint8_t const* target){
  uint8_t byte; size_t i;
  bool isMatch = false;
  printf("check_addr_vec: \n");
  vec_foreach(addr, byte, i){
     printf("\t In [%02x] -- Target [%02x]\n",byte,target[i]);
     if(byte == target[i]){
          isMatch = true;
     }else{isMatch = false;}
 }

//  if (memcmp(target, addr, 6)== 0){
  if (isMatch){
     printf("check_addr_vec: Addresses Match...\n");
  }else{
     printf("check_addr_vec: Addresses Don't Match...\n");
  }
}

void print_vec_str(vec_string_t * strings) {
  size_t i; char * string;
  vec_foreach(strings, string, i) {
    printf("Names[%zu] = \"%s\"\n", i, string);
  }
}

void print_vec_byte(vec_byte_t * bytes) {
  size_t i; uint8_t byte;
  vec_foreach(bytes, byte, i) {
    printf("Byte[%zu] = \"%02x\"\n", i, byte);
  }
}

void print_vec_bytes(vec_bytes_t * bytes) {
  size_t i;
  size_t j;
  uint8_t byte;
  vec_byte_t byte2;
  vec_foreach(bytes, byte2, i) {
    printf("Byte[%zu] = ", i);
     vec_foreach(&byte2, byte, j) {
          printf("%02x ", byte);
     }
  }
     printf("\r\n");
}

static uint32_t parse_nus_data(uint8_t * p_data){
     int i = 0;
     char * pch;
     bool flag_stop = false;
     vec_string_t strings;
     vec_byte_t tmps;
     vec_init(&strings);
     vec_init(&tmps);

     uint16_t num;
     printf ("Splitting string \"%s\" into tokens:\n",(char*) p_data);
     pch = strtok ((char*) p_data," :");
     while(pch != NULL){
          if(i == 0){
               if(strcmp(pch,"add_dev") == 0){
                    printf("parse_nus_data: --- Adding device\r\n");
               }else if(strcmp(pch,"del_dev") == 0){
                    printf("parse_nus_data: --- Removing device\r\n");
               }else{
                    flag_stop = true;
               }
          }
          if(i == 1){
               printf ("%s\n", pch);
               vec_push(&strings, pch);
          }
          if(i>1){
               num = (uint16_t)strtol(pch, NULL, 16);       // number base 16
               printf("%d (%X) \n", num,num);                        // print it as decimal
               vec_push(&tmps, num);
//               printf ("%s\n", pch);
          }
          pch = strtok (NULL, " :");
          if(flag_stop)
               break;
          i++;
     }
     
     vec_push(&addrs,tmps);
     vec_push(&names,&strings);
     print_vec_str(&names);
     print_vec_bytes(&addrs);
     return NRF_SUCCESS;
}



static ble_gap_scan_params_t const m_scan_params =
{
    .active   = 1,
    .interval = 0x00A0,
    .window   = 0x0050,
    .timeout  = 0x0000,
};

static void scan_start(void)
{
    ret_code_t ret;

    ret = sd_ble_gap_scan_start(&m_scan_params);
    APP_ERROR_CHECK(ret);
}


static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata){
    uint32_t   index = 0;
    uint8_t* p_data;


    p_data = p_advdata->p_data;
    while (index < p_advdata->data_len){
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type){
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->data_len = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}

static bool find_adv_name(ble_gap_evt_adv_report_t const * p_adv_report, char const * name_to_find){
    ret_code_t err_code;
    data_t     adv_data;
    data_t     dev_name;

    // Initialize advertisement report for parsing
    adv_data.p_data   = (uint8_t *)p_adv_report->data;
    adv_data.data_len = p_adv_report->dlen;

    //search for advertising names
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, &adv_data, &dev_name);
    if (err_code == NRF_SUCCESS){
        if (memcmp(name_to_find, dev_name.p_data, dev_name.data_len) == 0){
            return true;
        }
    }else{
        // Look for the short local name if it was not found as complete
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, &adv_data, &dev_name);
        if (err_code != NRF_SUCCESS){
            return false;
        }
        if (memcmp(m_target_periph_name, dev_name.p_data, dev_name.data_len) == 0){
            return true;
        }
    }
    return false;
}

static bool find_adv_uuid(ble_gap_evt_adv_report_t const * p_adv_report, uint16_t uuid_to_find){
    ret_code_t err_code;
    data_t     adv_data;
    data_t     type_data;

    // Initialize advertisement report for parsing.
    adv_data.p_data   = (uint8_t *)p_adv_report->data;
    adv_data.data_len = p_adv_report->dlen;

    err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE,&adv_data, &type_data);

    if (err_code != NRF_SUCCESS){
        // Look for the services in 'complete' if it was not found in 'more available'.
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE, &adv_data, &type_data);

        if (err_code != NRF_SUCCESS){
            // If we can't parse the data, then exit.
            return false;
        }
    }

    // Verify if any UUID match the given UUID.
    for (uint32_t i = 0; i < (type_data.data_len / sizeof(uint16_t)); i++){
        uint16_t extracted_uuid = uint16_decode(&type_data.p_data[i * sizeof(uint16_t)]);
        if (extracted_uuid == uuid_to_find){
            return true;
        }
    }
    return false;
}

static bool pet_proximity_check(ble_evt_t const * p_ble_evt){
     ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
     were_tags_nearby = tags_nearby;
     const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;

     temp_report = &p_gap_evt->params.adv_report;
     tmpName = (char*)temp_report->peer_addr.addr;
     tmpAddr = temp_report->peer_addr.addr;

     int tmpRssi = -10000;
     if (strlen(m_target_periph_name) != 0){
         if (memcmp(target_mac, tmpAddr, 6)== 0){
             tmpRssi = temp_report->rssi;
//             NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" Found Target Device! =====  %d\r\n",temp_report->rssi);
               tags_detected = true;
         }else if (memcmp(target_mac_rvr, tmpAddr, 6)== 0){
             tmpRssi = temp_report->rssi;
             tags_detected = true;
//             NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" Found Target Device Reverse! =====  %d\r\n",temp_report->rssi);
         }else if (strlen(_buffer) != 0){
//                    NRF_LOG_INFO("CENTRAL: Looking for alternative name for advertising peer (\'%s\')...\r\n",_buffer);
             if (find_adv_name(&p_gap_evt->params.adv_report, _buffer))
             {
                 if(!name_found){
                    NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"CENTRAL: Alternative Target (\'%s\') Found with MAC (\'",_buffer);
//                            altAddr = &p_gap_evt->params.adv_report.peer_addr.addr;
                    for (uint8_t i = 0; i < 6; i++)
                    {
                         NRF_LOG_RAW_INFO("%02x ", tmpAddr[i]);
                    }

                    NRF_LOG_RAW_INFO("\')!\r\n");
                    name_found = true;
                 }
                 tmpRssi = temp_report->rssi;
             }else{
                 tmpRssi = -10000;
             }
         }else{
             tmpRssi = -10000;
             tags_detected = false;
         }

         if(tmpRssi >= rssiThresh){
           tags_nearby = true;
           curRssi = tmpRssi;
//           NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"CENTRAL: Found Target Device Reverse! =====  %d\r\n",tmpRssi);
         }else{
           tags_nearby = false;
         }
     }
     return false;
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
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
    uint32_t err_code;

    NRF_LOG_INFO("Received data from BLE NUS. Handling data on UART....\r\n");
    NRF_LOG_HEXDUMP_DEBUG(p_data, length);

    err_code = parse_nus_data(p_data);

    memset(_buffer, 0, sizeof(_buffer));
    memcpy(&_buffer[0], (char*)p_data, sizeof(uint8_t)*length);
    NRF_LOG_INFO("nus_data_handler(): ----- Data = %s\r\n",_buffer);

    for (uint32_t i = 0; i < length; i++)
    {
        do
        {
            err_code = app_uart_put(p_data[i]);
            if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
            {
                NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. \r\n", err_code);
                APP_ERROR_CHECK(err_code);
            }
        } while (err_code == NRF_ERROR_BUSY);
    }
    if (p_data[length-1] == '\r')
    {
        while (app_uart_put('\n') == NRF_ERROR_BUSY);
    }

}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("services_init(): ----- Completed.\r\n");
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static bool is_already_connected(ble_gap_addr_t const * p_connected_adr){
    for (uint32_t i = 0; i < NRF_BLE_LINK_COUNT; i++){
        if (m_connected_peers[i].is_connected){
            if (m_connected_peers[i].address.addr_type == p_connected_adr->addr_type){
                if (memcmp(m_connected_peers[i].address.addr, p_connected_adr->addr, sizeof(m_connected_peers[i].address.addr)) == 0){
                    return true;
                }
            }
        }
    }
    return false;
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
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


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
//            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
//            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
//            sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for the application's SoftDevice event handler.
 *
 * @param[in] p_ble_evt SoftDevice event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
    uint32_t err_code;
    bool flag_pets_near = false;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
//            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
//            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

            NRF_LOG_INFO("Connected\r\n");
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_DISCONNECTED:
//            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
//            APP_ERROR_CHECK(err_code);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            NRF_LOG_INFO("Disconnected\r\n");
            break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GAP_EVT_SEC_PARAMS_REQUEST

         case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
        {
            ble_gap_data_length_params_t dl_params;

            // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
            memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
            err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_SYS_ATTR_MISSING

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_TIMEOUT

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_EVT_USER_MEM_REQUEST

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
        case BLE_GAP_EVT_ADV_REPORT:{
               bool do_connect = false;
               NRF_LOG_DEBUG("on_ble_evt: ===== SCANNING ADVERTISING PEERS...\r\n");
               flag_pets_near = pet_proximity_check(p_ble_evt);

               if (is_already_connected(&p_gap_evt->params.adv_report.peer_addr)){
                    NRF_LOG_INFO("central Already connected to something...\r\n");
                    break;
               }

               if (strlen(m_target_phone_name) != 0){
                    if (find_adv_name(&p_gap_evt->params.adv_report, m_target_phone_name)){
                         NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"CENTRAL: Host Phone Found!\r\n");
                         do_connect = true;
                    }else{}
               }else{
                    // We do not want to connect to two peripherals offering the same service, so when
                    // a UUID is matched, we check that we are not already connected to a peer which
                    // offers the same service.
                    if(find_adv_uuid(&p_gap_evt->params.adv_report, BLE_UUID_NUS_SERVICE) && (m_conn_handle == BLE_CONN_HANDLE_INVALID)){
                         do_connect = true;
                    }
               }
          } break; // BLE_GAP_ADV_REPORT

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a SoftDevice event to all modules with a SoftDevice
 *        event handler.
 *
 * @details This function is called from the SoftDevice event interrupt handler after a
 *          SoftDevice event has been received.
 *
 * @param[in] p_ble_evt  SoftDevice event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    nrf_ble_gatt_on_ble_evt(&m_gatt, p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);

}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize SoftDevice.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = softdevice_app_ram_start_get(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Overwrite some of the default configurations for the BLE stack.
    ble_cfg_t ble_cfg;

    // Configure the maximum number of connections.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum ATT MTU.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                 = CONN_CFG_TAG;
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = NRF_BLE_GATT_MAX_MTU_SIZE;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATT, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum event length.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                     = CONN_CFG_TAG;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = 320;
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = BLE_GAP_CONN_COUNT_DEFAULT;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = softdevice_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, const nrf_ble_gatt_evt_t * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)\r\n", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x\r\n", p_gatt->att_mtu_desired_central, p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, 64);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
//            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist();
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;
             NRF_LOG_INFO("uart_event_handle: -----  APP_UART_DATA_READY\r\n");
            if ((data_array[index - 1] == '\n') || (index >= (m_ble_nus_max_data_len)))
            {
                NRF_LOG_INFO("Ready to send data over BLE NUS\r\n");
                NRF_LOG_HEXDUMP_DEBUG(data_array, index);

                do
                {
                    err_code = ble_nus_string_send(&m_nus, data_array, index);
                    if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
                    {
                        APP_ERROR_CHECK(err_code);
                    }
                } while (err_code == NRF_ERROR_BUSY);

                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        .rx_pin_no    = UART_PIN_DISCONNECTED,
        .tx_pin_no    = UART_PIN_DISCONNECTED,
        .rts_pin_no   = UART_PIN_DISCONNECTED,
        .cts_pin_no   = UART_PIN_DISCONNECTED,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advdata_t          advdata;
    ble_advdata_t          scanrsp;
    ble_adv_modes_config_t options;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(CONN_CFG_TAG);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for placing the application in low power state while waiting for events.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

int tmpDuty;
uint8_t new_duty_cycle = 255;
/**@brief Application main function.
 */
int main(void)
{
    uint32_t err_code;
    bool     erase_bonds;
    vec_init(&addrs);
    vec_init(&names);
    vec_init(&testAddr);
    vec_push(&testAddr,0xb0); vec_push(&testAddr,0x91); vec_push(&testAddr,0x22);
    vec_push(&testAddr,0xf7); vec_push(&testAddr,0x6d); vec_push(&testAddr,0x55);

    check_addr_vec(&testAddr,&target_mac);
    
    // Initialize.
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    uart_init();
    log_init();

    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();

    printf("\r\nUART Start!\r\n");
    NRF_LOG_INFO("UART Start!\r\n");
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    scan_start();

    board_init();

    pwm_init(pwm_pin,dir_pin,&motor);
    buzzer_init(speaker_handler);
    battery_monitor_init(300000, battery_monitor_handler, &battery);
    new_duty_cycle = motor.pwm.period;
     
    NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"===== Thingy demo started! =====  \r\n"NRF_LOG_COLOR_CODE_DEFAULT);
     
    nrf_gpio_cfg_input(BUTTON, NRF_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(SX_IOEXT_0,DRV_EXT_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(SX_IOEXT_1,DRV_EXT_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(SX_IOEXT_2,DRV_EXT_GPIO_PIN_PULLUP);
    drv_ext_gpio_cfg_input(SX_IOEXT_3,DRV_EXT_GPIO_PIN_PULLUP);
    
    uint32_t tmpLevel;

    // Enter main loop.
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
         drv_ext_light_rgb_intensity_set(0,&color_purple);
       }else if((level == 0) && (level3 == 0)){
         tmpDuty = new_duty_cycle + 1;
         drv_ext_light_rgb_intensity_set(0,&color_darkpurple);
       }else if((level == 1) && (level3 == 0)){
          drv_ext_light_rgb_intensity_set(0,&color_aqua);
          initiate_alarm_sequence(2500,50,5,500,1000);
       }else{
         // Nothing
//              nrf_delay_ms(500);
//               drv_ext_light_rgb_intensity_set(0,&color_yellow);
//               nrf_delay_ms(500);
//               drv_ext_light_rgb_intensity_set(0,&color_orange);
//               nrf_delay_ms(500);
       }
          if((tags_detected) && (!tags_nearby)){
               drv_ext_light_rgb_intensity_set(0,&color_blue);
          }else if(tags_nearby){
               drv_ext_light_rgb_intensity_set(0,&color_green);
          }else{
               drv_ext_light_rgb_intensity_set(0,&color_black);
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
    }
}


/**
 * @}
 */
