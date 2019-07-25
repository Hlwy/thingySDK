#ifndef NUS_HELPERS_H__
#define NUS_HELPERS_H__

// // #include "ble_nus.h"
// #include "nrf_ble_gatt.h"
// #include "ble_hci.h"
//
// #include "nrf_log.h"
//
// // #include "vector_c.h"
// #include "nrf_calender.h"
// #include "dd_cmd_relay.h"
//
// #define CONN_CFG_TAG                    1                                           /**< A tag that refers to the BLE stack configuration we set with @ref sd_ble_cfg_set. Default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */
// #define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */
//
// static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
// static ble_gap_evt_adv_report_t const* ble_report;
//
// static int nBleUpdates;
// static int debounceCounter;
// static int debounceThresh = 35;
//
// static bool flag_close_door = true;
// static bool flag_debug = false;
// static bool flag_debug_rssi = false;
// static bool tags_nearby = false;
// static bool tags_detected = false;
// static bool were_tags_nearby = false;
// static bool name_found = false;
//
// static struct tm* tmpT;
// static time_t begin;
// static time_t end;
// static time_t now;
// static time_t last_time;
// static uint32_t myTimeStamp;
// static float dt;
//
// //extern vec_bytes_t tagAddrs;
//
// static bool pet_proximity_check(ble_evt_t const * p_ble_evt){
//      int dump;
//      ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
//      const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
//      if(flag_debug) NRF_LOG_INFO("pet_proximity_check: --------- \r\n");
//
//      ble_report = &p_gap_evt->params.adv_report;
//      uint8_t* ble_addr = (uint8_t*)ble_report->peer_addr.addr;
//      int tmpRssi = ble_report->rssi;;
//
//      bool isMatch = check_addr_vecs(&tagAddrs,ble_addr,&dump);
//      if(isMatch){
//           if(flag_debug_rssi) printf(NRF_LOG_COLOR_CODE_GREEN" Found Target Device! =====  %d\r\n"NRF_LOG_COLOR_CODE_DEFAULT,tmpRssi);
//           tags_detected = true;
//           if(tmpRssi >= rssiThresh){
//                tags_nearby = true;
//                return true;
//           }else
//                tags_nearby = false;
//      }else{
//           tags_detected = false;
//           tmpRssi = -10000;
//      }
//      return false;
// }
//
//
// /**@brief Function for the application's SoftDevice event handler. */
// static void on_ble_evt(ble_evt_t * p_ble_evt){
//      uint32_t err_code;
//      ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
//
//      if(nBleUpdates == 0){ last_time = begin; }
//      tmpT = nrf_cal_get_time_calibrated();
//      now = mktime(tmpT);
//      dt = (now - last_time)/3925.0;
//
//      if(dt >= 1.0){
//           if(flag_debug) NRF_LOG_INFO("on_ble_evt(): --- Current Time = %d Time since last BLE update = " NRF_LOG_FLOAT_MARKER "\n", now, NRF_LOG_FLOAT(dt));
//           last_time = now;
//      }
//
//      switch(p_ble_evt->header.evt_id){
//           case BLE_GAP_EVT_CONNECTED:{
//                m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//                if(flag_debug) NRF_LOG_INFO("Connected\r\n");
//           } break;
//
//           case BLE_GAP_EVT_DISCONNECTED:{
//                m_conn_handle = BLE_CONN_HANDLE_INVALID;
//                if(flag_debug) NRF_LOG_INFO("Disconnected\r\n");
//           } break;
//
//           case BLE_GAP_EVT_ADV_REPORT:{
//                bool flag_pets_near = false;
//                uint32_t now = millis();
//                uint32_t tmpdt = compareMillis(myTimeStamp, now);
//                float dt = tmpdt/(float)1000.0;
//                myTimeStamp = now;
//                // printf(NRF_LOG_FLOAT_MARKER" seconds have passed\n",NRF_LOG_FLOAT(dt));
//                if(nDevices>0){
//                     flag_pets_near = pet_proximity_check(p_ble_evt);
//                     if(flag_pets_near){
//                          debounceCounter = 0;
//                          flag_close_door = false;
//                          if(flag_debug) NRF_LOG_INFO("Opening Door...\r\n");
//                     }else{
//                          debounceCounter++;
//                          if(debounceCounter >= debounceThresh){
//                               flag_close_door = true;
//                               if(flag_debug) NRF_LOG_INFO("Closing Door...\r\n");
//                          }
//                     }
//                }else{
//                     debounceCounter = 0;
//                     flag_close_door = true;
//                }
//           } break;
//
//           /** Pairing not supported */
//           case BLE_GAP_EVT_SEC_PARAMS_REQUEST:{
//                err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
//                APP_ERROR_CHECK(err_code);
//           } break;
//
//           case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:{
//                ble_gap_data_length_params_t dl_params;
//                // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
//                memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
//                err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
//                APP_ERROR_CHECK(err_code);
//           } break;
//
//           /** No system attributes have been stored. */
//           case BLE_GATTS_EVT_SYS_ATTR_MISSING:{
//                err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
//                APP_ERROR_CHECK(err_code);
//           } break;
//
//           /** Disconnect on GATT Client timeout event. */
//           case BLE_GATTC_EVT_TIMEOUT:{
//                err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//                APP_ERROR_CHECK(err_code);
//           } break;
//
//           /** Disconnect on GATT Server timeout event. */
//           case BLE_GATTS_EVT_TIMEOUT:{
//                err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//                APP_ERROR_CHECK(err_code);
//           } break;
//
//           case BLE_EVT_USER_MEM_REQUEST:{
//                err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
//                APP_ERROR_CHECK(err_code);
//           } break;
//
//           case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:{
//                ble_gatts_evt_rw_authorize_request_t  req;
//                ble_gatts_rw_authorize_reply_params_t auth_reply;
//                req = p_ble_evt->evt.gatts_evt.params.authorize_request;
//
//                if(req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID){
//                     if(  (req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
//                          (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
//                          (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL) )
//                     {
//                          if(req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
//                               auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
//                          else
//                               auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
//                          auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
//                          err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
//                          APP_ERROR_CHECK(err_code);
//                     }
//                }
//           } break;
//      }
//      nBleUpdates++;
// }


#endif // NUS_HELPERS_H__
