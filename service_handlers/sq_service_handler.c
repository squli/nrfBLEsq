/**
    
*/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include <string.h>
#include "sq_service_handler.h"

/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */

static ble_sq_t m_sqs;   /**< Structure used to identify the custom (sq_) service. */


/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

static void on_sq_evt(ble_sq_t * p_bas, ble_sq_evt_t * p_evt);

/**
    @brief Event handler for sq-service
*/
static void on_sq_evt(ble_sq_t * p_bas, ble_sq_evt_t * p_evt) {
    
    /// There are no special events of this service to handle it
    
}

/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */

/**
    @brief init function of service: set event handler and default values
*/
uint32_t sq_service_init(void) {
    
    uint32_t err_code;
    
    ble_sq_init_t    sqs_init;    
    memset(&sqs_init, 0, sizeof(ble_sq_init_t));
    
    sqs_init.evt_handler = on_sq_evt;
    sqs_init.in_reg_value   = 0xCC;
    sqs_init.out_reg1_value = 0xAA;
    sqs_init.out_reg2_value = 0xBB;
    sqs_init.rssi_reg_value = 0xDD;

    err_code = ble_sqs_init(&m_sqs, &sqs_init);
    
    return err_code;
}


/**
    @brief BLE-event handler of sq-service
*/
void sq_on_ble_evt(ble_evt_t * p_ble_evt) {
    ble_sqs_on_ble_evt(&m_sqs, p_ble_evt);    
}

/**
    @brief Callback to update adc value characteristic in database
*/
uint32_t sq_service_update_adc_characteristic(const uint16_t adc_value) {
    return sqs_update_adc_characteristic(&m_sqs, adc_value);
}

/**
    @brief Callback to update input value characteristic in database
*/
uint32_t sq_service_update_input_characteristic(uint8_t new_value) {
    return sqs_update_input_characteristic(&m_sqs, new_value);
}

/**
    @brief Callback to update rssi value characteristic in database
*/
uint32_t sq_service_update_rssi_value(const int8_t rssi_val) {
    uint8_t val = (uint8_t)rssi_val;
    return sqs_update_rssi_characteristic(&m_sqs, val);        
}

