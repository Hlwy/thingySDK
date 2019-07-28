#ifndef DD_CMD_RELAY_H__
#define DD_CMD_RELAY_H__

#include <string.h>
#include <stdlib.h>

#include "ble_nus.h"
#include "nrf_ble_gatt.h"
#include "ble_hci.h"

#include "nrf_calender.h"
#include "nrf_battery_monitor.h"
#include "vector_c.h"
#include "nrf_log.h"

#define CONN_CFG_TAG                    1
#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2
#define DEFAULT_RSSI_THRESHOLD          -70

#define DEBUGGING
//#define DEBUG_CHECKING
#define DEBUG_COMMANDS

typedef enum{
     DD_CMD_TYPE_NONE,
     DD_CMD_TYPE_ADD_DEV,
     DD_CMD_TYPE_DEL_DEV,
     DD_CMD_TYPE_QUERY,
     DD_CMD_TYPE_GET,
     DD_CMD_TYPE_SET,
}dd_cmd_type_t;

typedef enum{
     DD_CMD_NONE,                       /** Id = 0 */
     DD_CMD_BATTERY_LEVEL,              /** Id = 1 */
     DD_CMD_CHARGE_STATUS,              /** Id = 2 */
     DD_CMD_BATTERY_INFO,               /** Id = 3 */
     DD_CMD_BLE_UPDATE_RATE,            /** Id = 4 */
     DD_CMD_DOOR_UPDATE_RATE,           /** Id = 5 */
     DD_CMD_TAG_CHECK_UPDATE_RATE,      /** Id = 6 */
     DD_CMD_TOGGLE_DOOR_OPEN,           /** Id = 7 */
     DD_CMD_OPEN_DOOR,                  /** Id = 8 */
     DD_CMD_CLOSE_DOOR,                 /** Id = 9 */
     DD_CMD_TOGGLE_DOOR_LOCK,           /** Id = 10 */
     DD_CMD_LOCK_DOOR,                  /** Id = 11 */
     DD_CMD_UNLOCK_DOOR,                /** Id = 12 */
     DD_CMD_DOOR_ENCODER_LIMIT,         /** Id = 13 */
     DD_CMD_DOOR_SPEED,                 /** Id = 14 */
     DD_CMD_DOOR_STATUS,                /** Id = 15 */
     DD_CMD_TAG_ID,                     /** Id = 16 */
     DD_CMD_TAG_ALIAS,                  /** Id = 18 */
     DD_CMD_TAG_ADDRESS,                /** Id = 19 */
     DD_CMD_TAG_RSSI,                   /** Id = 20 */
     DD_CMD_TAG_RSSI_THRESHOLD,         /** Id = 21 */
     DD_CMD_TAG_DEBOUNCE_THRESHOLD,     /** Id = 22 */
     DD_CMD_TAG_TIMEOUT,                /** Id = 23 */
     DD_CMD_TAG_LAST_ACTIVITY,          /** Id = 24 */
     DD_CMD_TAG_STATISTICS,             /** Id = 25 */
}dd_cmd_t;

typedef struct{
     char name[128];
     uint8_t addr[6];
     char alias[128];
     int thresh;
     int rssi;
     int timeout;
}dd_tag_t;

typedef vec_t(const char*) vec_string_t;
typedef vec_t(uint8_t) vec_byte_t;
typedef vec_t(vec_byte_t) vec_bytes_t;
typedef vec_t(bool) vec_bool_t;
typedef vec_t(dd_tag_t) vec_tags_t;

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;
static ble_gap_evt_adv_report_t const* ble_report;

static vec_bytes_t tagAddrs;
static vec_string_t tagNames;
static vec_tags_t m_tags;

static uint8_t* ble_addr;
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

static struct cmd_entry{
    char *str;
    dd_cmd_t n;
}cmd_entry;

/* For mapping command string to command id */
static struct cmd_entry cmd_dict[] = {
    "battery_level", DD_CMD_BATTERY_LEVEL,
    "charge_status", DD_CMD_CHARGE_STATUS,
    "battery", DD_CMD_BATTERY_INFO,
    "ble_rate", DD_CMD_BLE_UPDATE_RATE,
    "door_update_rate", DD_CMD_DOOR_UPDATE_RATE,
    "tag_check_rate", DD_CMD_TAG_CHECK_UPDATE_RATE,
    "toggle_door_open", DD_CMD_TOGGLE_DOOR_OPEN,
    "open_door", DD_CMD_OPEN_DOOR,
    "close_door", DD_CMD_CLOSE_DOOR,
    "toggle_door_lock", DD_CMD_TOGGLE_DOOR_LOCK,
    "lock_door", DD_CMD_LOCK_DOOR,
    "unlock_door", DD_CMD_UNLOCK_DOOR,
    "encoder_limit", DD_CMD_DOOR_ENCODER_LIMIT,
    "door_speed", DD_CMD_DOOR_SPEED,
    "door_status", DD_CMD_DOOR_STATUS,
    "tag_id", DD_CMD_TAG_ID,
    "tag_alias", DD_CMD_TAG_ALIAS,
    "tag_addr", DD_CMD_TAG_ADDRESS,
    "tag_rssi", DD_CMD_TAG_RSSI,
    "rssi", DD_CMD_TAG_RSSI,
    "tag_rssi_thresh", DD_CMD_TAG_RSSI_THRESHOLD,
    "rssi_thresh", DD_CMD_TAG_RSSI_THRESHOLD,
    "tag_debounce_thresh", DD_CMD_TAG_DEBOUNCE_THRESHOLD,
    "tag_timeout", DD_CMD_TAG_TIMEOUT,
    "tag_stats", DD_CMD_TAG_STATISTICS,
    "tag_last_act", DD_CMD_TAG_LAST_ACTIVITY,
};


void print_addr(const uint8_t* addr);
void print_vec_str(const vec_string_t * strings);
void print_vec_byte(const vec_byte_t * bytes);
void print_vec_bytes(const vec_bytes_t * bytes);


// bool check_addr_vec(vec_byte_t* addr, uint8_t* target);
// bool check_addr_vecs(vec_bytes_t* addrs_, uint8_t* target, int* foundIdx);

// void send_vec_bytes(vec_bytes_t * bytes);
// void send_stored_info();
// void send_misc_info(uint8_t type);

// bool pet_proximity_check(ble_evt_t const * p_ble_evt);
// uint32_t parse_nus_data(uint8_t * p_data);

//static bool check_addr_vec(const vec_byte_t* addr, const uint8_t* target){
//     char* tmpMatch;
//     uint8_t byte; size_t i;
//     bool isMatch = false;
//     if(memcmp(target, addr->data,6)== 0)
//          isMatch = true;
//     else isMatch = false;
//     if(!isMatch){
//          // printf("check_addr_vec: checking address in reverse...\r\n");
//          int j = 0;
//          vec_foreach_rev(addr, byte, i){
//               if(i>=6){
//                    // printf("check_addr_vec: Forcing break...\r\n");
//                    break;
//               }
//               if(byte == target[j]){isMatch = true;tmpMatch = "true";
//               }else{isMatch = false;tmpMatch = "false";}
//               // printf("\tByte[%d] - In [%02x] -- Target [%02x] --- Match = %s\n",i,byte,target[j],tmpMatch);
//               j++;
//          }
//     }
//     return isMatch;
//}

//static bool check_addr_vecs(vec_bytes_t* addrs_, uint8_t* target, int* foundIdx){
//     int idxT;
//     size_t i, idx;
//     bool isMatch = false;
//     bool flag_out;
//
//     vec_byte_t byte_vec;
//     vec_bool_t matches;
//     vec_init(&matches);
//
//     // printf("check_addr_vecs: Checking Known Addresses for Target - ");
//     // print_addr(target); printf("\r\n");
//     vec_foreach(addrs_,byte_vec,i){
//          isMatch = check_addr_vec(&byte_vec,target);
//          vec_push(&matches, isMatch);
//     }
//     vec_find(&matches, true, idxT);
//     *foundIdx = idxT;
//     if(idxT == -1) flag_out = false;
//     else flag_out = true;
//     vec_deinit(&matches);
//     return flag_out;
//}

//static void send_vec_bytes(ble_nus_t * p_nus, vec_bytes_t * bytes, uint16_t length) {
//     uint8_t data[128];
//     for(size_t i = 0; i < length; i++){
//          sprintf((char *)data, "Bytes[%zu] = %02x:%02x:%02x:%02x:%02x:%02x\r\n",i,bytes->data[i].data[0],bytes->data[i].data[1],bytes->data[i].data[2],bytes->data[i].data[3],bytes->data[i].data[4],bytes->data[i].data[5]);
//          // printf("sending string:\r\n\t%s",(char*)data);
//          ble_nus_string_send(p_nus, data, 32);
//     }
//}

static bool check_addr_vec_reverse(const uint8_t* addr, const uint8_t* target){
     uint8_t byte;
     bool isMatch = false;

     for(int i = 0; i < 6; i++){
          byte = addr[5-i];
          if(byte == target[i]) isMatch = true;
          else isMatch = false;
#ifdef DEBUG_CHECKING
          char* descMatch = (isMatch ? "True":"False");
          printf("\tByte[%d] - In [%02x] -- Target [%02x] --- Match = %s\n",i,byte,target[i],descMatch);
#endif
     }
     return isMatch;
}

static bool check_addr_vec(const uint8_t* addr, const uint8_t* target){
     bool isMatch = false;

     if(memcmp(target, addr,6)== 0){
#ifdef DEBUG_CHECKING
          printf("check_addr_vec: found address normally.\r\n");
#endif
          isMatch = true;
     }else if(check_addr_vec_reverse(addr,target)){
#ifdef DEBUG_CHECKING
          printf("check_addr_vec: found address in reverse.\r\n");
#endif
          isMatch = true;
     }else{
#ifdef DEBUG_CHECKING
          printf("check_addr_vec: Could not find target address.\r\n");
#endif
          isMatch = false;
     }
     return isMatch;
}

static bool check_tag_addrs(const vec_tags_t* tags_, const uint8_t* target, int* foundIdx){
     int idx;
     bool flag_out;

     int nTags = tags_->length;
     bool isMatch = false;

     vec_bool_t matches;
     vec_init(&matches);
#ifdef DEBUG_CHECKING
     printf("check_tag_addrs: Checking [%d] Known Addresses for Target - ",nTags);
     print_addr(target); printf("\r\n");
#endif
     for(int i = 0; i < nTags;i++){
          uint8_t* testAddr = tags_->data[i].addr;
          isMatch = check_addr_vec(testAddr,target);
          vec_push(&matches, isMatch);
#ifdef DEBUG_CHECKING
          char* descMatch = (isMatch ? "True":"False");
          printf("\tChecking address [");print_addr(target); printf("] - isMatch = %s\r\n",descMatch);
#endif
     }
     vec_find(&matches, true, idx);
     *foundIdx = idx;
     if(idx == -1){
          flag_out = false;
     }else{
          flag_out = true;
#ifdef DEBUG_CHECKING
          printf("\check_tag_addrs - Found match at %d\n",idx);
#endif
     }
     vec_deinit(&matches);
     return flag_out;
}


static bool check_tag_names(const vec_tags_t* tags_, const char* target, int* foundIdx){
     int idx;
     bool flag_out;

     int nTags = tags_->length;
     bool isNameMatch = false;
     bool isAliasMatch = false;

     vec_bool_t matches;
     vec_init(&matches);
#ifdef DEBUG_CHECKING
     printf("check_tag_names: Checking [%d] Known Addresses for Target [%s]...\r\n",nTags,target);
#endif
     for(int i = 0; i < nTags;i++){
          bool testMatch = false;
          const char* testName = tags_->data[i].name;
          const char* testAlias = tags_->data[i].alias;
          if(strcmp(testName,target) == 0){
               isNameMatch = true;
               // printf("check_tag_names - Found Name match at %d\n",i);
          }
          if(strcmp(testAlias,target) == 0){
               isAliasMatch = true;
               // printf("check_tag_names - Found Alias match at %d\n",i);
          }
          if(isAliasMatch || isNameMatch) testMatch = true;
#ifdef DEBUG_CHECKING
          char* descMatch = (isMatch ? "True":"False");
          printf("\tChecking Name [");print_addr(target); printf("] - isMatch = %s\r\n",descMatch);
#endif
          vec_push(&matches, testMatch);
     }
     vec_find(&matches, true, idx);
     *foundIdx = idx;
     if(idx == -1){
          flag_out = false;
     }else{
          flag_out = true;
#ifdef DEBUG_CHECKING
          printf("\check_tag_names - Found match at %d\n",idx);
#endif
     }
     vec_deinit(&matches);
     return flag_out;
}

static uint32_t parse_nus_commands(const uint8_t * p_data, vec_string_t* outputs){
     int i = 0;
     char * pch;
     bool flag_stop = false;

     // printf ("Splitting string \"%s\" into tokens:\n",(char*) p_data);
     pch = strtok ((char*) p_data," :");
     while(pch != NULL){
          vec_push(outputs, pch);
          pch = strtok (NULL, " :");
          if(flag_stop)
               break;
          i++;
     }
#ifdef DEBUG_COMMANDS
     for(int n = 0; n < i; n++){
          const char* ch = outputs->data[n];
          printf("parse_nus_commands: --- cmd [%d] = %s\r\n",n,ch);
     }
#endif
     return (uint32_t) i;
}

static int isValidCommand(const char* cmd){
     int code;

     if(strcmp(cmd,"add_dev") == 0)
          code = DD_CMD_TYPE_ADD_DEV;
     else if(strcmp(cmd,"del_dev") == 0)
          code = DD_CMD_TYPE_DEL_DEV;
     else if(strcmp(cmd,"query") == 0)
          code = DD_CMD_TYPE_QUERY;
     else if(strcmp(cmd,"get") == 0)
          code = DD_CMD_TYPE_GET;
     else if(strcmp(cmd,"set") == 0)
          code = DD_CMD_TYPE_SET;
     else
          code = -1;
     return code;
}

static void report_tag_data(ble_nus_t* p_nus, const vec_tags_t* tags_, uint16_t length){
     uint8_t data[128];
     for(int i = 0; i < length; i++){
          memset(data, 0, sizeof(data));
          dd_tag_t tmpTag = tags_->data[i];
          size_t sz = snprintf(NULL, 0, "%s %02x:%02x:%02x:%02x:%02x:%02x %d\r\n",tmpTag.name,tmpTag.addr[0],tmpTag.addr[1],tmpTag.addr[2],tmpTag.addr[3],tmpTag.addr[4],tmpTag.addr[5],tmpTag.thresh);
          sprintf((char *)data, "%s %02x:%02x:%02x:%02x:%02x:%02x %d\r\n",tmpTag.name,tmpTag.addr[0],tmpTag.addr[1],tmpTag.addr[2],tmpTag.addr[3],tmpTag.addr[4],tmpTag.addr[5],tmpTag.thresh);
          ble_nus_string_send(p_nus, data,sz+1);
     }
}

static uint32_t send_stored_info(ble_nus_t* p_nus, const vec_tags_t* tags_, uint16_t length){
     uint8_t data[32];
     int num_tags = tags_->length;
     if(num_tags <= 0){
          char* buff = "No Devices Stored\r\n";
          sprintf((char *)data, buff);
          ble_nus_string_send(p_nus, data, strlen(buff));
          return NRF_SUCCESS;
     }

     // printf("Currently have %d tags stored.\r\n",num_tags);
     report_tag_data(p_nus, tags_, length);
     return NRF_SUCCESS;
}

static void send_misc_info(ble_nus_t* p_nus, uint8_t type){
     uint8_t data[128];

     if(type == DD_CMD_TAG_RSSI_THRESHOLD){
          sprintf((char *)data, "RSSI Threshold = %d\r\n",rssiThresh);
          ble_nus_string_send(p_nus, data, 32);
     }else if(type == DD_CMD_BATTERY_INFO){
          sprintf((char *)data, "Battery Voltage (mV) = %d (%d%%)\r\n",voltage,bat_percent);
          ble_nus_string_send(p_nus, data, 34);
     }else{
          // send_vec_bytes(&tagAddrs);
     }
}

static int compare_cmds(const void *s1, const void *s2){
     const struct cmd_entry *e1 = s1;
     const struct cmd_entry *e2 = s2;
     return strcmp(e1->str, e2->str);
}

static dd_cmd_t interpret_cmd(ble_nus_t* p_nus, const char* cmd_){
     char* input = (char*) cmd_;
     size_t nObjs = sizeof(cmd_dict)/sizeof(cmd_dict[0]);
     // printf("interpret_cmd: Searching for input [%s] among %d known commands...\r\n",input,nObjs);
     qsort(cmd_dict, nObjs, sizeof(struct cmd_entry), compare_cmds);
     struct cmd_entry *result, key = {input};
     result = bsearch(&key, cmd_dict, nObjs, sizeof(struct cmd_entry), compare_cmds);

     // printf("%s - %d", result->str, result->n);
     // if(result->n == DD_CMD_TAG_RSSI_THRESHOLD) printf(" Matched\r\n");
     // else printf(" No Match\r\n");
     return (dd_cmd_t) result->n;
}

static uint32_t parse_nus_data(ble_nus_t * p_nus, const uint8_t * p_data){
     int testIdx;
     int targetIdx = -2;

     bool isMatch;
     bool isValid = false;
     bool flag_stop = false;
     dd_cmd_t cmd_id = DD_CMD_NONE;

     char tmpName[4096];
     memset(tmpName, '\0', sizeof(tmpName));
     uint8_t tmpAddr[6];
     memset(tmpAddr, 0, sizeof(tmpAddr));

     vec_string_t cmds;       vec_init(&cmds);
     uint32_t nCmds = parse_nus_commands(p_data,&cmds);
     int cmd_type = isValidCommand(cmds.data[0]);

     if(cmd_type < 0){
          vec_deinit(&cmds);
          return NRF_ERROR_INVALID_DATA;
     }else if(cmd_type == DD_CMD_TYPE_QUERY){
          send_stored_info(p_nus, &m_tags, nDevices);
          vec_deinit(&cmds);
          return NRF_SUCCESS;
     }else if( (cmd_type == DD_CMD_TYPE_ADD_DEV) || (cmd_type == DD_CMD_TYPE_DEL_DEV)){
          strcpy(tmpName, cmds.data[1]);
          // printf("Provided Info for Tag: %s [",tmpName);
          for(int i = 0; i < 6; i++){
               uint16_t num = (uint16_t)strtol(cmds.data[i + 2], NULL, 16);
               tmpAddr[i] = num;
               // printf("%02x:",tmpAddr[i]);
          }
          // printf("\b]\r\n");
     }else if((cmd_type == DD_CMD_TYPE_GET) || (cmd_type == DD_CMD_TYPE_SET)){
          int tmpIdx, tmpIdx2;
          cmd_id = interpret_cmd(p_nus,cmds.data[1]);
          if(cmd_id >= DD_CMD_TAG_ID){
               // printf("Handling Pet Tag specific data. Need to know Specific pet's tag id...\r\n");
               if(check_tag_names(&m_tags,cmds.data[2],&tmpIdx)) targetIdx = tmpIdx;
               else if(check_tag_addrs(&m_tags,cmds.data[2],&tmpIdx2)) targetIdx = tmpIdx2;
               else{
                    // printf("[ERROR] parse_nus_data() ---- \'%s\' The DoggyTag ID given [%s] is invalid, or not recognized.\r\n",cmds.data[1],cmds.data[2]);
                    targetIdx = -1;
                    vec_deinit(&cmds);
                    return NRF_ERROR_INVALID_PARAM;
               }
          }// else printf("Handling Doogy Door Controller (DDC) specific data...\r\n");
     }

     // TODO: Implement dd_parameter_handler() function
     if(cmd_type == DD_CMD_TYPE_GET){
          // dd_parameter_handler(cmd_id, targetIdx, false, NULL, p_nus);
     }else if(cmd_type == DD_CMD_TYPE_SET){
          // dd_parameter_handler(cmd_id, targetIdx, true, cmds.data[3], p_nus);
          // int32_t val = (int32_t)strtol(cmds.data[3], NULL, 10);
          // printf("Setting Value to %d...",val);
     }

     if(cmd_type == DD_CMD_TYPE_ADD_DEV){
          dd_tag_t tmpTag;
          memset(&tmpTag, 0, sizeof(tmpTag));
          strcpy(&tmpTag.name[0], tmpName);
          tmpTag.addr[0] = tmpAddr[0];
          tmpTag.addr[1] = tmpAddr[1];
          tmpTag.addr[2] = tmpAddr[2];
          tmpTag.addr[3] = tmpAddr[3];
          tmpTag.addr[4] = tmpAddr[4];
          tmpTag.addr[5] = tmpAddr[5];
          tmpTag.thresh = DEFAULT_RSSI_THRESHOLD;
          vec_insert(&m_tags,0,tmpTag);
          nDevices = m_tags.length;
     }else if(cmd_type == DD_CMD_TYPE_DEL_DEV){
          testIdx = 0;
          isMatch = check_tag_addrs(&m_tags,tmpAddr,&testIdx);
          if(isMatch){
               vec_splice(&m_tags, testIdx, 1);
               vec_compact(&m_tags);
          }
          nDevices = m_tags.length;
     }

     vec_deinit(&cmds);
     return NRF_SUCCESS;
}

//static uint32_t parse_nus_data(uint8_t * p_data){
//     int i = 0;
//     char * pch;
//     int defaultRssiThresh = -70;
//
//     bool flag_stop = false;
//     char tmpStr[4096];
//     memset(tmpStr, '\0', sizeof(tmpStr));
//
//     vec_string_t strings;
//     vec_string_t cmds;
//     vec_byte_t tmps;
//     vec_init(&strings);
//     vec_init(&cmds);
//     vec_init(&tmps);
//
//     // printf ("Splitting string \"%s\" into tokens:\n",(char*) p_data);
//     pch = strtok ((char*) p_data," :");
//     while(pch != NULL){
//          vec_push(&cmds, pch);
//          if(i == 0){
//               // printf ("%s", pch);
//               if(strcmp(pch,"add_dev") == 0){
//                    // printf("parse_nus_data: --- Adding device\r\n");
//                    action_id = DD_CMD_TYPE_ADD_DEV;
//                    nDevices++;
//               }else if(strcmp(pch,"del_dev") == 0){
//                    // printf("parse_nus_data: --- Removing device\r\n");
//                    action_id = DD_CMD_TYPE_DEL_DEV;
//                    nDevices--;
//               }else if(strcmp(pch,"query") == 0){
//                    action_id = DD_CMD_TYPE_QUERY;
//                    send_stored_info(nDevices);
//               }else if(strcmp(pch,"get") == 0){
//                    action_id = DD_CMD_TYPE_GET;
//               }else if(strcmp(pch,"set") == 0){
//                    action_id = DD_CMD_TYPE_SET;
//               }else{
//                    action_id = DD_CMD_TYPE_NONE;
//                    flag_stop = true;
//               }
//          }
//          if(i == 1){
//               if(action_id == DD_CMD_TYPE_ADD_DEV){
//                    strcpy(tmpStr, pch);
//                    // printf("parse_nus_data: --- Adding device name = %s\r\n",tmpStr);
//                    vec_insert(&tagNames,0,tmpStr);
//               }else if(action_id == DD_CMD_TYPE_GET){
//                    if(strcmp(pch,"rssi_thresh") == 0){
//                         send_misc_info(DD_CMD_DATA_RSSI_THRESHOLD);
//                    }else if(strcmp(pch,"battery") == 0){
//                         send_misc_info(DD_CMD_DATA_BATTERY_INFO);
//                    }
//               }
//          }
//          if(i>1){
//               if((action_id == DD_CMD_TYPE_ADD_DEV) || (action_id == DD_CMD_TYPE_DEL_DEV)){
//                    uint16_t num = (uint16_t)strtol(pch, NULL, 16);
//                    vec_push(&tmps, num);
//               }else if(action_id == DD_CMD_TYPE_SET){
//                    int32_t val = (int32_t)strtol(pch, NULL, 10);
//                    rssiThresh = val;
//                    // printf("Setting Value to %d...",val);
//               }
//          }
//          pch = strtok (NULL, " :");
//          if(flag_stop)
//               break;
//          i++;
//     }
//     if(action_id == DD_CMD_TYPE_ADD_DEV){
//          dd_tag_t tmpTag;
//          memset(&tmpTag, 0, sizeof(tmpTag));
//          strcpy(&tmpTag.name[0], tmpStr);
//          tmpTag.addr[0] = tmps.data[0];
//          tmpTag.addr[1] = tmps.data[1];
//          tmpTag.addr[2] = tmps.data[2];
//          tmpTag.addr[3] = tmps.data[3];
//          tmpTag.addr[4] = tmps.data[4];
//          tmpTag.addr[5] = tmps.data[5];
//          tmpTag.thresh = defaultRssiThresh;
//          vec_insert(&m_tags,0,tmpTag);
//          // vec_push(&tagAddrs,tmps);
//          vec_insert(&tagAddrs,0,tmps);
//     }else if(action_id == DD_CMD_TYPE_DEL_DEV){
//          int tmpIdx;
//          bool isMatch;
//          uint8_t tmpVec[6] = {tmps.data[0],tmps.data[1],tmps.data[2],tmps.data[3],tmps.data[4],tmps.data[5]};
//          isMatch = check_addr_vecs(&tagAddrs,tmpVec,&tmpIdx);
//          if(isMatch){
//               bool dum = false;
//               vec_splice(&tagAddrs, tmpIdx, 1);
//               vec_compact(&tagAddrs);
//          }
//     }
//
//     // printf ("%s\t", cmds.data[0]);
//     // printf ("%s\t", cmds.data[1]);
//     // printf ("%s", cmds.data[2]);
//     // printf ("\r\n");
//
//     // print_vec_str(&tagNames);
//     // print_vec_bytes(&tagAddrs);
//
//     // vec_deinit(&strings);
//     // vec_deinit(&tmps);
//     vec_deinit(&cmds);
//     return NRF_SUCCESS;
//}


static bool pet_proximity_check(ble_evt_t const * p_ble_evt){
     int dump;
     ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
     const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
     if(flag_debug) NRF_LOG_INFO("pet_proximity_check: --------- \r\n");

     ble_report = &p_gap_evt->params.adv_report;
     ble_addr = (uint8_t*)ble_report->peer_addr.addr;
     int tmpRssi = ble_report->rssi;;

     bool isMatch = check_tag_addrs(&m_tags,ble_addr,&dump);
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
static void on_ble_evt(ble_evt_t * p_ble_evt){//, ble_nus_t * p_nus){
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
               // printf("BLE_GAP_EVT_CONNECTED\r\n");
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
               // printf("BLE_GAP_EVT_SEC_PARAMS_REQUEST\r\n");
               err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
               APP_ERROR_CHECK(err_code);
          } break;

          case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:{
               // printf("BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST\r\n");
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
               // printf("BLE_EVT_USER_MEM_REQUEST\r\n");
               err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
               APP_ERROR_CHECK(err_code);
          } break;

          case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:{
               // printf("BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST\r\n");
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

          case BLE_GATTS_EVT_WRITE:{
               // printf("BLE_GATTS_EVT_WRITE\r\n");
          } break;
          case BLE_GATTS_EVT_HVC:{
               // printf("BLE_GATTS_EVT_HVC\r\n");
          } break;
          case BLE_GATTS_EVT_HVN_TX_COMPLETE:{
               // printf("BLE_GATTS_EVT_HVN_TX_COMPLETE\r\n");
          } break;
     }
     nBleUpdates++;
}

#endif // DD_CMD_RELAY_H__
