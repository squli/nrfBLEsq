/*!
    @brief Custom service:
            1) 1 byte to control output register;
            2) 1 byte to control another output register;
            3) 1 byte to control input register;
            4) 1 byte to check the adc-input;
            5) 1 byte to store RSII of current connection.
            
            To create was used this tutorial - https://devzone.nordicsemi.com/tutorials/8/
*/

#include "sq_service.h"
#include "app_error.h"
#include "string.h"
#include "my_gpio_manager.h"

#define NRF_LOG_MODULE_NAME "CSERV"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define BLE_UUID_REG_OUT1_CHARACTERISTC_UUID    0x02
#define BLE_UUID_REG_OUT2_CHARACTERISTC_UUID    0x04
#define BLE_UUID_REG_IN_CHARACTERISTC_UUID      0x08
#define BLE_UUID_REG_ADC_CHARACTERISTC_UUID     0x0F
#define BLE_UUID_REG_RSSI_CHARACTERISTC_UUID    0x20


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_sqs       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_sq_t * p_sqs, ble_evt_t * p_ble_evt)
{
    p_sqs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_sqs       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_sq_t * p_sqs, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_sqs->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the Write event.
 *
 * @param[in]   p_sqs       sq service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_sq_t * p_sqs, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    if ( (p_evt_write->handle == p_sqs->sqs_reg_out1_handles.value_handle)
          && (p_evt_write->len == 1) )
    {
        NRF_LOG_INFO("WRITE 0x%x to REG_OUT1\r\n", p_evt_write->data[0]);
        my_gpio_out_change_state(GPIO_OUT_REG1, p_evt_write->data[0]);
    }
    
}

/**
 * @brief Function for initializing the sq service.
 *
 * @param[out]  p_sqs       SQ-Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_sqs_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
#pragma push /* Save existing optimization level */
#pragma O0   /* Optimization level now O0 */
uint32_t ble_sqs_init(ble_sq_t * p_sqs, ble_sq_init_t * p_sqs_init) {
        
    uint32_t err_code;
    
    /// Connection handle to invalid value
    p_sqs->conn_handle = BLE_CONN_HANDLE_INVALID;
        
    /// OUR_JOB: Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = BLE_BASE_UUID_SQ_SERVICE;
    service_uuid.uuid = BLE_UUID_SQ_SERVICE;
    
    p_sqs->evt_handler = p_sqs_init->evt_handler;    
    p_sqs->conn_handle = BLE_CONN_HANDLE_INVALID;
                
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /// add service to BLE stack
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                    &service_uuid,
                                    &p_sqs->service_handle);    
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /// then add all characteristics to ble stack - https://devzone.nordicsemi.com/tutorials/17/

    /**************
    *   REG_OUT1  *
    ***************/    
    /// Use custom UUID to define characteristic value type
    ble_uuid_t          char_uuid;
    char_uuid.uuid      = BLE_UUID_REG_OUT1_CHARACTERISTC_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    /// Configure the Attribute Metadata    
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    
    /// Set read/write permissions to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    /// Configure the Characteristic Value Attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));    
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    
    /// Add read/write properties to our characteristic value
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;
       
    /// Set characteristic length
    attr_char_value.max_len     = 1;
    attr_char_value.init_len    = 1;
    attr_char_value.p_value     = &(p_sqs_init->out_reg1_value);   
            
    /// Add the new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_sqs->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_sqs->sqs_reg_out1_handles);                                      
    
    APP_ERROR_CHECK(err_code);

    /**************
    *  REG_INPUT  *
    ***************/
    
    /// Use custom UUID to define characteristic value type
    char_uuid.uuid      = BLE_UUID_REG_IN_CHARACTERISTC_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    /// Configure the Attribute Metadata    
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    
    /// Configure the Characteristic Value Attribute
    memset(&attr_char_value, 0, sizeof(attr_char_value));    
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;    
    /// Set characteristic length
    attr_char_value.max_len     = 1;
    attr_char_value.init_len    = 1;
    attr_char_value.p_value     = &(p_sqs_init->in_reg_value);
    
    /// Add read/write properties to our characteristic value
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read     = 1;    
    char_md.char_props.notify   = 1;
    
    /// Set read/write permissions to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    /// Add the new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_sqs->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_sqs->sqs_reg_in_handles);
    APP_ERROR_CHECK(err_code);
               
    /**************
    *  ADC_INPUT  *
    ***************/
    
    /// Use custom UUID to define characteristic value type
    char_uuid.uuid      = BLE_UUID_REG_ADC_CHARACTERISTC_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    /// Configure the Attribute Metadata    
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    
    /// Configure the Characteristic Value Attribute
    memset(&attr_char_value, 0, sizeof(attr_char_value));    
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    
    /// Add read/write properties to our characteristic value
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read     = 1;    
    char_md.char_props.notify   = 1;
    
    /// Set read/write permissions to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    
    /// Set characteristic length
    attr_char_value.max_len     = 2;
    attr_char_value.init_len    = 2;
    attr_char_value.p_value     = &(p_sqs_init->adc_reg_value);
        
    /// Add the new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_sqs->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_sqs->sqs_adc_handles);
    APP_ERROR_CHECK(err_code);           
    
    /***************
    *  RSSI_VALUE  *
    ****************/
    
    /// Use custom UUID to define characteristic value type
    char_uuid.uuid      = BLE_UUID_REG_RSSI_CHARACTERISTC_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    /// Configure the Attribute Metadata    
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    
    /// Configure the Characteristic Value Attribute
    memset(&attr_char_value, 0, sizeof(attr_char_value));    
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    
    /// Add read/write properties to our characteristic value
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read     = 1;    
    char_md.char_props.notify   = 1;
    
    /// Set read/write permissions to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    
    /// Set characteristic length
    attr_char_value.max_len     = 1;
    attr_char_value.init_len    = 1;
    attr_char_value.p_value     = &(p_sqs_init->rssi_reg_value);
        
    /// Add the new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_sqs->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_sqs->sqs_rssi_handles);
    APP_ERROR_CHECK(err_code);           
    
    return err_code;
}
#pragma pop /* Restore original optimization level */

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the SQ-Service.
 *
 * @param[in]   p_sqs      SQ Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_sqs_on_ble_evt(ble_sq_t * p_sqs, ble_evt_t * p_ble_evt) {

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_sqs, p_ble_evt);
            break;
        
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_sqs, p_ble_evt);
            break;
        
        case BLE_GATTS_EVT_WRITE:
            on_write(p_sqs, p_ble_evt);
            break;
        
        default:
            // No implementation needed.
            break;
    }    
}

/**
    @brief update adc registers characteristic of sq_service with new value
    @param[in] p_sqs - sq service handler
    @param[in] value - new value of adc
*/
uint32_t sqs_update_adc_characteristic(ble_sq_t * p_sqs, uint16_t adc_value) {
    
    if (p_sqs != NULL) {
        if (p_sqs->reg_adc != adc_value) {
            /**
                update value in local database
            */
            
            uint32_t err_code = NRF_SUCCESS;
            ble_gatts_value_t gatts_value;
            
            // Initialize value struct.
            memset(&gatts_value, 0, sizeof(gatts_value));
    
            uint8_t adc_val[sizeof(uint16_t)];
            adc_val[0] = (adc_value & 0x00FF);
            adc_val[1] = (adc_value & 0xFF00)  >> 8;
            
            gatts_value.len     = sizeof(uint16_t);
            gatts_value.offset  = 0;
            gatts_value.p_value = &adc_val[0];
            
            // Update database.
            err_code = sd_ble_gatts_value_set(p_sqs->conn_handle,
                                            p_sqs->sqs_adc_handles.value_handle,
                                            &gatts_value);
            if (err_code == NRF_SUCCESS)
            {
                p_sqs->reg_adc = adc_value;
            }
            else
            {
                return err_code;
            }
            
            /**
                send notification of changed rssi:
            */                               
            if (p_sqs->conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                ble_gatts_hvx_params_t hvx_params;
            
                memset(&hvx_params, 0, sizeof(hvx_params));
            
                hvx_params.handle = p_sqs->sqs_adc_handles.value_handle;
                hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = gatts_value.offset;
                hvx_params.p_len  = &gatts_value.len;
                hvx_params.p_data = gatts_value.p_value;
            
                err_code = sd_ble_gatts_hvx(p_sqs->conn_handle, &hvx_params);
                return err_code;
            }
            else
            {
                err_code = NRF_ERROR_INVALID_STATE;
            }
            
            return err_code;
        }
        return NRF_SUCCESS;
    }
    else 
        return NRF_ERROR_NULL;        
}

/**
    @brief update input registers characteristic of sq_service with new value
    @param[in] p_sqs - sq service handler
    @param[in] value - new value of inputs
*/
uint32_t sqs_update_input_characteristic(ble_sq_t * p_sqs, uint8_t value) {
    
   if (p_sqs != NULL) {
        if (p_sqs->reg_in != value) {
            /**
                update value in local database
            */
            
            uint32_t err_code = NRF_SUCCESS;
            ble_gatts_value_t gatts_value;
            
            // Initialize value struct.
            memset(&gatts_value, 0, sizeof(gatts_value));
                
            gatts_value.len     = sizeof(uint8_t);
            gatts_value.offset  = 0;
            gatts_value.p_value = &value;
            
            // Update database.
            err_code = sd_ble_gatts_value_set(p_sqs->conn_handle,
                                            p_sqs->sqs_reg_in_handles.value_handle,
                                            &gatts_value);
            if (err_code == NRF_SUCCESS)
            {
                p_sqs->reg_adc = value;
            }
            else
            {
                return err_code;
            }
            
            /**
                send notification of changed rssi:
            */                               
            if (p_sqs->conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                ble_gatts_hvx_params_t hvx_params;
            
                memset(&hvx_params, 0, sizeof(hvx_params));
            
                hvx_params.handle = p_sqs->sqs_reg_in_handles.value_handle;
                hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = gatts_value.offset;
                hvx_params.p_len  = &gatts_value.len;
                hvx_params.p_data = gatts_value.p_value;
            
                err_code = sd_ble_gatts_hvx(p_sqs->conn_handle, &hvx_params);
                return err_code;
            }
            else
            {
                err_code = NRF_ERROR_INVALID_STATE;
            }
            
            return err_code;
        }
        return NRF_SUCCESS;
    }
    else 
        return NRF_ERROR_NULL;      
    
}

/**
    @brief update rssi characteristic of sq_service with new value
    @param[in] p_sqs - sq service handler
    @param[in] value - new value of rssi
*/
uint32_t sqs_update_rssi_characteristic(ble_sq_t * p_sqs, uint8_t value) {
    
    if (p_sqs != NULL) {
        if (p_sqs->reg_rssi != value) {
            /**
                update value in local database
            */
            
            uint32_t err_code = NRF_SUCCESS;
            ble_gatts_value_t gatts_value;
            
            // Initialize value struct.
            memset(&gatts_value, 0, sizeof(gatts_value));
    
            gatts_value.len     = sizeof(uint8_t);
            gatts_value.offset  = 0;
            gatts_value.p_value = &value;
    
            // Update database.
            err_code = sd_ble_gatts_value_set(p_sqs->conn_handle,
                                            p_sqs->sqs_rssi_handles.value_handle,
                                            &gatts_value);
            if (err_code == NRF_SUCCESS)
            {
                p_sqs->reg_rssi = value;
            }
            else
            {
                return err_code;
            }
            
            /**
                send notification of changed rssi:
            */                               
            if (p_sqs->conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                ble_gatts_hvx_params_t hvx_params;
            
                memset(&hvx_params, 0, sizeof(hvx_params));
            
                hvx_params.handle = p_sqs->sqs_rssi_handles.value_handle;
                hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = gatts_value.offset;
                hvx_params.p_len  = &gatts_value.len;
                hvx_params.p_data = gatts_value.p_value;
            
                err_code = sd_ble_gatts_hvx(p_sqs->conn_handle, &hvx_params);
                return err_code;
            }
            else
            {
                err_code = NRF_ERROR_INVALID_STATE;
            }
            return err_code;
        }
        return NRF_SUCCESS;
    }
    else 
        return NRF_ERROR_NULL;
}
