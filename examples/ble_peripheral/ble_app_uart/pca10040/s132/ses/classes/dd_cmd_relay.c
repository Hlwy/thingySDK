#include "dd_cmd_relay.h"

void print_addr(uint8_t* addr){
     for(uint8_t i = 0; i < 6; i++){ printf("%02x ", addr[i]); }
}

void print_vec_str(vec_string_t * strings){
     size_t i; const char * string;
     vec_foreach(strings, string, i){ printf("Names[%zu] = \"%s\"\n", i, string); }
}

void print_vec_byte(vec_byte_t * bytes){
     size_t i; uint8_t byte;
     printf("Bytes[%zu] = ", i);
     vec_foreach(bytes, byte, i) {printf("%02x ", byte);}
}

void print_vec_bytes(vec_bytes_t * bytes){
     size_t i; size_t j;
     uint8_t byte;
     vec_byte_t byte2;
     vec_foreach(bytes, byte2, i){
          printf("Bytes[%zu] = ", i);
          vec_foreach(&byte2, byte, j){printf("%02x ", byte);}
     }
     printf("\r\n");
}


// bool check_addr_vec(vec_byte_t* addr, uint8_t* target){
//      char* tmpMatch;
//      uint8_t byte; size_t i;
//      bool isMatch = false;
//      if(memcmp(target, addr->data,6)== 0) isMatch = true;
//      else isMatch = false;
//      if(!isMatch){
//           // printf("check_addr_vec: checking address in reverse...\r\n");
//           int j = 0;
//           vec_foreach_rev(addr, byte, i){
//                if(i>=6){
//                     // printf("check_addr_vec: Forcing break...\r\n");
//                     break;
//                }
//                if(byte == target[j]){isMatch = true;tmpMatch = "true";
//                }else{isMatch = false;tmpMatch = "false";}
//                // printf("\tByte[%d] - In [%02x] -- Target [%02x] --- Match = %s\n",i,byte,target[j],tmpMatch);
//                j++;
//           }
//      }
//      return isMatch;
// }
//
// bool check_addr_vecs(vec_bytes_t* addrs_, uint8_t* target, int* foundIdx){
//      int idxT;
//      size_t i, idx;
//      bool isMatch = false;
//      bool flag_out;
//
//      vec_byte_t byte_vec;
//      vec_bool_t matches;
//      vec_init(&matches);
//
//      // printf("check_addr_vecs: Checking Known Addresses for Target - ");
//      // print_addr(target); printf("\r\n");
//      vec_foreach(addrs_,byte_vec,i){
//           isMatch = check_addr_vec(&byte_vec,target);
//           vec_push(&matches, isMatch);
//      }
//      vec_find(&matches, true, idxT);
//      *foundIdx = idxT;
//      if(idxT == -1) flag_out = false;
//      else flag_out = true;
//      vec_deinit(&matches);
//      return flag_out;
// }


// void send_vec_bytes(vec_bytes_t * bytes){
//      size_t i;
//      vec_byte_t byte_vec;
//      uint8_t data[128];
//
//      vec_foreach(bytes, byte_vec, i){
//           sprintf((char *)data, "Bytes[%zu] = %02x:%02x:%02x:%02x:%02x:%02x\r\n",i,byte_vec.data[0],byte_vec.data[1],byte_vec.data[2],byte_vec.data[3],byte_vec.data[4],byte_vec.data[5]);
//           // printf("sending string:\r\n\t%s",(char*)data);
//           ble_nus_string_send(&m_nus, data, 32);
//      }
// }
//
// void send_stored_info(){
//      char* buff;
//      uint8_t data[32];
//      if(nDevices == 0){
//           char* buff = "No Devices Stored\r\n";
//           sprintf((char *)data, buff);
//           ble_nus_string_send(&m_nus, data, strlen(buff));
//      }else{
//           send_vec_bytes(&tagAddrs);
//      }
// }
//
// void send_misc_info(uint8_t type){
//      uint8_t data[128];
//
//      if(type == DD_CMD_DATA_RSSI_THRESHOLD){
//           sprintf((char *)data, "RSSI Threshold = %d\r\n",rssiThresh);
//           ble_nus_string_send(&m_nus, data, 32);
//      }else if(type == DD_CMD_DATA_BATTERY_INFO){
//           sprintf((char *)data, "Battery Voltage (mV) = %d (%d%%)\r\n",voltage,bat_percent);
//           ble_nus_string_send(&m_nus, data, 34);
//      }else{
//           // send_vec_bytes(&tagAddrs);
//      }
// }


// uint32_t parse_nus_data(uint8_t * p_data){
//      int i = 0;
//      char * pch;
//      uint16_t num;
//
//      bool flag_stop = false;
//      char tmpStr[4096];
//
//      vec_string_t strings;
//      vec_string_t cmds;
//      vec_byte_t tmps;
//      vec_init(&strings);
//      vec_init(&cmds);
//      vec_init(&tmps);
//
//      // printf ("Splitting string \"%s\" into tokens:\n",(char*) p_data);
//      pch = strtok ((char*) p_data," :");
//      while(pch != NULL){
//           vec_push(&cmds, pch);
//           if(i == 0){
//                // printf ("%s", pch);
//                if(strcmp(pch,"add_dev") == 0){
//                     // printf("parse_nus_data: --- Adding device\r\n");
//                     action_id = DD_CMD_TYPE_ADD_DEV;
//                     nDevices++;
//                }else if(strcmp(pch,"del_dev") == 0){
//                     // printf("parse_nus_data: --- Removing device\r\n");
//                     action_id = DD_CMD_TYPE_DEL_DEV;
//                     nDevices--;
//                }else if(strcmp(pch,"query") == 0){
//                     send_stored_info();
//                }else if(strcmp(pch,"get") == 0){
//                     action_id = DD_CMD_TYPE_GET;
//                }else if(strcmp(pch,"set") == 0){
//                     action_id = DD_CMD_TYPE_SET;
//                }else{
//                     action_id = DD_CMD_TYPE_NONE;
//                     flag_stop = true;
//                }
//           }
//           if(i == 1){
//                if(action_id == DD_CMD_TYPE_ADD_DEV){
//                     memset(tmpStr, 0, sizeof(tmpStr));
//                     memcpy(&tmpStr[0], (char*)pch, sizeof(uint8_t)*(strlen(pch)));
//                     const char* tmpName = (const char*)tmpStr;
//                     vec_insert(&tagNames,0,tmpName);
//                }else if(action_id == DD_CMD_TYPE_GET){
//                     if(strcmp(pch,"rssi_thresh") == 0){
//                          send_misc_info(DD_CMD_DATA_RSSI_THRESHOLD);
//                     }else if(strcmp(pch,"battery") == 0){
//                          send_misc_info(DD_CMD_DATA_BATTERY_INFO);
//                     }
//                }
//           }
//           if(i>1){
//                if((action_id == DD_CMD_TYPE_ADD_DEV) || (action_id == DD_CMD_TYPE_DEL_DEV)){
//                     num = (uint16_t)strtol(pch, NULL, 16);
//                     vec_push(&tmps, num);
//                }else if(action_id == DD_CMD_TYPE_SET){
//                     int32_t val = (int32_t)strtol(pch, NULL, 10);
//                     rssiThresh = val;
//                     // printf("Setting Value to %d...",val);
//                }
//           }
//           pch = strtok (NULL, " :");
//           if(flag_stop)
//                break;
//           i++;
//      }
//      if(action_id == DD_CMD_TYPE_ADD_DEV){
//           vec_push(&tagAddrs,tmps);
//           // vec_insert(&tagAddrs,0,tmps);
//      }else if(action_id == DD_CMD_TYPE_DEL_DEV){
//           int tmpIdx;
//           bool isMatch;
//           uint8_t tmpVec[6] = {tmps.data[0],tmps.data[1],tmps.data[2],tmps.data[3],tmps.data[4],tmps.data[5]};
//           isMatch = check_addr_vecs(&tagAddrs,tmpVec,&tmpIdx);
//           if(isMatch){
//                bool dum = false;
//                vec_splice(&tagAddrs, tmpIdx, 1);
//           }
//      }
//
//      // printf ("%s\t", cmds.data[0]);
//      // printf ("%s\t", cmds.data[1]);
//      // printf ("%s", cmds.data[2]);
//      // printf ("\r\n");
//
//      // print_vec_str(&tagNames);
//      // print_vec_bytes(&tagAddrs);
//
//      vec_deinit(&strings);
//      vec_deinit(&tmps);
//      vec_deinit(&cmds);
//      return NRF_SUCCESS;
// }
//
//
// bool pet_proximity_check(ble_evt_t const * p_ble_evt){
//      int dump;
//      ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
//      const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
//      if(flag_debug) NRF_LOG_INFO("pet_proximity_check: --------- \r\n");
//
//      ble_report = &p_gap_evt->params.adv_report;
//      ble_addr = (uint8_t*)ble_report->peer_addr.addr;
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
