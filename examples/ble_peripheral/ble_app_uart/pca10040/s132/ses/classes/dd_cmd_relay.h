#ifndef DD_MSG_RELAY_H__
#define DD_MSG_RELAY_H__

#include <string.h>
#include <stdlib.h>

#include "ble_nus.h"
#include "nrf_log.h"
#include "vector_c.h"
#include "nrf_lights.h"
#include "nrf_calender.h"
#include "nrf_drv_gpiote.h"
#include "nrf_battery_monitor.h"

#define DEFAULT_RSSI_THRESHOLD          -70
#define DEFAULT_DEBOUNCE_THRESHOLD       150

//#define DEBUG_CHECKING
//#define DEBUG_COMMANDS
//#define DEBUG_DD_MSGS

typedef enum{
     DD_MSG_TYPE_NONE,
     DD_MSG_TYPE_ADD_DEV,
     DD_MSG_TYPE_DEL_DEV,
     DD_MSG_TYPE_QUERY,
     DD_MSG_TYPE_GET,
     DD_MSG_TYPE_SET,
     DD_MSG_TYPE_REPORT,
}dd_msg_type_t;

typedef enum{
     DD_MSG_DATA_NONE,                       /** Id = 0 */
     DD_MSG_DATA_BATTERY_LEVEL,              /** Id = 1 (R) */
     DD_MSG_DATA_CHARGE_STATUS,              /** Id = 2 (R) */
     DD_MSG_DATA_BATTERY_INFO,               /** Id = 3 (R) */
     DD_MSG_DATA_BLE_UPDATE_RATE,            /** Id = 4 (R/W) */
     DD_MSG_DATA_DOOR_UPDATE_RATE,           /** Id = 5 (R/W) */
     DD_MSG_DATA_TAG_CHECK_UPDATE_RATE,      /** Id = 6 (R/W) */
     DD_MSG_DATA_NORMAL_DOOR_OPERATION,      /** Id = 7 (W) */
     DD_MSG_DATA_STOP_MOTOR,                 /** Id = 8 (W) */
     DD_MSG_DATA_OPEN_DOOR,                  /** Id = 8 (W) */
     DD_MSG_DATA_CLOSE_DOOR,                 /** Id = 9 (W) */
     DD_MSG_DATA_TOGGLE_DOOR_LOCK,           /** Id = 10 (W) */
     DD_MSG_DATA_LOCK_DOOR,                  /** Id = 11 (W) */
     DD_MSG_DATA_UNLOCK_DOOR,                /** Id = 12 (W) */
     DD_MSG_DATA_DOOR_ENCODER_LIMIT,         /** Id = 13 (R/W) */
     DD_MSG_DATA_DOOR_SPEED,                 /** Id = 14 (R/W) */
     DD_MSG_DATA_DOOR_STATUS,                /** Id = 15 (R) */
     DD_MSG_DATA_LIMIT_SWITCH,               /** Id = 16 (R) */
     DD_MSG_DATA_LOCK_STATUS,                /** Id = 17 (W) */
     DD_MSG_DATA_OPEN_STATUS,                /** Id = 18 (W) */
     DD_MSG_DATA_UPPER_SWITCH_STATUS,        /** Id = 19 (W) */
     DD_MSG_DATA_LOWER_SWITCH_STATUS,        /** Id = 20 (W) */
     DD_MSG_DATA_TAG_ID,                     /** Id = 21 (R) */
     DD_MSG_DATA_TAG_INFO,                   /** Id = 22 (R) */
     DD_MSG_DATA_TAG_ALIAS,                  /** Id = 23 (R/W) */
     DD_MSG_DATA_TAG_ADDRESS,                /** Id = 24 (R) */
     DD_MSG_DATA_TAG_RSSI,                   /** Id = 25 (R) */
     DD_MSG_DATA_TAG_RSSI_THRESHOLD,         /** Id = 26 (R/W) */
     DD_MSG_DATA_TAG_DEBOUNCE_THRESHOLD,     /** Id = 27 (R/W) */
     DD_MSG_DATA_TAG_TIMEOUT,                /** Id = 28 (R/W) */
     DD_MSG_DATA_TAG_LAST_ACTIVITY,          /** Id = 29 (R) */
     DD_MSG_DATA_TAG_STATISTICS,             /** Id = 30 (R) */
}dd_msg_data_t;

typedef struct{
     dd_msg_type_t type;
     dd_msg_data_t data;
}dd_msg_t;

typedef struct{
     char name[128];
     uint8_t addr[6];
     char alias[128];
     int thresh;
     int rssi;
     int timeout;
     int times_seen;
}dd_tag_t;

typedef vec_t(const char*) vec_string_t;
typedef vec_t(uint8_t) vec_byte_t;
typedef vec_t(vec_byte_t) vec_bytes_t;
typedef vec_t(bool) vec_bool_t;
typedef vec_t(dd_tag_t) vec_tags_t;

static ble_gap_evt_adv_report_t const* ble_report;

static vec_bytes_t tagAddrs;
static vec_string_t tagNames;
static vec_tags_t m_tags;

static uint8_t* ble_addr;
static int nDevices = 0;

static int debounceCounter = 0;
static int debounceThresh = DEFAULT_DEBOUNCE_THRESHOLD;

static bool flag_debug = false;
static bool flag_debug_rssi = false;
static bool tags_detected = false;

static bool isDoorOpen = false;
static bool flag_close_door = true;
static bool flag_force_door_open = false;
static bool flag_force_door_close = false;
static bool flag_upper_limit_switch_activated = false;
static bool flag_lower_limit_switch_activated = false;
static bool flag_door_locked = false;
static bool flag_force_door_stop = false;

static float motor_speed = 0.5;
static int32_t encoder_limit = 2945;

static float dt;
static time_t begin;
static time_t end;
static time_t now;
static time_t last_time;
static uint32_t myTimeStamp;
static struct tm* tmpT;

static struct dd_msg_data_entry{
    char *str;
    dd_msg_data_t n;
}dd_msg_data_entry;

/* For mapping command string to command id */
static struct dd_msg_data_entry dd_msg_data_dict[] = {
    "battery_level", DD_MSG_DATA_BATTERY_LEVEL,
    "charge_status", DD_MSG_DATA_CHARGE_STATUS,
    "battery", DD_MSG_DATA_BATTERY_INFO,
    "ble_rate", DD_MSG_DATA_BLE_UPDATE_RATE,
    "door_update_rate", DD_MSG_DATA_DOOR_UPDATE_RATE,
    "tag_check_rate", DD_MSG_DATA_TAG_CHECK_UPDATE_RATE,
    "normal_door_operation", DD_MSG_DATA_NORMAL_DOOR_OPERATION,
    "stop_motor", DD_MSG_DATA_STOP_MOTOR,
    "open_door", DD_MSG_DATA_OPEN_DOOR,
    "close_door", DD_MSG_DATA_CLOSE_DOOR,
    "toggle_door_lock", DD_MSG_DATA_TOGGLE_DOOR_LOCK,
    "lock_door", DD_MSG_DATA_LOCK_DOOR,
    "unlock_door", DD_MSG_DATA_UNLOCK_DOOR,
    "encoder_limit", DD_MSG_DATA_DOOR_ENCODER_LIMIT,
    "limit_switch", DD_MSG_DATA_LIMIT_SWITCH,
    "door_speed", DD_MSG_DATA_DOOR_SPEED,
    "door_status", DD_MSG_DATA_DOOR_STATUS,
    "open_status", DD_MSG_DATA_OPEN_STATUS,
    "lock_status", DD_MSG_DATA_LOCK_STATUS,
    "upper_switch", DD_MSG_DATA_UPPER_SWITCH_STATUS,
    "lower_switch", DD_MSG_DATA_LOWER_SWITCH_STATUS,
    "tag_info", DD_MSG_DATA_TAG_INFO,
    "tag_id", DD_MSG_DATA_TAG_ID,
    "tag_alias", DD_MSG_DATA_TAG_ALIAS,
    "tag_addr", DD_MSG_DATA_TAG_ADDRESS,
    "tag_rssi", DD_MSG_DATA_TAG_RSSI,
    "rssi", DD_MSG_DATA_TAG_RSSI,
    "tag_rssi_thresh", DD_MSG_DATA_TAG_RSSI_THRESHOLD,
    "rssi_thresh", DD_MSG_DATA_TAG_RSSI_THRESHOLD,
    "tag_debounce_thresh", DD_MSG_DATA_TAG_DEBOUNCE_THRESHOLD,
    "tag_timeout", DD_MSG_DATA_TAG_TIMEOUT,
    "tag_stats", DD_MSG_DATA_TAG_STATISTICS,
    "tag_last_act", DD_MSG_DATA_TAG_LAST_ACTIVITY,
};

void print_addr(const uint8_t* addr);
void print_vec_str(const vec_string_t * strings);
void print_vec_byte(const vec_byte_t * bytes);
void print_vec_bytes(const vec_bytes_t * bytes);

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
//          vec_push(&matches, isMatch);
          if(isMatch){
               *foundIdx = i;
               return true;
          }
#ifdef DEBUG_CHECKING
          char* descMatch = (isMatch ? "True":"False");
          printf("\tChecking address [");print_addr(target); printf("] - isMatch = %s\r\n",descMatch);
#endif
     }
//     vec_find(&matches, true, idx);
//     *foundIdx = idx;
     if(!isMatch){
          *foundIdx = -1;
          return false;
     }
     if(idx == -1){
          flag_out = false;
     }else{
          flag_out = true;
#ifdef DEBUG_CHECKING
          printf("\check_tag_addrs - Found match at %d\n",idx);
#endif
     }
//     vec_deinit(&matches);
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

static uint32_t parse_nus_message(const uint8_t * p_data, vec_string_t* outputs){
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
          printf("parse_nus_message: --- cmd [%d] = %s\r\n",n,ch);
     }
#endif
     return (uint32_t) i;
}

static int isValidMessage(const char* cmd){
     int code;

     if(strcmp(cmd,"add_dev") == 0) code = DD_MSG_TYPE_ADD_DEV;
     else if(strcmp(cmd,"del_dev") == 0) code = DD_MSG_TYPE_DEL_DEV;
     else if(strcmp(cmd,"query") == 0) code = DD_MSG_TYPE_QUERY;
     else if(strcmp(cmd,"get") == 0) code = DD_MSG_TYPE_GET;
     else if(strcmp(cmd,"set") == 0) code = DD_MSG_TYPE_SET;
     else if(strcmp(cmd,"report") == 0) code = DD_MSG_TYPE_REPORT;
     else code = -1;
     return code;
}

static void report_tags_data(ble_nus_t* p_nus, const vec_tags_t* tags_, uint16_t length){
     uint8_t data[4096];
     for(int i = 0; i < length; i++){
          memset(data, 0, sizeof(data));
          dd_tag_t tmpTag = tags_->data[i];
          size_t sz = snprintf(NULL, 0, "report add_tag %s %02x:%02x:%02x:%02x:%02x:%02x\n",\
                                        tmpTag.name,tmpTag.addr[0],tmpTag.addr[1],tmpTag.addr[2],tmpTag.addr[3],tmpTag.addr[4],tmpTag.addr[5]);
          sprintf((char *)data, "report add_tag %s %02x:%02x:%02x:%02x:%02x:%02x\n",\
                                        tmpTag.name,tmpTag.addr[0],tmpTag.addr[1],tmpTag.addr[2],tmpTag.addr[3],tmpTag.addr[4],tmpTag.addr[5]);
          ble_nus_string_send(p_nus, data,sz);
     }
}

static void report_limit_switch_state(ble_nus_t* p_nus, const char* switch_id, bool state){
     uint8_t data[4096];
     size_t sz = 0;
     memset(data, 0, sizeof(data));
     sz = snprintf(NULL, 0, "report limit_switch %s_switch %s\n",switch_id,state ? "true" : "false");
     sprintf((char *)data, "report limit_switch %s_switch %s\n",switch_id,state ? "true" : "false");
     ble_nus_string_send(p_nus, data,sz);
}

// TODO
// static void report_battery_update(ble_nus_t* p_nus, const int , uint16_t length){
//      uint8_t data[4096];
//      memset(data, 0, sizeof(data));
// }

static void report_open_status(ble_nus_t* p_nus, bool isOpen){
     uint8_t data[4096];
     size_t sz = 0;
     memset(data, 0, sizeof(data));
     sz = snprintf(NULL, 0, "report open_status %s\n",isOpen ? "true" : "false");
     sprintf((char *)data, "report open_status %s\n",isOpen ? "true" : "false");
     ble_nus_string_send(p_nus, data,sz);
}
static void report_lock_status(ble_nus_t* p_nus, bool isLocked){
     uint8_t data[4096];
     size_t sz = 0;
     memset(data, 0, sizeof(data));
     sz = snprintf(NULL, 0, "report lock_status %s\n",isLocked ? "true" : "false");
     sprintf((char *)data, "report lock_status %s\n",isLocked ? "true" : "false");
     ble_nus_string_send(p_nus, data,sz);
}

static uint32_t send_stored_info(ble_nus_t* p_nus, const vec_tags_t* tags_, uint16_t length){
     uint8_t data[4096];
     int num_tags = tags_->length;
     if(num_tags <= 0){
          char* buff = "No Devices Stored\r\n";
          sprintf((char *)data, buff);
          ble_nus_string_send(p_nus, data, strlen(buff));
     }else{
          // printf("Currently have %d tags stored.\r\n",num_tags);
          report_tags_data(p_nus, tags_, length);
     }
     return NRF_SUCCESS;
}

static int compare_msg_data_entries(const void *s1, const void *s2){
     const struct dd_msg_data_entry *e1 = s1;
     const struct dd_msg_data_entry *e2 = s2;
     return strcmp(e1->str, e2->str);
}

static dd_msg_data_t interpret_msg_data_type(ble_nus_t* p_nus, const char* cmd_){
     char* input = (char*) cmd_;
     size_t nObjs = sizeof(dd_msg_data_dict)/sizeof(dd_msg_data_dict[0]);
     // printf("interpret_msg_data_type: Searching for input [%s] among %d known commands...\r\n",input,nObjs);
     qsort(dd_msg_data_dict, nObjs, sizeof(struct dd_msg_data_entry), compare_msg_data_entries);
     struct dd_msg_data_entry *result, key = {input};
     result = bsearch(&key, dd_msg_data_dict, nObjs, sizeof(struct dd_msg_data_entry), compare_msg_data_entries);

     // printf("%s - %d", result->str, result->n);
     return (dd_msg_data_t) result->n;
}

static uint32_t dd_msg_handler(dd_msg_data_t cmd_id_, int target_tag_index, bool isSetOp, const char* raw_str_value, ble_nus_t * p_nus){
     // Ensure a valid tag id is given when tag-specific commands are recognized
     if((cmd_id_ >= DD_MSG_DATA_TAG_ID) && (target_tag_index < 0)){
          // printf("[ERROR] parse_nus_data() ---- The DoggyTag ID given [%d] is invalid, or not recognized.\n",target_tag_index);
          return NRF_ERROR_INVALID_PARAM;
     }

     size_t sz = 0;
     uint8_t buffer[4096];
     // TODO: Verify proper manipulation of parameter and storage (i.e. pointers)
     dd_tag_t* targetTag = &m_tags.data[target_tag_index];

     switch(cmd_id_){
          case DD_MSG_DATA_NONE:{
               // printf("DD_MSG_DATA_NONE\n");
               return NRF_SUCCESS;
          } break;

          /** ***************       WRITE ONLY  COMMANDS     **************** */
          case DD_MSG_DATA_STOP_MOTOR:{
               // printf("DD_MSG_DATA_STOP_MOTOR\n");
               flag_force_door_open = false;
               flag_force_door_close = false;
               flag_force_door_stop = !flag_force_door_stop;
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_OPEN_DOOR:{
               // printf("DD_MSG_DATA_OPEN_DOOR\n");
               flag_force_door_open = !flag_force_door_open;
               flag_force_door_close = false;
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_CLOSE_DOOR:{
               // printf("DD_MSG_DATA_CLOSE_DOOR\n");
               flag_force_door_open = false;
               flag_force_door_close = !flag_force_door_close;
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_LOCK_DOOR:{
               // printf("DD_MSG_DATA_LOCK_DOOR\n");
               flag_door_locked = true;
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_UNLOCK_DOOR:{
               // printf("DD_MSG_DATA_UNLOCK_DOOR\n");
               flag_door_locked = false;
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_NORMAL_DOOR_OPERATION:{
               flag_force_door_open = false;
               flag_force_door_close = false;
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TOGGLE_DOOR_LOCK:{
               // printf("DD_MSG_DATA_TOGGLE_DOOR_LOCK\n");
               return NRF_SUCCESS;
          } break;

          /** **********   Tag-Independent READ ONLY  COMMANDS   ************ */
          case DD_MSG_DATA_BATTERY_LEVEL:{
               // printf("[DoggyDoor] -- Current Battery Voltage (mV): %d (%d%%)\n",voltage,bat_percent);
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report battery_level %d %d\n",voltage,bat_percent);
               sprintf((char *)buffer, "report battery_level %d %d\n",voltage,bat_percent);
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_CHARGE_STATUS:{
               // printf("[DoggyDoor] -- Battery Charge Status: \n");
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report charge_status %d\n",dd_bat_status);
               sprintf((char *)buffer, "report charge_status %d\n",dd_bat_status);
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_BATTERY_INFO:{
               // printf("[DoggyDoor] -- Current Battery Info (mV): %d (%d%%)\n",voltage,bat_percent);
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report battery %d %d %d\n",voltage,bat_percent,dd_bat_status);
               sprintf((char *)buffer, "report battery %d %d %d\n",voltage,bat_percent,dd_bat_status);
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_DOOR_STATUS:{
               // printf("DD_MSG_DATA_DOOR_STATUS\n");
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report door_status %s %s\n",isDoorOpen ? "true" : "false",flag_door_locked ? "true" : "false");
               sprintf((char *)buffer, "report door_status %s %s\n",isDoorOpen ? "true" : "false",flag_door_locked ? "true" : "false");
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_LIMIT_SWITCH:{
               // printf("Specified Pet Tag's stored RSSI threshold is %d.\n",targetTag->thresh);
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report limit_switch %s %s\n",flag_lower_limit_switch_activated ? "true" : "false",flag_upper_limit_switch_activated ? "true" : "false");
               sprintf((char *)buffer, "report limit_switch %s %s\n",flag_lower_limit_switch_activated ? "true" : "false",flag_upper_limit_switch_activated ? "true" : "false");
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          // TODO:
         case DD_MSG_DATA_OPEN_STATUS:{
              // printf("DD_MSG_DATA_OPEN_STATUS\n");
              report_open_status(p_nus,isDoorOpen);
              return NRF_SUCCESS;
         } break;
         case DD_MSG_DATA_LOCK_STATUS:{
              // printf("DD_MSG_DATA_LOCK_STATUS\n");
              report_lock_status(p_nus,flag_door_locked);
              return NRF_SUCCESS;
         } break;


          /** **********   Tag-Independent R/W  COMMANDS   ************ */
          case DD_MSG_DATA_BLE_UPDATE_RATE:{                               // TODO
               // printf("DD_MSG_DATA_BLE_UPDATE_RATE\n");
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_DOOR_UPDATE_RATE:{                              // TODO
               // printf("DD_MSG_DATA_DOOR_UPDATE_RATE\n");
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TAG_CHECK_UPDATE_RATE:{                         // TODO
               // printf("DD_MSG_DATA_TAG_CHECK_UPDATE_RATE\n");
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_DOOR_ENCODER_LIMIT:{                            // TODO
               if(isSetOp){
                    // printf("Modified specified pet tag's stored RSSI threshold from %d to %d.\n",prevVal,newVal);
                    int32_t prevVal = encoder_limit;
                    int32_t newVal = (int32_t)strtol(raw_str_value, NULL, 10);
                    encoder_limit = newVal;
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "Modified DoggyDoor's encoder limit from %d to %d.\n",prevVal,newVal);
                    sprintf((char *)buffer, "Modified DoggyDoor's encoder limit from %d to %d.\n",prevVal,newVal);
                    ble_nus_string_send(p_nus, buffer,sz);
               }else{
                    // printf("Specified Pet Tag's stored RSSI threshold is %d.\n",targetTag->thresh);
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "report encoder_limit %d\n",encoder_limit);
                    sprintf((char *)buffer, "report encoder_limit %d\n",encoder_limit);
                    ble_nus_string_send(p_nus, buffer,sz);
               }
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_DOOR_SPEED:{                                    // TODO
               if(isSetOp){
                    // printf("Modified specified pet tag's stored RSSI threshold from %d to %d.\n",prevVal,newVal);
                    float prevVal = motor_speed;
                    float newVal = (float)strtof(raw_str_value, NULL);
                    motor_speed = newVal;
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "Modified DoggyDoor's motor speed from %.2f to %.2f.\n",prevVal,newVal);
                    sprintf((char *)buffer, "Modified DoggyDoor's motor speed from %.2f to %.2f.\n",prevVal,newVal);
                    ble_nus_string_send(p_nus, buffer,sz);
               }else{
                    // printf("Specified Pet Tag's stored RSSI threshold is %d.\n",targetTag->thresh);
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "report door_speed %.4f\n", motor_speed);
                    sprintf((char *)buffer, "report door_speed %.4f\n", motor_speed);
                    ble_nus_string_send(p_nus, buffer,sz);
               }
               return NRF_SUCCESS;
          } break;

          /** **********   Tag-Dependent READ Only  COMMANDS   ************ */
          case DD_MSG_DATA_TAG_INFO:{
               // printf("Specified Pet Tag's Internally stored ID is \'%s\'\n",targetTag->name);
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report tag_info %s %02x:%02x:%02x:%02x:%02x:%02x %s %d %d %d %d \n",\
                                        targetTag->name,targetTag->addr[0],targetTag->addr[1],targetTag->addr[2],targetTag->addr[3],targetTag->addr[4],targetTag->addr[5],\
                                        targetTag->alias,targetTag->rssi,targetTag->thresh,targetTag->timeout,targetTag->times_seen);
               sprintf((char *)buffer, "report tag_info %s %02x:%02x:%02x:%02x:%02x:%02x %s %d %d %d %d \n",\
                                        targetTag->name,targetTag->addr[0],targetTag->addr[1],targetTag->addr[2],targetTag->addr[3],targetTag->addr[4],targetTag->addr[5],\
                                        targetTag->alias,targetTag->rssi,targetTag->thresh,targetTag->timeout,targetTag->times_seen);
#ifdef DEBUG_DD_MSGS
               printf("[DoggyDoor Message] Sending NUS Message w/ %d characters to client ----\r\n\treport tag_info %s %02x:%02x:%02x:%02x:%02x:%02x %s %d %d %d %d \n",\
                                        sz,targetTag->name,targetTag->addr[0],targetTag->addr[1],targetTag->addr[2],targetTag->addr[3],targetTag->addr[4],targetTag->addr[5],\
                                        targetTag->alias,targetTag->rssi,targetTag->thresh,targetTag->timeout,targetTag->times_seen);
#endif
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TAG_ID:{
               // printf("Specified Pet Tag's Internally stored ID is \'%s\'\n",targetTag->name);
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report tag_id %s\n",targetTag->name);
               sprintf((char *)buffer, "report tag_id %s\n",targetTag->name);
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TAG_ADDRESS:{
               // printf("Specified Pet Tag's Address is stored as %02x:%02x:%02x:%02x:%02x:%02x\n",targetTag->addr[0],targetTag->addr[1],targetTag->addr[2],targetTag->addr[3],targetTag->addr[4],targetTag->addr[5]);
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report tag_addr %02x:%02x:%02x:%02x:%02x:%02x\n",targetTag->addr[0],targetTag->addr[1],targetTag->addr[2],targetTag->addr[3],targetTag->addr[4],targetTag->addr[5]);
               sprintf((char *)buffer, "report tag_addr %02x:%02x:%02x:%02x:%02x:%02x\n",targetTag->addr[0],targetTag->addr[1],targetTag->addr[2],targetTag->addr[3],targetTag->addr[4],targetTag->addr[5]);
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TAG_RSSI:{
               // printf("Specified Pet Tag's detected RSSI value is %d.\n",targetTag->rssi);
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report tag_rssi %d\n",targetTag->rssi);
               sprintf((char *)buffer, "report tag_rssi %d\n",targetTag->rssi);
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;

          /** **********   Tag-Dependent R/W  COMMANDS   ************ */
          case DD_MSG_DATA_TAG_ALIAS:{
               if(isSetOp){
                    // printf("Modified specified pet tag's stored alias from \'%s\' to \'%s\'.\n",prevVal,targetTag->alia);
                    char prevVal[128];                 memset(prevVal, '\0', sizeof(prevVal));
                    strcpy(prevVal, targetTag->alias);
                    memset(&targetTag->alias, '\0', 128);
                    strcpy(&targetTag->alias[0], raw_str_value);
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "Modified specified pet tag's stored alias from \'%s\' to \'%s\'.\n",prevVal,targetTag->alias);
                    sprintf((char *)buffer, "Modified specified pet tag's stored alias from \'%s\' to \'%s\'.\n",prevVal,targetTag->alias);
                    ble_nus_string_send(p_nus, buffer,sz);
               }else{
                    // printf("Specified Pet Tag's stored alias is %s.\n",targetTag->alias);
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "report tag_alias %s\n",targetTag->alias);
                    sprintf((char *)buffer, "report tag_alias %s\n",targetTag->alias);
                    ble_nus_string_send(p_nus, buffer,sz);
               }
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TAG_RSSI_THRESHOLD:{
               if(isSetOp){
                    // printf("Modified specified pet tag's stored RSSI threshold from %d to %d.\n",prevVal,newVal);
                    int prevVal = targetTag->thresh;
                    int32_t newVal = (int32_t)strtol(raw_str_value, NULL, 10);
                    targetTag->thresh = newVal;
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "Modified specified pet tag's stored RSSI threshold from %d to %d.\n",prevVal,newVal);
                    sprintf((char *)buffer, "Modified specified pet tag's stored RSSI threshold from %d to %d.\n",prevVal,newVal);
                    ble_nus_string_send(p_nus, buffer,sz);
               }else{
                    // printf("Specified Pet Tag's stored RSSI threshold is %d.\n",targetTag->thresh);
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "report tag_rssi_thresh %d\n",targetTag->thresh);
                    sprintf((char *)buffer, "report tag_rssi_thresh %d\n",targetTag->thresh);
                    ble_nus_string_send(p_nus, buffer,sz);
               }
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TAG_DEBOUNCE_THRESHOLD:{
               if(isSetOp){
                    // printf("Modified specified pet tag's stored debounce threshold from %d to %d.\n",prevVal,newVal);
                    int prevVal = debounceThresh;
                    // int prevVal = targetTag->timeout; /** TODO: Figure out how to implement tag-specific timeouts */
                    int32_t newVal = (int32_t)strtol(raw_str_value, NULL, 10);
                    debounceThresh = newVal;
                    // targetTag->timeout = newVal; /** TODO: Figure out how to implement tag-specific timeouts */
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "Modified specified pet tag's stored debounce threshold from %d to %d.\n",prevVal,newVal);
                    sprintf((char *)buffer, "Modified specified pet tag's stored debounce threshold from %d to %d.\n",prevVal,newVal);
                    ble_nus_string_send(p_nus, buffer,sz);
               }else{
                    // printf("Specified Pet Tag's stored debounce threshold is %d.\n",targetTag->timeout);
                    int curVal = debounceThresh;
                    // int curVal =  targetTag->timeout; /** TODO: Figure out how to implement tag-specific timeouts */
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "report tag_debounce_thresh %d\n",curVal);
                    sprintf((char *)buffer, "report tag_debounce_thresh %d\n",curVal);
                    ble_nus_string_send(p_nus, buffer,sz);
               }
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TAG_TIMEOUT:{
               if(isSetOp){
                    // printf("TODO: Sorry this feature has not been implemented yet. For now use the debounce threshold.\n");
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "TODO: Sorry this feature has not been implemented yet. For now use the debounce threshold.\n");
                    sprintf((char *)buffer, "TODO: Sorry this feature has not been implemented yet. For now use the debounce threshold.\n");
                    ble_nus_string_send(p_nus, buffer,sz);
               }else{
                    // printf("TODO: Sorry this feature has not been implemented yet. For now use the debounce threshold, which is set to %d.\n",targetTag->timeout);
                    memset(buffer, 0, sizeof(buffer));
                    sz = snprintf(NULL, 0, "TODO: Sorry this feature has not been implemented yet. For now use the debounce threshold, which is set to %d.\n",targetTag->timeout);
                    sprintf((char *)buffer, "TODO: Sorry this feature has not been implemented yet. For now use the debounce threshold, which is set to %d.\n",targetTag->timeout);
                    ble_nus_string_send(p_nus, buffer,sz);
               }
               return NRF_SUCCESS;
          } break;
          /** ******** TODO: Commands that need to be implemented ********** */
          case DD_MSG_DATA_TAG_LAST_ACTIVITY:{
               // printf("TODO: Not Yet Implemented. Sorry\n");
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "TODO: Not Yet Implemented. Sorry\n");
               sprintf((char *)buffer, "TODO: Not Yet Implemented. Sorry\n");
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
          case DD_MSG_DATA_TAG_STATISTICS:{
               // printf("TODO: Not Yet Implemented. Sorry\n");
               memset(buffer, 0, sizeof(buffer));
               sz = snprintf(NULL, 0, "report tag_stats %d\n",targetTag->times_seen);
               sprintf((char *)buffer, "report tag_stats %d\n",targetTag->times_seen);
               ble_nus_string_send(p_nus, buffer,sz);
               return NRF_SUCCESS;
          } break;
     }

     return NRF_SUCCESS;
}

static uint32_t parse_nus_data(ble_nus_t * p_nus, const uint8_t * p_data){
     int testIdx;
     int targetIdx = -2;

     bool isMatch;
     bool isValid = false;
     bool flag_stop = false;
     dd_msg_data_t cmd_id = DD_MSG_DATA_NONE;

     char tmpName[4096];
     memset(tmpName, '\0', sizeof(tmpName));
     uint8_t tmpAddr[6];
     memset(tmpAddr, 0, sizeof(tmpAddr));

     vec_string_t cmds;       vec_init(&cmds);
     uint32_t nCmds = parse_nus_message(p_data,&cmds);
     int cmd_type = isValidMessage(cmds.data[0]);

     if(cmd_type < 0){
          vec_deinit(&cmds);
          return NRF_ERROR_INVALID_DATA;
     }else if(cmd_type == DD_MSG_TYPE_QUERY){
          send_stored_info(p_nus, &m_tags, nDevices);
          vec_deinit(&cmds);
          return NRF_SUCCESS;
     }else if( (cmd_type == DD_MSG_TYPE_ADD_DEV) || (cmd_type == DD_MSG_TYPE_DEL_DEV)){
          strcpy(tmpName, cmds.data[1]);
          // printf("Provided Info for Tag: %s [",tmpName);
          for(int i = 0; i < 6; i++){
               uint16_t num = (uint16_t)strtol(cmds.data[i + 2], NULL, 16);
               tmpAddr[i] = num;
               // printf("%02x:",tmpAddr[i]);
          }
          // printf("\b]\n");
     }else if((cmd_type == DD_MSG_TYPE_GET) || (cmd_type == DD_MSG_TYPE_SET)){
          int tmpIdx, tmpIdx2;
          cmd_id = interpret_msg_data_type(p_nus,cmds.data[1]);
          if(cmd_id >= DD_MSG_DATA_TAG_ID){
               // printf("Handling Pet Tag specific data. Need to know Specific pet's tag id...\n");
               if(check_tag_names(&m_tags,cmds.data[2],&tmpIdx)) targetIdx = tmpIdx;
               else if(check_tag_addrs(&m_tags,cmds.data[2],&tmpIdx2)) targetIdx = tmpIdx2;
               else{
                    // printf("[ERROR] parse_nus_data() ---- \'%s\' The DoggyTag ID given [%s] is invalid, or not recognized.\n",cmds.data[1],cmds.data[2]);
                    targetIdx = -1;
                    vec_deinit(&cmds);
                    return NRF_ERROR_INVALID_PARAM;
               }
          }// else printf("Handling Doogy Door Controller (DDC) specific data...\n");
     }

     // TODO: Implement dd_msg_handler() function
     if(cmd_type == DD_MSG_TYPE_GET){
          dd_msg_handler(cmd_id, targetIdx, false, "NULL", p_nus);
     }else if(cmd_type == DD_MSG_TYPE_SET){
          dd_msg_handler(cmd_id, targetIdx, true, cmds.data[3], p_nus);
     }

     if(cmd_type == DD_MSG_TYPE_ADD_DEV){
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
          tmpTag.timeout = DEFAULT_DEBOUNCE_THRESHOLD;
          strcpy(&tmpTag.alias[0], tmpName);
          vec_insert(&m_tags,0,tmpTag);
          nDevices = m_tags.length;
     }else if(cmd_type == DD_MSG_TYPE_DEL_DEV){
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
          dd_tag_t* foundTag = &m_tags.data[dump];
          foundTag->rssi = tmpRssi;
          foundTag->times_seen++;
          if(flag_debug_rssi) printf(NRF_LOG_COLOR_CODE_GREEN" Found Target Device! =====  %d\r\n"NRF_LOG_COLOR_CODE_DEFAULT,tmpRssi);
          tags_detected = true;
          if(tmpRssi >= foundTag->thresh){
               return true;
          }
     }
     return false;
}

#endif // DD_MSG_RELAY_H__
