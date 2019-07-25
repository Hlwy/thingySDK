#ifndef DD_CMD_RELAY_H__
#define DD_CMD_RELAY_H__

#include "ble_nus.h"
#include "nrf_ble_gatt.h"
#include "ble_hci.h"

#include "nrf_calender.h"
#include "nrf_battery_monitor.h"
#include "vector_c.h"
#include "nrf_log.h"

#define CONN_CFG_TAG                    1
#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2

typedef vec_t(const char*) vec_string_t;
typedef vec_t(uint8_t) vec_byte_t;
typedef vec_t(vec_byte_t) vec_bytes_t;
typedef vec_t(bool) vec_bool_t;

static ble_nus_t m_nus;
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;
static ble_gap_evt_adv_report_t const* ble_report;

static vec_bytes_t tagAddrs;
static vec_string_t tagNames;

static uint8_t* ble_addr;
static char* tmpName;
static int action_id;
static int nDevices = 0;
static int nBleUpdates = 0;
static int rssiThresh = -70;
static int debounceCounter = 0;
static int debounceThresh = 35;

static bool flag_close_door = true;
static bool flag_debug = false;
static bool flag_debug_rssi = false;
static bool tags_nearby = false;
static bool tags_detected = false;
static bool were_tags_nearby = false;
static bool name_found = false;

static float dt;
static time_t begin;
static time_t end;
static time_t now;
static time_t last_time;
static uint32_t myTimeStamp;
static struct tm* tmpT;

typedef enum{
     DD_CMD_TYPE_NONE,
     DD_CMD_TYPE_ADD_DEV,
     DD_CMD_TYPE_DEL_DEV,
     DD_CMD_TYPE_QUERY,
     DD_CMD_TYPE_GET,
     DD_CMD_TYPE_SET,
}dd_cmd_type_t;

typedef enum{
     DD_CMD_DATA_NONE,
     DD_CMD_DATA_RSSI_THRESHOLD,
     DD_CMD_DATA_BATTERY_INFO,
}dd_cmd_data_t;

void print_addr(uint8_t* addr);
void print_vec_str(vec_string_t * strings);
void print_vec_byte(vec_byte_t * bytes);
void print_vec_bytes(vec_bytes_t * bytes);


// bool check_addr_vec(vec_byte_t* addr, uint8_t* target);
// bool check_addr_vecs(vec_bytes_t* addrs_, uint8_t* target, int* foundIdx);

// void send_vec_bytes(vec_bytes_t * bytes);
// void send_stored_info();
// void send_misc_info(uint8_t type);

// bool pet_proximity_check(ble_evt_t const * p_ble_evt);
// uint32_t parse_nus_data(uint8_t * p_data);

static bool check_addr_vec(vec_byte_t* addr, uint8_t* target){
     char* tmpMatch;
     uint8_t byte; size_t i;
     bool isMatch = false;
     if(memcmp(target, addr->data,6)== 0) isMatch = true;
     else isMatch = false;
     if(!isMatch){
          // printf("check_addr_vec: checking address in reverse...\r\n");
          int j = 0;
          vec_foreach_rev(addr, byte, i){
               if(i>=6){
                    // printf("check_addr_vec: Forcing break...\r\n");
                    break;
               }
               if(byte == target[j]){isMatch = true;tmpMatch = "true";
               }else{isMatch = false;tmpMatch = "false";}
               // printf("\tByte[%d] - In [%02x] -- Target [%02x] --- Match = %s\n",i,byte,target[j],tmpMatch);
               j++;
          }
     }
     return isMatch;
}

static bool check_addr_vecs(vec_bytes_t* addrs_, uint8_t* target, int* foundIdx){
     int idxT;
     size_t i, idx;
     bool isMatch = false;
     bool flag_out;

     vec_byte_t byte_vec;
     vec_bool_t matches;
     vec_init(&matches);

     // printf("check_addr_vecs: Checking Known Addresses for Target - ");
     // print_addr(target); printf("\r\n");
     vec_foreach(addrs_,byte_vec,i){
          isMatch = check_addr_vec(&byte_vec,target);
          vec_push(&matches, isMatch);
     }
     vec_find(&matches, true, idxT);
     *foundIdx = idxT;
     if(idxT == -1) flag_out = false;
     else flag_out = true;
     vec_deinit(&matches);
     return flag_out;
}

static void send_vec_bytes(vec_bytes_t * bytes) {
     size_t i;
     vec_byte_t byte_vec;
     uint8_t data[128];

     vec_foreach(bytes, byte_vec, i){
          sprintf((char *)data, "Bytes[%zu] = %02x:%02x:%02x:%02x:%02x:%02x\r\n",i,byte_vec.data[0],byte_vec.data[1],byte_vec.data[2],byte_vec.data[3],byte_vec.data[4],byte_vec.data[5]);
          // printf("sending string:\r\n\t%s",(char*)data);
          ble_nus_string_send(&m_nus, data, 32);
     }
}

static void send_stored_info(){
     uint8_t data[32];
     if(nDevices == 0){
          char* buff = "No Devices Stored\r\n";
          sprintf((char *)data, buff);
          ble_nus_string_send(&m_nus, data, strlen(buff));
     }else{
          send_vec_bytes(&tagAddrs);
     }
}

static void send_misc_info(uint8_t type){
     uint8_t data[128];

     if(type == DD_CMD_DATA_RSSI_THRESHOLD){
          sprintf((char *)data, "RSSI Threshold = %d\r\n",rssiThresh);
          ble_nus_string_send(&m_nus, data, 32);
     }else if(type == DD_CMD_DATA_BATTERY_INFO){
          sprintf((char *)data, "Battery Voltage (mV) = %d (%d%%)\r\n",voltage,bat_percent);
          ble_nus_string_send(&m_nus, data, 34);
     }else{
          // send_vec_bytes(&tagAddrs);
     }
}


static uint32_t parse_nus_data(uint8_t * p_data){
     int i = 0;
     char * pch;

     bool flag_stop = false;
     char tmpStr[128];

     vec_string_t strings;
     vec_string_t cmds;
     vec_byte_t tmps;
     vec_init(&strings);
     vec_init(&cmds);
     vec_init(&tmps);

     // printf ("Splitting string \"%s\" into tokens:\n",(char*) p_data);
     pch = strtok ((char*) p_data," :");
     while(pch != NULL){
          vec_push(&cmds, pch);
          if(i == 0){
               // printf ("%s", pch);
               if(strcmp(pch,"add_dev") == 0){
                    // printf("parse_nus_data: --- Adding device\r\n");
                    action_id = DD_CMD_TYPE_ADD_DEV;
                    nDevices++;
               }else if(strcmp(pch,"del_dev") == 0){
                    // printf("parse_nus_data: --- Removing device\r\n");
                    action_id = DD_CMD_TYPE_DEL_DEV;
                    nDevices--;
               }else if(strcmp(pch,"query") == 0){
                    send_stored_info();
               }else if(strcmp(pch,"get") == 0){
                    action_id = DD_CMD_TYPE_GET;
               }else if(strcmp(pch,"set") == 0){
                    action_id = DD_CMD_TYPE_SET;
               }else{
                    action_id = DD_CMD_TYPE_NONE;
                    flag_stop = true;
               }
          }
          if(i == 1){
               if(action_id == DD_CMD_TYPE_ADD_DEV){
                    memset(tmpStr, 0, sizeof(tmpStr));
                    memcpy(&tmpStr[0], (char*)pch, sizeof(uint8_t)*(strlen(pch)));
                    const char* tmpName = (const char*)tmpStr;
                    vec_insert(&tagNames,0,tmpName);
               }else if(action_id == DD_CMD_TYPE_GET){
                    if(strcmp(pch,"rssi_thresh") == 0){
                         send_misc_info(DD_CMD_DATA_RSSI_THRESHOLD);
                    }else if(strcmp(pch,"battery") == 0){
                         send_misc_info(DD_CMD_DATA_BATTERY_INFO);
                    }
               }
          }
          if(i>1){
               if((action_id == DD_CMD_TYPE_ADD_DEV) || (action_id == DD_CMD_TYPE_DEL_DEV)){
                    uint16_t num = (uint16_t)strtol(pch, NULL, 16);
                    vec_push(&tmps, num);
               }else if(action_id == DD_CMD_TYPE_SET){
                    int32_t val = (int32_t)strtol(pch, NULL, 10);
                    rssiThresh = val;
                    // printf("Setting Value to %d...",val);
               }
          }
          pch = strtok (NULL, " :");
          if(flag_stop)
               break;
          i++;
     }
     if(action_id == DD_CMD_TYPE_ADD_DEV){
          // vec_push(&tagAddrs,tmps);
          vec_insert(&tagAddrs,0,tmps);
     }else if(action_id == DD_CMD_TYPE_DEL_DEV){
          int tmpIdx;
          bool isMatch;
          uint8_t tmpVec[6] = {tmps.data[0],tmps.data[1],tmps.data[2],tmps.data[3],tmps.data[4],tmps.data[5]};
          isMatch = check_addr_vecs(&tagAddrs,tmpVec,&tmpIdx);
          if(isMatch){
               bool dum = false;
               vec_splice(&tagAddrs, tmpIdx, 1);
               vec_compact(&tagAddrs);
          }
     }

     // printf ("%s\t", cmds.data[0]);
     // printf ("%s\t", cmds.data[1]);
     // printf ("%s", cmds.data[2]);
     // printf ("\r\n");

     // print_vec_str(&tagNames);
     // print_vec_bytes(&tagAddrs);

     // vec_deinit(&strings);
     // vec_deinit(&tmps);
     vec_deinit(&cmds);
     return NRF_SUCCESS;
}


static bool pet_proximity_check(ble_evt_t const * p_ble_evt){
     int dump;
     ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
     const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
     if(flag_debug) NRF_LOG_INFO("pet_proximity_check: --------- \r\n");

     ble_report = &p_gap_evt->params.adv_report;
     ble_addr = (uint8_t*)ble_report->peer_addr.addr;
     int tmpRssi = ble_report->rssi;;

     bool isMatch = check_addr_vecs(&tagAddrs,ble_addr,&dump);
     if(isMatch){
          if(flag_debug_rssi) printf(NRF_LOG_COLOR_CODE_GREEN" Found Target Device! =====  %d\r\n"NRF_LOG_COLOR_CODE_DEFAULT,tmpRssi);
          tags_detected = true;
          if(tmpRssi >= rssiThresh){
               tags_nearby = true;
               return true;
          }else
               tags_nearby = false;
     }else{
          tags_detected = false;
          tmpRssi = -10000;
     }
     return false;
}


/**@brief Function for the application's SoftDevice event handler. */
static void on_ble_evt(ble_evt_t * p_ble_evt){
     uint32_t err_code;
     ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

     if(nBleUpdates == 0){ last_time = begin; }
     tmpT = nrf_cal_get_time_calibrated();
     now = mktime(tmpT);
     dt = (now - last_time)/3925.0;

     if(dt >= 1.0){
          if(flag_debug) NRF_LOG_INFO("on_ble_evt(): --- Current Time = %d Time since last BLE update = " NRF_LOG_FLOAT_MARKER "\n", now, NRF_LOG_FLOAT(dt));
          last_time = now;
     }

     switch(p_ble_evt->header.evt_id){
          case BLE_GAP_EVT_CONNECTED:{
               m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
               if(flag_debug) NRF_LOG_INFO("Connected\r\n");
          } break;

          case BLE_GAP_EVT_DISCONNECTED:{
               m_conn_handle = BLE_CONN_HANDLE_INVALID;
               if(flag_debug) NRF_LOG_INFO("Disconnected\r\n");
          } break;

          case BLE_GAP_EVT_ADV_REPORT:{
               bool flag_pets_near = false;
               uint32_t now = millis();
               uint32_t tmpdt = compareMillis(myTimeStamp, now);
               float dt = tmpdt/(float)1000.0;
               myTimeStamp = now;
               // printf(NRF_LOG_FLOAT_MARKER" seconds have passed\n",NRF_LOG_FLOAT(dt));
               if(nDevices>0){
                    flag_pets_near = pet_proximity_check(p_ble_evt);
                    if(flag_pets_near){
                         debounceCounter = 0;
                         flag_close_door = false;
                         if(flag_debug) NRF_LOG_INFO("Opening Door...\r\n");
                    }else{
                         debounceCounter++;
                         if(debounceCounter >= debounceThresh){
                              flag_close_door = true;
                              if(flag_debug) NRF_LOG_INFO("Closing Door...\r\n");
                         }
                    }
               }else{
                    debounceCounter = 0;
                    flag_close_door = true;
               }
          } break;

          /** Pairing not supported */
          case BLE_GAP_EVT_SEC_PARAMS_REQUEST:{
               err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
               APP_ERROR_CHECK(err_code);
          } break;

          case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:{
               ble_gap_data_length_params_t dl_params;
               // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
               memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
               err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
               APP_ERROR_CHECK(err_code);
          } break;

          /** No system attributes have been stored. */
          case BLE_GATTS_EVT_SYS_ATTR_MISSING:{
               err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
               APP_ERROR_CHECK(err_code);
          } break;

          /** Disconnect on GATT Client timeout event. */
          case BLE_GATTC_EVT_TIMEOUT:{
               err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
               APP_ERROR_CHECK(err_code);
          } break;

          /** Disconnect on GATT Server timeout event. */
          case BLE_GATTS_EVT_TIMEOUT:{
               err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
               APP_ERROR_CHECK(err_code);
          } break;

          case BLE_EVT_USER_MEM_REQUEST:{
               err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
               APP_ERROR_CHECK(err_code);
          } break;

          case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:{
               ble_gatts_evt_rw_authorize_request_t  req;
               ble_gatts_rw_authorize_reply_params_t auth_reply;
               req = p_ble_evt->evt.gatts_evt.params.authorize_request;

               if(req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID){
                    if(  (req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                         (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                         (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL) )
                    {
                         if(req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                              auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                         else
                              auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                         auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                         err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
                         APP_ERROR_CHECK(err_code);
                    }
               }
          } break;
     }
     nBleUpdates++;
}

#endif // DD_CMD_RELAY_H__
