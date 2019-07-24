#ifndef NUS_HELPERS_H__
#define NUS_HELPERS_H__

#include "ble_nus.h"
#include "nrf_ble_gatt.h"
#include "ble_hci.h"

#include "nrf_log.h"

#include "vector_c.h"
#include "nrf_calender.h"

#define CONN_CFG_TAG                    1                                           /**< A tag that refers to the BLE stack configuration we set with @ref sd_ble_cfg_set. Default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */
#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */


typedef vec_t(const char*) vec_string_t;
typedef vec_t(uint8_t) vec_byte_t;
typedef vec_t(vec_byte_t) vec_bytes_t;
typedef vec_t(bool) vec_bool_t;

static ble_nus_t                        m_nus;
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */

static int nDevices = 0;

static vec_bytes_t addrs;
static vec_string_t names;

static int nTimeUpdates;
static int closeCounter;
static int closeCounterThresh = 35;
static bool flag_close_door = true;
static bool flag_debug = false;

static char* tmpName;
static uint8_t * tmpAddr;
static ble_gap_evt_adv_report_t const* temp_report;
static bool tags_nearby = false;
static bool tags_detected = false;
static int curRssi;
static bool were_tags_nearby = false;
static bool name_found = false;

static const int rssiThresh = -70;

static time_t begin;
static time_t last_time;
static time_t end;
static time_t now;
static struct tm* tmpT;

static float dtD;
static float dt;
static char _buffer[4096];


static bool check_addr_vec(vec_byte_t* addr, uint8_t* target){
     uint8_t byte; size_t i;
     bool isMatch = false;
     if (memcmp(target, addr->data,6)== 0){isMatch = true;
     }else{isMatch = false;}
     char* tmpMatch;
     if(!isMatch){
//          printf("check_addr_vec: checking address in reverse...\r\n");
          int j = 0;
          vec_foreach_rev(addr, byte, i){
               if(i>=6){
//                    printf("check_addr_vec: Forcing break...\r\n");
                    break;
               }
               if(byte == target[j]){isMatch = true;tmpMatch = "true";
               }else{isMatch = false;tmpMatch = "false";}
//               printf("\tByte[%d] - In [%02x] -- Target [%02x] --- Match = %s\n",i,byte,target[j],tmpMatch);
               j++;
          }
     }
//     if (isMatch){printf("check_addr_vec: Addresses Match...\n");
//     }else{printf("check_addr_vec: Addresses Don't Match...\n");}
     return isMatch;
}

static void send_vec_bytes(vec_bytes_t * bytes) {
  size_t i; size_t j;
  uint8_t byte;
  vec_byte_t byte2;
  uint8_t data[128];

  vec_foreach(bytes, byte2, i) {
//    printf("Bytes[%zu] = ", i);
    sprintf((char *)data, "Bytes[%zu] = %02x:%02x:%02x:%02x:%02x:%02x\r\n",i,byte2.data[0],byte2.data[1],byte2.data[2],byte2.data[3],byte2.data[4],byte2.data[5]);
//    printf("sending string:\r\n\t%s",(char*)data);
    ble_nus_string_send(&m_nus, data, 32);
//     vec_foreach(&byte2, byte, j){
////          printf("%02x ", byte);
//          sprintf((char *)data, "%02x ", byte);
//          ble_nus_string_send(&m_nus, data, 3);
//     }
//     sprintf((char *)data, "\r\n");
//     ble_nus_string_send(&m_nus, data, 4);
////     printf("\r\n");
  }
//     printf("sending string:\r\n\t%s",(char*)data);
}

static bool check_addr_vecs(vec_bytes_t* addrs, uint8_t* target, int* foundIdx){
  size_t i;
  size_t idx;
  int idxT;
  bool isMatch = false;
  vec_bool_t matches;
  vec_init(&matches);

  vec_byte_t byte_vec;
//  printf("check_addr_vecs: Checking Known Addresses for Target - ");
//  print_addr(target); printf("\r\n");
  vec_foreach(addrs,byte_vec,i){
//     printf("\tChecking addr[%d]: ",i);print_addr(byte_vec.data); printf("\r\n");
     isMatch = check_addr_vec(&byte_vec,target);
     vec_push(&matches, isMatch);
  }
  vec_find(&matches, true, idxT);
//  printf("\tvec_find returned = %d\n",idxT);
  *foundIdx = idxT;
  if(idxT == -1){
     vec_deinit(&matches);
     return false;
  }else{
     vec_deinit(&matches);
     return true;
  }
}

static void send_stored_info(){
//     printf("parse_nus_data: --- Sending info\r\n");
     uint8_t data[32];
     char* buff;
     if(nDevices == 0){
          char* buff = "No Devices Stored\r\n";
          sprintf((char *)data, buff);
          ble_nus_string_send(&m_nus, data, strlen(buff));
     }else{
          send_vec_bytes(&addrs);
     }

//     ble_nus_string_send(&m_nus, data, sizeof(data));
}

static void send_misc_info(uint8_t type){
     uint8_t data[128];

     if(type == 0){
          sprintf((char *)data, "RSSI Threshold = %d\r\n",rssiThresh);
          ble_nus_string_send(&m_nus, data, 32);
     }else{
//          send_vec_bytes(&addrs);
     }

}

static uint32_t parse_nus_data(uint8_t * p_data){
     int i = 0;
     int action_id = 0;
     char * pch;
     char tmpStr[4096];
     bool flag_stop = false;
//     bool flag_get_info = false;
     vec_string_t strings;
     vec_string_t cmds;
     vec_byte_t tmps;
     vec_init(&strings);
     vec_init(&cmds);
     vec_init(&tmps);

     uint16_t num;
//     printf ("Splitting string \"%s\" into tokens:\n",(char*) p_data);
     pch = strtok ((char*) p_data," :");
     while(pch != NULL){
          vec_push(&cmds, pch);
          if(i == 0){
//               printf ("%s", pch);
               if(strcmp(pch,"add_dev") == 0){
//                    printf("parse_nus_data: --- Adding device\r\n");
                    action_id = 1;
                    nDevices++;
               }else if(strcmp(pch,"del_dev") == 0){
//                    printf("parse_nus_data: --- Removing device\r\n");
                    action_id = 2;
                    nDevices--;
               }else if(strcmp(pch,"query") == 0){
                    send_stored_info();
               }else if(strcmp(pch,"get") == 0){
//                    flag_get_info = true;
                    action_id = 3;
               }else if(strcmp(pch,"set") == 0){
//                    flag_set_info = true;
                    action_id = 4;
               }else{
                    action_id = 0;
                    flag_stop = true;
//                    flag_get_info = false;
               }
          }
          if(i == 1){
               if(action_id == 1){
                    memset(tmpStr, 0, sizeof(tmpStr));
                    memcpy(&tmpStr[0], (char*)pch, sizeof(uint8_t)*(strlen(pch)));
                    const char* tmpName = (const char*)tmpStr;
                    vec_insert(&names,0,tmpName);
               }else if(action_id == 3){
                    if(strcmp(pch,"rssi_thresh") == 0){
                         send_misc_info(0);
                    }
               }
          }
          if(i>1){
               num = (uint16_t)strtol(pch, NULL, 16);       // number base 16
               vec_push(&tmps, num);
          }
          pch = strtok (NULL, " :");
          if(flag_stop)
               break;
          i++;
     }
     if(action_id == 1){
          vec_insert(&addrs,0,tmps);
     }else if(action_id == 2){
          int tmpIdx;
          bool isMatch;
          uint8_t hmm[6] = {tmps.data[0],tmps.data[1],tmps.data[2],tmps.data[3],tmps.data[4],tmps.data[5]};
          isMatch = check_addr_vecs(&addrs,hmm,&tmpIdx);
          if(isMatch){
              bool dum = false;
              vec_splice(&addrs, tmpIdx, 1);
          }
     }

//     print_vec_str(&names);
//     print_vec_bytes(&addrs);

//     printf ("%s\t", cmds.data[0]);
//     printf ("%s\t", cmds.data[1]);
//     printf ("%s", cmds.data[2]);
//     printf ("\r\n");
     vec_deinit(&cmds);

     return NRF_SUCCESS;
}

//static uint32_t now;
static uint32_t myTimeStamp;

static bool pet_proximity_check(ble_evt_t const * p_ble_evt){
//     NRF_LOG_INFO("pet_proximity_check: --------- \r\n");
     ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
     were_tags_nearby = tags_nearby;
     const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;

     temp_report = &p_gap_evt->params.adv_report;
     tmpName = (char*)temp_report->peer_addr.addr;
     tmpAddr = (uint8_t*)temp_report->peer_addr.addr;

     int tmpRssi = -10000;
     int dummy;
     bool isMatch = check_addr_vecs(&addrs,tmpAddr,&dummy);
     if(isMatch){
          tmpRssi = temp_report->rssi;
          tags_detected = true;
          if(flag_debug) NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" Found Target Device! =====  %d\r\n"NRF_LOG_COLOR_CODE_DEFAULT,tmpRssi);
//          printf(NRF_LOG_COLOR_CODE_GREEN" Found Target Device! =====  %d\r\n"NRF_LOG_COLOR_CODE_DEFAULT,tmpRssi);
          if(tmpRssi >= rssiThresh){
               tags_nearby = true;
               return true;
          }else{
               tags_nearby = false;
          }
     }else{
          tags_detected = false;
          tmpRssi = -10000;
     }
     return false;
}

/**@brief Function for the application's SoftDevice event handler.
 *
 * @param[in] p_ble_evt SoftDevice event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
    uint32_t err_code;

    if(nTimeUpdates == 0){
          last_time = begin;
    }
    tmpT = nrf_cal_get_time_calibrated();
    now = mktime(tmpT);
    dt = (now - last_time)/3925.0;

    if(dt >= 1.0){
//          NRF_LOG_INFO("on_ble_evt(): --- Current Time = %d Time since last BLE update = " NRF_LOG_FLOAT_MARKER "\n", now, NRF_LOG_FLOAT(dt));
         last_time = now;
    }
    nTimeUpdates++;
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
               bool flag_pets_near = false;
               NRF_LOG_DEBUG("on_ble_evt: ===== SCANNING ADVERTISING PEERS...\r\n");
               uint32_t now = millis();
               uint32_t tmpdt = compareMillis(myTimeStamp, now);
               float dt = tmpdt/(float)1000.0;
//               printf(NRF_LOG_FLOAT_MARKER" seconds have passed\n",NRF_LOG_FLOAT(dt));
               myTimeStamp = now;
               if(nDevices>0){
                    flag_pets_near = pet_proximity_check(p_ble_evt);
                    if(flag_pets_near){
                         closeCounter = 0;
                         flag_close_door = false;
                         if(flag_debug) NRF_LOG_INFO("Opening Door...\r\n");
                    }else{
                         closeCounter++;
                         if(closeCounter >= closeCounterThresh){
                              flag_close_door = true;
                              if(flag_debug) NRF_LOG_INFO("Closing Door...\r\n");
                         }
                    }
               }else{
                    closeCounter = 0;
                    flag_close_door = true;
               }
            // No implementation needed.
            break;
       }
    }
}


#endif // NUS_HELPERS_H__
