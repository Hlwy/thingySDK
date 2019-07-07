// #include "nrf_ble_main.h"


//static ble_gap_evt_adv_report_t const* temp_report;
//static char* tmpName;
//static uint8_t const * tmpAddr;
//static char const m_target_periph_name[] = "BlueCharm";
//static char const m_target_phone_name[] = "Samsung Galaxy S7";
//static uint8_t const target_mac[] = {0xb0,0x91,0x22,0xf7,0x6d,0x55};
//static uint8_t const target_mac_rvr[] = {0x55,0x6d,0xf7,0x22,0x91,0xb0};
//static bool tags_nearby = false;
//
//static ble_os_t m_our_service;


//typedef struct
//{
//    bool           is_connected;
//    ble_gap_addr_t address;
//} conn_peer_t;
//
///**@brief Connection parameters requested for connection.
// */
//static const ble_gap_conn_params_t m_connection_param =
//{
//    MIN_CONNECTION_INTERVAL,
//    MAX_CONNECTION_INTERVAL,
//    SLAVE_LATENCY,
//    SUPERVISION_TIMEOUT
//};
//
//static uint16_t           m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
//static nrf_ble_gatt_t     m_gatt;                                           /**< GATT module instance. */
//static ble_nus_t          m_nus;                                      /**< Structure to identify the Nordic UART Service. */
//static ble_nus_t          m_nus2;                                      /**< Structure to identify the Nordic UART Service. */
//
//static conn_peer_t        m_connected_peers[NRF_BLE_LINK_COUNT];
//
////For UART
//#define CONN_CFG_TAG_THINGY 1
// #define CONN_CFG_TAG                    1                                           /**< A tag that refers to the BLE stack configuration we set with @ref sd_ble_cfg_set. Default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */
//#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
//
//
//#define RANDOM_VECTOR_DEVICE_ID_SIZE         4
//static uint16_t      m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH; /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
//static m_ble_evt_handler_t        m_evt_handler = 0;
//static m_ble_service_handle_t   * m_service_handles = 0;
//static uint32_t                   m_service_num = 0;
//static char                       m_mac_addr[SUPPORT_FUNC_MAC_ADDR_STR_LEN];            /**< The device MAC address. */
//static uint8_t                    m_random_vector_device_id[RANDOM_VECTOR_DEVICE_ID_SIZE];        /**< Device random ID. Used for NFC BLE pairng on iOS. */
//#define NRF_BLE_MAX_MTU_SIZE            BLE_GATT_ATT_MTU_DEFAULT*12         /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
// #define BLE_GAP_EVENT_LENGTH_MAX 200


////END UART
//
////#define RAW_Q_FORMAT_ACC_INTEGER_BITS   (6)
//
//#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
//#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
//
//
//#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */
//#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */
//

//static m_ble_service_handle_t  m_ble_service_handles[THINGY_SERVICES_MAX];
//static ble_uuid_t m_adv_uuids[] = {
//    {BLE_UUID_NUS_SERVICE, BLE_UUID_TYPE_BLE},
//    {BLE_UUID_OUR_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN},
//};  /**< Universally unique service identifier. */
//
//
///*****************************************************************************
// * Static functions
// *****************************************************************************/
//
//static ble_gap_scan_params_t const m_scan_params =
//{
//    .active   = 1,
//    .interval = 0x00A0,
//    .window   = 0x0050,
//    .timeout  = 0x0000,
//};
//
//static void scan_start(void)
//{
//    ret_code_t ret;
//
//    ret = sd_ble_gap_scan_start(&m_scan_params);
//    APP_ERROR_CHECK(ret);
//}
//
//
///**
// * @brief Parses advertisement data, providing length and location of the field in case
// *        matching data is found.
// *
// * @param[in]  Type of data to be looked for in advertisement data.
// * @param[in]  Advertisement report length and pointer to report.
// * @param[out] If data type requested is found in the data report, type data length and
// *             pointer to data will be populated here.
// *
// * @retval NRF_SUCCESS if the data type is found in the report.
// * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
// */
//static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
//{
//    uint32_t   index = 0;
//    uint8_t  * p_data;
//
//    p_data = p_advdata->p_data;
//
//    while (index < p_advdata->data_len)
//    {
//        uint8_t field_length = p_data[index];
//        uint8_t field_type   = p_data[index + 1];
//
//        if (field_type == type)
//        {
//            p_typedata->p_data   = &p_data[index + 2];
//            p_typedata->data_len = field_length - 1;
//            return NRF_SUCCESS;
//        }
//        index += field_length + 1;
//    }
//    return NRF_ERROR_NOT_FOUND;
//}
//
//
///**@brief Function for searching a given name in the advertisement packets.
// *
// * @details Use this function to parse received advertising data and to find a given
// * name in them either as 'complete_local_name' or as 'short_local_name'.
// *
// * @param[in]   p_adv_report   advertising data to parse.
// * @param[in]   name_to_find   name to search.
// * @return   true if the given name was found, false otherwise.
// */
//static bool find_adv_name(ble_gap_evt_adv_report_t const * p_adv_report, char const * name_to_find)
//{
//    ret_code_t err_code;
//    data_t     adv_data;
//    data_t     dev_name;
//
//    // Initialize advertisement report for parsing
//    adv_data.p_data   = (uint8_t *)p_adv_report->data;
//    adv_data.data_len = p_adv_report->dlen;
//
//    //search for advertising names
//    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, &adv_data, &dev_name);
//    if (err_code == NRF_SUCCESS)
//    {
//        if (memcmp(name_to_find, dev_name.p_data, dev_name.data_len) == 0)
//        {
//            return true;
//        }
//    }
//    else
//    {
//        // Look for the short local name if it was not found as complete
//        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, &adv_data, &dev_name);
//        if (err_code != NRF_SUCCESS)
//        {
//            return false;
//        }
//        if (memcmp(m_target_periph_name, dev_name.p_data, dev_name.data_len) == 0)
//        {
//            return true;
//        }
//    }
//    return false;
//}
//
///**@brief Function for searching a UUID in the advertisement packets.
// *
// * @details Use this function to parse received advertising data and to find a given
// * UUID in them.
// *
// * @param[in]   p_adv_report   advertising data to parse.
// * @param[in]   uuid_to_find   UUIID to search.
// * @return   true if the given UUID was found, false otherwise.
// */
//static bool find_adv_uuid(ble_gap_evt_adv_report_t const * p_adv_report, uint16_t uuid_to_find)
//{
//    ret_code_t err_code;
//    data_t     adv_data;
//    data_t     type_data;
//
//    // Initialize advertisement report for parsing.
//    adv_data.p_data   = (uint8_t *)p_adv_report->data;
//    adv_data.data_len = p_adv_report->dlen;
//
//    err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE,
//                                &adv_data,
//                                &type_data);
//
//    if (err_code != NRF_SUCCESS)
//    {
//        // Look for the services in 'complete' if it was not found in 'more available'.
//        err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE,
//                                    &adv_data,
//                                    &type_data);
//
//        if (err_code != NRF_SUCCESS)
//        {
//            // If we can't parse the data, then exit.
//            return false;
//        }
//    }
//
//    // Verify if any UUID match the given UUID.
//    for (uint32_t i = 0; i < (type_data.data_len / sizeof(uint16_t)); i++)
//    {
//        uint16_t extracted_uuid = uint16_decode(&type_data.p_data[i * sizeof(uint16_t)]);
//        if (extracted_uuid == uuid_to_find)
//        {
//            return true;
//        }
//    }
//    return false;
//}
//
//
///**@brief Function for checking if a link already exists with a new connected peer.
// *
// * @details This function checks if a newly connected device is already connected
// *
// * @param[in]   p_connected_evt Bluetooth connected event.
// * @return                      True if the peer's address is found in the list of connected peers,
// *                              false otherwise.
// */
//static bool is_already_connected(ble_gap_addr_t const * p_connected_adr)
//{
//    for (uint32_t i = 0; i < NRF_BLE_LINK_COUNT; i++)
//    {
//        if (m_connected_peers[i].is_connected)
//        {
//            if (m_connected_peers[i].address.addr_type == p_connected_adr->addr_type)
//            {
//                if (memcmp(m_connected_peers[i].address.addr,
//                           p_connected_adr->addr,
//                           sizeof(m_connected_peers[i].address.addr)) == 0)
//                {
//                    return true;
//                }
//            }
//        }
//    }
//    return false;
//}


///**@brief Function for handling errors from the Connection Parameters module.
// *
// * @param[in] nrf_error  Error code containing information about what went wrong.
// */
//static void conn_params_error_handler(uint32_t nrf_error)
//{
//    APP_ERROR_HANDLER(nrf_error);
//}

//static void notify_nearby_tags(void){
//  if(tags_nearby){
//    led_set(&led_found, NULL);
//  }else{
//    led_set(&led_search, NULL);
//  }
//}

//static const int rssiThresh = -40;
//static int curRssi;
//static bool were_tags_nearby = false;


///**@brief Function for handling BLE Stack events common to both the central and peripheral roles.
// * @param[in] conn_handle Connection Handle.
// * @param[in] p_ble_evt  Bluetooth stack event.
// */
//static void on_ble_evt(uint16_t conn_handle, ble_evt_t * p_ble_evt)
//{
//    ret_code_t err_code;
//    uint16_t role = ble_conn_state_role(conn_handle);
//
//    switch (p_ble_evt->header.evt_id)
//    {
//        case BLE_GAP_EVT_CONNECTED:
//              NRF_LOG_INFO("on_ble_evt: connected\r\n");
//              m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//            break;
//
//        case BLE_GAP_EVT_DISCONNECTED:
//            m_conn_handle = BLE_CONN_HANDLE_INVALID;
//            m_connected_peers[conn_handle].is_connected = false;
//            break;
//
//        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
//            NRF_LOG_INFO("on_ble_evt: -- BLE_GAP_EVT_SEC_PARAMS_REQUEST --- Sending \'Pairing unsupported\' reply...\r\n");
//            // Pairing not supported
//            err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_GAP_EVT_SEC_PARAMS_REQUEST
//
//        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
//            // No system attributes have been stored.
//            NRF_LOG_INFO("on_ble_evt: -- BLE_GATTS_EVT_SYS_ATTR_MISSING --- No system attributes have been stored.\r\n");
//
//            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_GATTS_EVT_SYS_ATTR_MISSING
//
//        case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
//        {
//            NRF_LOG_INFO("on_ble_evt: BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST\r\n");
//            ble_gap_data_length_params_t dl_params;
//
//            // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
//            memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
//            err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
//            APP_ERROR_CHECK(err_code);
//        } break;
//
//        case BLE_GATTC_EVT_TIMEOUT:
//            // Disconnect on GATT Client timeout event.
//            NRF_LOG_INFO("on_ble_evt: Disconnect on GATT Client timeout event.\r\n");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_GATTC_EVT_TIMEOUT
//
//        case BLE_GATTS_EVT_TIMEOUT:
//            // Disconnect on GATT Server timeout event.
//            NRF_LOG_INFO("on_ble_evt: Disconnect on GATT Server timeout event.\r\n");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_GATTS_EVT_TIMEOUT
//
//        case BLE_EVT_USER_MEM_REQUEST:
//            NRF_LOG_INFO("on_ble_evt: BLE_EVT_USER_MEM_REQUEST\r\n");
//            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_EVT_USER_MEM_REQUEST
//
//        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
//        {
//            NRF_LOG_INFO("on_ble_evt: BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST\r\n");
//            ble_gatts_evt_rw_authorize_request_t  req;
//            ble_gatts_rw_authorize_reply_params_t auth_reply;
//
//            req = p_ble_evt->evt.gatts_evt.params.authorize_request;
//
//            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
//            {
//                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
//                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
//                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
//                {
//                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE){
//                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
//                    }else{
//                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
//                    }
//                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
//                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,&auth_reply);
//                    APP_ERROR_CHECK(err_code);
//                }
//            }
//        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
//
//        default:
//            // No implementation needed.
//            break;
//    }
//}
//
///**@brief Function for handling BLE Stack events involving peripheral applications. Manages the
// * LEDs used to report the status of the peripheral applications.
// *
// * @param[in] p_ble_evt  Bluetooth stack event.
// */
//static void on_ble_peripheral_evt(ble_evt_t * p_ble_evt)
//{
//    ret_code_t err_code;
//    switch (p_ble_evt->header.evt_id)
//    {
//        case BLE_GAP_EVT_CONNECTED:
//            NRF_LOG_INFO("PERIPHERAL: connected\r\n");
//            break; // BLE_GAP_EVT_CONNECTED
//
//        case BLE_GAP_EVT_DISCONNECTED:
//            NRF_LOG_INFO("PERIPHERAL: disconnected, reason 0x%x\r\n",
//                          p_ble_evt->evt.gap_evt.params.disconnected.reason);
//        break; // BLE_GAP_EVT_DISCONNECTED
//
//        case BLE_GATTC_EVT_TIMEOUT:
//            // Disconnect on GATT Client timeout event.
//            NRF_LOG_DEBUG("PERIPHERAL: GATT Client Timeout.\r\n");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
//                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_GATTC_EVT_TIMEOUT
//
//        case BLE_GATTS_EVT_TIMEOUT:
//            // Disconnect on GATT Server timeout event.
//            NRF_LOG_DEBUG("PERIPHERAL: GATT Server Timeout.\r\n");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
//                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_GATTS_EVT_TIMEOUT
//
//        case BLE_EVT_USER_MEM_REQUEST:
//            NRF_LOG_DEBUG("PERIPHERAL: BLE_EVT_USER_MEM_REQUEST\r\n");
//            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gap_evt.conn_handle, NULL);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_EVT_USER_MEM_REQUEST
//
//        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
//        {
//            NRF_LOG_DEBUG("PERIPHERAL: BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST\r\n");
//        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
//
//        default:
//            // No implementation needed.
//            break;
//    }
//}
//
///**@brief Function for handling advertising events.
// *
// * @param[in] ble_adv_evt  Advertising event.
// */
//static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
//{
//    switch (ble_adv_evt)
//    {
//        case BLE_ADV_EVT_FAST:
//            break;
//
//        case BLE_ADV_EVT_IDLE:
//        {
//            ret_code_t err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
//            APP_ERROR_CHECK(err_code);
//        } break;
//
//        default:
//            // No implementation needed.
//            break;
//    }
//}

///**@brief Function for handling BLE Stack events concerning central applications.
// *
// * @details This function keeps the connection handles of central applications up-to-date. It
// * parses scanning reports, initiating a connection attempt to peripherals when a target UUID
// * is found, and manages connection parameter update requests. Additionally, it updates the status
// * of LEDs used to report central applications activity.
// *
// * @note        Since this function updates connection handles, @ref BLE_GAP_EVT_DISCONNECTED events
// *              should be dispatched to the target application before invoking this function.
// *
// * @param[in]   p_ble_evt   Bluetooth stack event.
// */
//static void on_ble_central_evt(ble_evt_t const * p_ble_evt)
//{
//    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
//    ret_code_t            err_code;
//
//    switch (p_ble_evt->header.evt_id)
//    {
//        // Upon connection, check which peripheral has connected (HR or RSC), initiate DB
//        //  discovery, update LEDs status and resume scanning if necessary.
//        case BLE_GAP_EVT_CONNECTED:
//        {
//            NRF_LOG_INFO("CENTRAL: connected\r\n");
//            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//            // Update LEDs status, and check if we should be looking for more
//            //  peripherals to connect to.
//            if (ble_conn_state_n_centrals() == NRF_BLE_CENTRAL_LINK_COUNT){
////                bsp_board_led_off(CENTRAL_SCANNING_LED);
//            }else{/** Resume scanning. **/}
//
//        } break; // BLE_GAP_EVT_CONNECTED
//
//        // Upon disconnection, reset the connection handle of the peer which disconnected, update
//        // the LEDs status and start scanning again.
//        case BLE_GAP_EVT_DISCONNECTED:
//        {
//            m_conn_handle = BLE_CONN_HANDLE_INVALID;
//            NRF_LOG_INFO("CENTRAL: disconnected (reason: 0x%x)\r\n", p_gap_evt->params.disconnected.reason);
//        } break; // BLE_GAP_EVT_DISCONNECTED
//
//        case BLE_GAP_EVT_ADV_REPORT:
//        {
//            were_tags_nearby = tags_nearby;
//            const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
//
//            temp_report = &p_gap_evt->params.adv_report;
//            tmpName = (char*)temp_report->peer_addr.addr;
//            tmpAddr = temp_report->peer_addr.addr;
//
//            int tmpRssi = -10000;
//            if (strlen(m_target_periph_name) != 0){
//                if (memcmp(target_mac, tmpAddr, 6)== 0){
//                    tmpRssi = temp_report->rssi;
////                    NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" Found Target Device! =====  %d\r\n",temp_report->rssi);
//                }else if (memcmp(target_mac_rvr, tmpAddr, 6)== 0){
//                    tmpRssi = temp_report->rssi;
////                    NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN" Found Target Device Reverse! =====  %d\r\n",temp_report->rssi);
//                }else if (strlen(_buffer) != 0){
////                    NRF_LOG_INFO("CENTRAL: Looking for alternative name for advertising peer (\'%s\')...\r\n",_buffer);
//                    if (find_adv_name(&p_gap_evt->params.adv_report, _buffer))
//                    {
//                        if(!name_found){
//                            NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"CENTRAL: Alternative Target (\'%s\') Found with MAC (\'",_buffer);
////                            altAddr = &p_gap_evt->params.adv_report.peer_addr.addr;
//                            for (uint8_t i = 0; i < 6; i++)
//                            {
//                                NRF_LOG_RAW_INFO("%02x ", tmpAddr[i]);
//                            }
//
//                            NRF_LOG_RAW_INFO("\')!\r\n");
//                            name_found = true;
//                        }
//                        tmpRssi = temp_report->rssi;
//                    }else{
//                        tmpRssi = -10000;
//                    }
//                }else{
//                    tmpRssi = -10000;
//                }
//
//                if(tmpRssi >= rssiThresh){
//                  tags_nearby = true;
//                  curRssi = tmpRssi;
//                  NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"CENTRAL: Found Target Device Reverse! =====  %d\r\n",tmpRssi);
//                }else{
//                  tags_nearby = false;
//                }
//            }
//
//            if (is_already_connected(&p_gap_evt->params.adv_report.peer_addr))
//            {
//                NRF_LOG_INFO("central Already connected to something...\r\n");
//                break;
//            }
//
//            bool do_connect = false;
//            if (strlen(m_target_phone_name) != 0)
//            {
//                if (find_adv_name(&p_gap_evt->params.adv_report, m_target_phone_name))
//                {
//                    NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN"CENTRAL: Host Phone Found!\r\n");
//                    do_connect = true;
//                }else{}
//            }
//            else
//            {
//                // We do not want to connect to two peripherals offering the same service, so when
//                // a UUID is matched, we check that we are not already connected to a peer which
//                // offers the same service.
//                if (   find_adv_uuid(&p_gap_evt->params.adv_report, BLE_UUID_NUS_SERVICE) && (m_conn_handle == BLE_CONN_HANDLE_INVALID))
//                {
//                    do_connect = true;
//                }
//            }
//            if (do_connect)
//            {
//                // Initiate connection.
//                NRF_LOG_INFO("central connecting ...\r\n");
//                err_code = sd_ble_gap_connect(&p_gap_evt->params.adv_report.peer_addr,
//                                              &m_scan_params,
//                                              &m_connection_param,
//                                              APP_CONN_CFG_TAG);
//
//                if (err_code != NRF_SUCCESS)
//                {
//                    NRF_LOG_DEBUG("Connection Request Failed, reason %d\r\n", err_code);
//                }
//            }
//        } break; // BLE_GAP_ADV_REPORT
//
//        case BLE_GAP_EVT_TIMEOUT:
//        {
//            // We have not specified a timeout for scanning, so only connection attemps can timeout.
//            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN){
//                NRF_LOG_DEBUG("CENTRAL: Connection Request timed out.\r\n");
//            }
//        } break; // BLE_GAP_EVT_TIMEOUT
//
//        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
//        {
//             // Accept parameters requested by peer.
//            NRF_LOG_DEBUG("CENTRAL: BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST\r\n");
//            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle, &p_gap_evt->params.conn_param_update_request.conn_params);
//            APP_ERROR_CHECK(err_code);
//        } break; // BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST
//
//        case BLE_GATTC_EVT_TIMEOUT:
//            // Disconnect on GATT Client timeout event.
//            NRF_LOG_DEBUG("CENTRAL: GATT Client Timeout.\r\n");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_GATTC_EVT_TIMEOUT
//
//        case BLE_GATTS_EVT_TIMEOUT:
//            // Disconnect on GATT Server timeout event.
//            NRF_LOG_DEBUG("CENTRAL: GATT Server Timeout.\r\n");
//            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//            APP_ERROR_CHECK(err_code);
//            break; // BLE_GATTS_EVT_TIMEOUT
//
//        default:
//            // No implementation needed.
//            break;
//    }
//}
//
//static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
//{
//  uint16_t conn_handle;
//  uint16_t role;
//
//  ble_conn_params_on_ble_evt(p_ble_evt);
//  ble_conn_state_on_ble_evt(p_ble_evt);
//  nrf_ble_gatt_on_ble_evt(&m_gatt, p_ble_evt);
//  ble_nus_on_ble_evt(&m_nus, p_ble_evt);
//  ble_nus_on_ble_evt(&m_nus2, p_ble_evt);
//
//  conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//  role        = ble_conn_state_role(conn_handle);
//  on_ble_evt(conn_handle, p_ble_evt);
//
//  // Based on the role this device plays in the connection, dispatch to the right applications.
//  if (role == BLE_GAP_ROLE_PERIPH)
//  {
//      // Manages peripheral LEDs.
//      on_ble_peripheral_evt(p_ble_evt);
//
//      ble_advertising_on_ble_evt(p_ble_evt);
//      ble_conn_params_on_ble_evt(p_ble_evt);
//  }
//  else if ((role == BLE_GAP_ROLE_CENTRAL) || (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_REPORT))
//  {
//      // on_ble_central_evt(p_ble_evt);
//
//      // on_ble_central_evt() will update the connection handles, so we want to execute it
//      // after dispatching to the central applications upon disconnection.
//      if (p_ble_evt->header.evt_id != BLE_GAP_EVT_DISCONNECTED){
//          on_ble_central_evt(p_ble_evt);
//      }
//
//      // If the peer disconnected, we update the connection handles last.
//      if (p_ble_evt->header.evt_id == BLE_GAP_EVT_DISCONNECTED){
//          on_ble_central_evt(p_ble_evt);
//      }
//  }
//}
//
///**@brief   Function for handling app_uart events.
// *
// * @details This function will receive a single character from the app_uart module and append it to
// *          a string. The string will be be sent over BLE when the last character received was a
// *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
// */
//void uart_event_handle(app_uart_evt_t * p_event)
//{
//    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
//    static uint8_t index = 0;
//    uint32_t       err_code;
//
//    switch (p_event->evt_type)
//    {
//        /**@snippet [Handling data from UART] */
//        case APP_UART_DATA_READY:
//            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
//            index++;
//
//            if ((data_array[index - 1] == '\n') || (index >= (m_ble_nus_max_data_len)))
//            {
//                NRF_LOG_DEBUG("Ready to send data over BLE NUS\r\n");
//                NRF_LOG_HEXDUMP_DEBUG(data_array, index);
//
//                do
//                {
//                    err_code = ble_nus_string_send(&m_nus, data_array, index);
//                    if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
//                    {
//                        APP_ERROR_CHECK(err_code);
//                    }
//                } while (err_code == NRF_ERROR_BUSY);
//
//                index = 0;
//            }
//            break;
//
//        /**@snippet [Handling data from UART] */
//        case APP_UART_COMMUNICATION_ERROR:
//            NRF_LOG_ERROR("Communication error occurred while handling UART.\r\n");
//            APP_ERROR_HANDLER(p_event->data.error_communication);
//            break;
//
//        case APP_UART_FIFO_ERROR:
//            NRF_LOG_ERROR("Error occurred in FIFO module used by UART.\r\n");
//            APP_ERROR_HANDLER(p_event->data.error_code);
//            break;
//
//        default:
//            break;
//    }
//}
//
//
///**@brief  Function for initializing the UART module.
// */
///**@snippet [UART Initialization] */
//static void uart_init(void)
//{
//    uint32_t                     err_code;
//    const app_uart_comm_params_t comm_params =
//    {
//        .rx_pin_no    = UART_PIN_DISCONNECTED,
//        .tx_pin_no    = UART_PIN_DISCONNECTED,
//        .rts_pin_no   = UART_PIN_DISCONNECTED,
//        .cts_pin_no   = UART_PIN_DISCONNECTED,
//
//        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
//        .use_parity   = false,
//        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
//    };
//
//    APP_UART_FIFO_INIT(&comm_params,
//                       UART_RX_BUF_SIZE,
//                       UART_TX_BUF_SIZE,
//                       uart_event_handle,
//                       APP_IRQ_PRIORITY_LOWEST,
//                       err_code);
//    APP_ERROR_CHECK(err_code);
//}
///**@brief Function for handling the data from the Nordic UART Service.
// *
// * @details This function will process the data received from the Nordic UART BLE Service and send
// *          it to the UART module.
// *
// * @param[in] p_nus    Nordic UART Service structure.
// * @param[in] p_data   Data to be send to UART module.
// * @param[in] length   Length of the data.
// */
///**@snippet [Handling the data received over BLE] */
//static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
//{
//    ret_code_t ret_val;
//    NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.\r\n");
//    NRF_LOG_HEXDUMP_DEBUG(p_data, length);
//
//    memset(_buffer, 0, sizeof(_buffer));
//    memcpy(&_buffer[0], (char*)p_data, sizeof(uint8_t)*length);
//    NRF_LOG_INFO("nus_data_handler(): ----- Data = %s\r\n",_buffer);
//
//    for (uint32_t i = 0; i < length; i++)
//    {
//        do
//        {
//            ret_val = app_uart_put(p_data[i]);
//            ret_val = NRF_SUCCESS;
//            if ((ret_val != NRF_SUCCESS) && (ret_val != NRF_ERROR_BUSY))
//            {
//                NRF_LOG_ERROR("app_uart_put failed for index 0x%04x.\r\n", i);
//                APP_ERROR_CHECK(ret_val);
//            }
//        } while (ret_val == NRF_ERROR_BUSY);
//    }
//    if (p_data[length-1] == '\r')
//    {
//        while (app_uart_put('\n') == NRF_ERROR_BUSY);
//    }
//    if (flag_echoback)
//    {
//       // Send data back to peripheral.
////       do
////       {
////           ret_val = app_uart(&m_ble_nus_c, p_data, data_len);
////           if ((ret_val != NRF_SUCCESS) && (ret_val != NRF_ERROR_BUSY))
////           {
////               NRF_LOG_ERROR("Failed sending NUS message. Error 0x%x. \r\n", ret_val);
////               APP_ERROR_CHECK(ret_val);
////           }
////       } while (ret_val == NRF_ERROR_BUSY);
//    }
//
//}
//
///**@brief Function for dispatching system events from the SoftDevice.
// *
// * @details This function is called from the SoftDevice event interrupt handler after a
// *          SoftDevice event has been received.
// *
// * @param[in] evt_id  System event id.
// */
//static void sys_evt_dispatch(uint32_t evt_id)
//{
//    ble_advertising_on_sys_evt(evt_id);
//}
//
//static uint32_t advertising_init(void)
//{
//    uint32_t      err_code;
//    ble_advdata_t advdata;//BLE_UUID_TYPE_BLE
//    ble_uuid_t    adv_uuids = {BLE_UUID_NUS_SERVICE,BLE_UUID_TYPE_BLE};
//
//    // Build advertising data struct to pass into @ref ble_advertising_init.
//    memset(&advdata, 0, sizeof(advdata));
//
//    advdata.name_type                       = BLE_ADVDATA_FULL_NAME;
//    advdata.include_appearance              = false;
//    advdata.flags                           = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
//    advdata.uuids_more_available.uuid_cnt   = 1;
//    advdata.uuids_more_available.p_uuids    = &adv_uuids;
//
//    ble_adv_modes_config_t options = {0};
//    options.ble_adv_fast_enabled  = true;
//    options.ble_adv_fast_interval = APP_ADV_INTERVAL_MS;
//    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;
//
//
//    ble_advdata_t scanrsp;
//    memset(&scanrsp, 0, sizeof(scanrsp));
//    // Manufacturer specific data in advertising packet.
//    ble_advdata_manuf_data_t adv_manuf_data;
//    uint8_t random_vector_device_id_reversed[RANDOM_VECTOR_DEVICE_ID_SIZE];
//    for (uint8_t i = 0; i < RANDOM_VECTOR_DEVICE_ID_SIZE; i++)
//    {
//        random_vector_device_id_reversed[i] = m_random_vector_device_id[RANDOM_VECTOR_DEVICE_ID_SIZE - i - 1];
//    }
//
//    adv_manuf_data.company_identifier   = NORDIC_COMPANY_ID;
//    adv_manuf_data.data.p_data          = random_vector_device_id_reversed;
//    adv_manuf_data.data.size            = RANDOM_VECTOR_DEVICE_ID_SIZE;
//
//    scanrsp.name_type             = BLE_ADVDATA_NO_NAME;
//    scanrsp.p_manuf_specific_data = &adv_manuf_data;
////    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
////    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;
//
//    // Set both advertisement data and scan response data.
//    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
//    RETURN_IF_ERROR(err_code);
//
//    ble_advertising_conn_cfg_tag_set(CONN_CFG_TAG_THINGY);
//
//    return NRF_SUCCESS;
//}
//
///**@brief Function for initializing the BLE stack.
// *
// * @details Initializes the SoftDevice and the BLE event interrupts.
// */
//static uint32_t test_ble_stack_init(void)
//{
//    ret_code_t err_code;
//
//    // Initialize the SoftDevice handler module.
//    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
//    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
//
//    // Fetch the start address of the application RAM.
//    uint32_t ram_start = 0;
//    err_code = softdevice_app_ram_start_get(&ram_start);
//    APP_ERROR_CHECK(err_code);
//
//    // Overwrite some of the default configurations for the BLE stack.
//    ble_cfg_t ble_cfg;
//
//    // Configure the number of custom UUIDS.
//    memset(&ble_cfg, 0, sizeof(ble_cfg));
//    ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 3;
//    err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);
//    APP_ERROR_CHECK(err_code);
//
//    // Configure the maximum number of connections.
//    memset(&ble_cfg, 0, sizeof(ble_cfg));
//    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = NRF_BLE_PERIPHERAL_LINK_COUNT;
//    ble_cfg.gap_cfg.role_count_cfg.central_role_count = NRF_BLE_CENTRAL_LINK_COUNT;
//    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = NRF_BLE_CENTRAL_LINK_COUNT;
//    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
//    APP_ERROR_CHECK(err_code);
//
//    // Configure the maximum ATT MTU.
//    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
//    ble_cfg.conn_cfg.conn_cfg_tag                 = CONN_CFG_TAG;
//    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = NRF_BLE_GATT_MAX_MTU_SIZE;
//    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATT, &ble_cfg, ram_start);
//    APP_ERROR_CHECK(err_code);
//
//    memset(&ble_cfg, 0, sizeof(ble_cfg));
//    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count     = NRF_BLE_LINK_COUNT;
//    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length   = BLE_GAP_EVENT_LENGTH_MAX;
//    ble_cfg.conn_cfg.conn_cfg_tag                       = APP_CONN_CFG_TAG;
//    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &ble_cfg, ram_start);
//    APP_ERROR_CHECK(err_code);
//
//    // Enable BLE stack.
//    err_code = softdevice_enable(&ram_start);
//    APP_ERROR_CHECK(err_code);
//
//    // Register with the SoftDevice handler module for BLE events.
//    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
//    APP_ERROR_CHECK(err_code);
//
//    // Register with the SoftDevice handler module for System events.
//    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
//    APP_ERROR_CHECK(err_code);
//    return NRF_SUCCESS;
//}
//
///**@brief Function for the GAP initialization.
// *
// * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
// *          device including the device name, appearance, and the preferred connection parameters.
// */
//static uint32_t test_gap_params_init(void)
//{
//    ret_code_t              err_code;
//    ble_gap_conn_params_t   gap_conn_params;
//    ble_gap_conn_sec_mode_t sec_mode;
//
//    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
//
//    err_code = sd_ble_gap_device_name_set(&sec_mode,(const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
//    APP_ERROR_CHECK(err_code);
//
//    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
//
//    gap_conn_params.min_conn_interval = MIN_CONNECTION_INTERVAL;
//    gap_conn_params.max_conn_interval = MAX_CONNECTION_INTERVAL;
//    gap_conn_params.slave_latency     = SLAVE_LATENCY;
//    gap_conn_params.conn_sup_timeout  = SUPERVISION_TIMEOUT;
//
//    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
//    APP_ERROR_CHECK(err_code);
//    return NRF_SUCCESS;
//}
//
///**@brief Function for handling events from the GATT library. */
//void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, const nrf_ble_gatt_evt_t * p_evt)
//{
//    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
//    {
//        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
//        NRF_LOG_INFO("Data len is set to 0x%X(%d)\r\n", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
//    }
//    NRF_LOG_INFO("ATT MTU exchange completed. central 0x%x peripheral 0x%x\r\n", p_gatt->att_mtu_desired_central, p_gatt->att_mtu_desired_periph);
//}
//
//
///**@brief Function for initializing the GATT module.
// */
//static uint32_t test_gatt_init(void)
//{
//    ret_code_t err_code;
//    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
//    APP_ERROR_CHECK(err_code);
//
//    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, 64);
//    APP_ERROR_CHECK(err_code);
//    return NRF_SUCCESS;
//}
//
///**@brief Function for initializing the Connection Parameters module.
// */
//static uint32_t test_conn_params_init(void)
//{
//    ret_code_t             err_code;
//    ble_conn_params_init_t cp_init;
//
//    memset(&cp_init, 0, sizeof(cp_init));
//
//    cp_init.p_conn_params                  = NULL;
//    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
//    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
//    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
//    cp_init.start_on_notify_cccd_handle    = BLE_CONN_HANDLE_INVALID; // Start upon connection.
//    cp_init.disconnect_on_fail             = true;
//    cp_init.evt_handler                    = NULL;  // Ignore events.
//    cp_init.error_handler                  = conn_params_error_handler;
//
//    err_code = ble_conn_params_init(&cp_init);
//    APP_ERROR_CHECK(err_code);
//    return NRF_SUCCESS;
//}
//
///**@brief Generate random number.
// */
//static uint32_t random_vector_generate(uint8_t * p_buff, uint8_t size)
//{
//    uint32_t err_code;
//    uint8_t  bytes_available = 0;
//
//    nrf_drv_rng_bytes_available(&bytes_available);
//    uint8_t retries = 0;
//
//    while (bytes_available < size)
//    {
//        retries++;
//        NRF_LOG_WARNING("Too few random bytes available. Trying again \r\n");
//        nrf_drv_rng_bytes_available(&bytes_available);
//        nrf_delay_ms(5);
//
//        if (retries > 5)    // Return after n attempts.
//        {
//            return NRF_ERROR_TIMEOUT;
//        }
//    }
//
//    NRF_LOG_INFO("Available random bytes: %d \r\n", bytes_available);
//
//    err_code = nrf_drv_rng_rand(p_buff, size);
//    RETURN_IF_ERROR(err_code);
//
//    NRF_LOG_INFO("Random value (hex): ");
//
//    for (uint8_t i = 0; i < size; i++)
//    {
//        NRF_LOG_RAW_INFO("%02x", p_buff[i]);
//    }
//
//    NRF_LOG_RAW_INFO("\r\n");
//
//    return NRF_SUCCESS;
//}
//
//static uint32_t ble_services_init(m_ble_service_handle_t * p_service_handles, uint32_t num_services)
//{
//    uint32_t err_code;
//    ble_nus_init_t nus_init;
//    ble_nus_init_t nus_init2;
//
//    nus_init.data_handler = nus_data_handler;
//    nus_init2.data_handler = nus_data_handler;
//
//    err_code = ble_nus_init(&m_nus, &nus_init);
//    APP_ERROR_CHECK(err_code);
//
//    err_code = ble_nus_init(&m_nus2, &nus_init2);
//    APP_ERROR_CHECK(err_code);
//
//    return NRF_SUCCESS;
//}
//
//
//static void thingy_ble_evt_handler(m_ble_evt_t * p_evt)
//{
//    switch (p_evt->evt_type)
//    {
//        case thingy_ble_evt_connected:
//            NRF_LOG_INFO(NRF_LOG_COLOR_CODE_GREEN "Thingy_ble_evt_connected \r\n");
//            break;
//
//        case thingy_ble_evt_disconnected:
//            NRF_LOG_INFO(NRF_LOG_COLOR_CODE_YELLOW "Thingy_ble_evt_disconnected \r\n");
//            NRF_LOG_FINAL_FLUSH();
//            nrf_delay_ms(5);
//            NVIC_SystemReset();
//            break;
//
//        case thingy_ble_evt_timeout:
//            NRF_LOG_INFO(NRF_LOG_COLOR_CODE_YELLOW "Thingy_ble_evt_timeout \r\n");
////            sleep_mode_enter();
////            NVIC_SystemReset();
//            break;
//    }
//}
//
//
//static uint32_t temp_m_ble_init(m_ble_init_t * p_params)
//{
//  uint32_t err_code;
//
//  m_evt_handler     = p_params->evt_handler;
//  m_service_handles = p_params->p_service_handles;
//  m_service_num     = p_params->service_num;
//
//  err_code = random_vector_generate(m_random_vector_device_id, RANDOM_VECTOR_DEVICE_ID_SIZE);
//  if (err_code != NRF_SUCCESS)
//  {
//      NRF_LOG_ERROR("random_vector_generate failed - %d\r\n", err_code);
//      return err_code;
//  }
//
//  err_code = test_gap_params_init();
//  if (err_code != NRF_SUCCESS)
//  {
//      NRF_LOG_ERROR("gap_params_init failed - %d\r\n", err_code);
//      return err_code;
//  }
//
//  err_code = test_gatt_init();
//  if (err_code != NRF_SUCCESS)
//  {
//      NRF_LOG_ERROR("gatt_init failed - %d\r\n", err_code);
//      return err_code;
//  }
//
//  err_code = ble_services_init(m_service_handles, m_service_num);
//  if (err_code != NRF_SUCCESS)
//  {
//      NRF_LOG_ERROR("Thingy_ble_init: services_init failed - %d\r\n", err_code);
//      return err_code;
//  }
//
//  err_code = advertising_init();
//  if (err_code != NRF_SUCCESS)
//  {
//      NRF_LOG_ERROR("Advertising_init failed - %d\r\n", err_code);
//      return err_code;
//  }
//
//  err_code = test_conn_params_init();
//  if (err_code != NRF_SUCCESS)
//  {
//      NRF_LOG_ERROR("Conn_params_init failed - %d\r\n", err_code);
//      return err_code;
//  }
//
//  err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
//  if (err_code != NRF_SUCCESS)
//  {
//      NRF_LOG_ERROR("ble_advertising_start failed - %d\r\n", err_code);
//      return err_code;
//  }else{NRF_LOG_INFO("BLE Advertising\r\n");}
//
//  err_code = support_func_ble_mac_address_get(m_mac_addr);
//  if (err_code != NRF_SUCCESS)
//  {
//      NRF_LOG_ERROR("mac address get failed - %d\r\n", err_code);
//      return err_code;
//  }
//
//  NRF_LOG_RAW_INFO("MAC addr-> %s \r\n", nrf_log_push(m_mac_addr));
//  nrf_delay_ms (10);
//
//  return NRF_SUCCESS;
//}
//
///**@brief Function for initializing the Thingy.
// */
//static void tmp_thingy_init(void)
//{
//    uint32_t                 err_code;
////    m_ui_init_t              ui_params;
//    m_ble_init_t             ble_params;
//
////    /**@brief Initialize the TWI manager. */
////    err_code = twi_manager_init(APP_IRQ_PRIORITY_THREAD);
////    APP_ERROR_CHECK(err_code);
////
////    /**@brief Initialize LED and button UI module. */
////    ui_params.p_twi_instance = &m_twi_sensors;
////
////    /** Begin Original*/
////    err_code = m_ui_init(&ui_params);
////    APP_ERROR_CHECK(err_code);
////    /** End Original **/
//
//    /**@brief Initialize BLE handling module. */
//    ble_params.evt_handler       = thingy_ble_evt_handler;
//    ble_params.p_service_handles = m_ble_service_handles;
//    ble_params.service_num       = THINGY_SERVICES_MAX;
//
//    err_code = temp_m_ble_init(&ble_params);
//    APP_ERROR_CHECK(err_code);
//
////    led_set(&led_search, NULL);
//}
