#ifndef __SQ_SERVICE__
#define __SQ_SERVICE__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_BASE_UUID_SQ_SERVICE     {(uint8_t)0x45, (uint8_t)0x56, (uint8_t)0x74, (uint8_t)0x46, \
                                      (uint8_t)0x0a, (uint8_t)0xbf, (uint8_t)0x48, (uint8_t)0x11, \
                                      (uint8_t)0x98, (uint8_t)0x32, (uint8_t)0x95, (uint8_t)0x2e, \
                                      (uint8_t)0x90, (uint8_t)0x8e, (uint8_t)0xbb, (uint8_t)0xcc};

#define BLE_UUID_SQ_SERVICE     (0x7446)     /*    https://www.uuidgenerator.net/ 
                                                45567446-0abf-4811-9832-952e908ebbcc
                                            */

/**
    @brief sq service event type. 
*/
typedef enum
{
    BLE_SQ_EVT_NOTIFICATION_ENABLED,                             /**< Battery value notification enabled event. */
    BLE_SQ_EVT_NOTIFICATION_DISABLED                             /**< Battery value notification disabled event. */
} ble_sq_evt_type_t;

/**
    @brief sq service event. 
*/
typedef struct
{
    ble_sq_evt_type_t evt_type;                                  /**< Type of event. */
} ble_sq_evt_t;

// Forward declaration of the ble_sq_t type.
typedef struct ble_sq_s ble_sq_t;

/**@brief sq service event handler type. */
typedef void (*ble_sq_evt_handler_t) (ble_sq_t * p_sq, ble_sq_evt_t * p_evt);

/**@brief sq service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_sq_evt_handler_t          evt_handler;                    /**< Event handler to be called for handling events in the Battery Service. */
    bool                          support_notification;           /**< TRUE if notification of measurements is supported. */
    ble_srv_report_ref_t *        p_report_ref;                   /**< If not NULL, a Report Reference descriptor with the specified value will be added to the Battery Level characteristic */
    uint8_t                       out_reg1_value;                 /**< Initial values of output registers */
    uint8_t                       out_reg2_value;                 /**< Initial values of output registers */
    uint8_t                       in_reg_value;                 /**< Initial values of output registers */
    uint8_t                       adc_reg_value;                 /**< Initial values of output registers */
    uint8_t                       rssi_reg_value;                 /**< Initial values of output registers */
    ble_srv_cccd_security_mode_t  sq_level_char_attr_md;     /**< Initial security level for sq characteristics attribute */
    ble_gap_conn_sec_mode_t       sq_level_report_read_perm; /**< Initial security level for sq report read attribute */
} ble_sq_init_t;

/**@brief sq-service structure. This contains various status information for the service. */
struct ble_sq_s
{
    ble_sq_evt_handler_t          evt_handler;                    /**< Event handler to be called for handling events in the Battery Service. */
    uint16_t                      service_handle;                 /**< Handle of Service (as provided by the BLE stack). */
    
    uint16_t                      report_ref_handle;              /**< Handle of the Report Reference descriptor. */

    ble_gatts_char_handles_t      sqs_reg_out1_handles;              /**< Handles related to the characteristics. */
    ble_gatts_char_handles_t      sqs_reg_out2_handles;              /**< Handles related to the characteristics. */
    ble_gatts_char_handles_t      sqs_reg_in_handles;              /**< Handles related to the characteristics. */
    ble_gatts_char_handles_t      sqs_adc_handles;              /**< Handles related to the characteristics. */
    ble_gatts_char_handles_t      sqs_rssi_handles;              /**< Handles related to the characteristics. */
        
    uint8_t                       reg_out1;                       /**< Last value of registers */
    uint8_t                       reg_out2;                       /**< Last value of registers */
    uint8_t                       reg_in;                         /**< Last value of registers */
    uint16_t                      reg_adc;                        /**< Last value of registers */
    uint8_t                       reg_rssi;                       /**< Last value of registers */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */    
};


uint32_t ble_sqs_init(ble_sq_t * p_sqs, ble_sq_init_t * p_sqs_init);

void ble_sqs_on_ble_evt(ble_sq_t * p_sqs, ble_evt_t * p_ble_evt);

uint32_t sqs_update_adc_characteristic(ble_sq_t * p_sqs, uint16_t adc_value);
uint32_t sqs_update_input_characteristic(ble_sq_t * p_sqs, uint8_t value);
uint32_t sqs_update_rssi_characteristic(ble_sq_t * p_sqs, uint8_t value);
#endif
