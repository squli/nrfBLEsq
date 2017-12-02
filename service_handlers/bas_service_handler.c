

#include "ble_conn_params.h"
#include "bas_service_handler.h"
#include <string.h>
#include "app_timer.h"

#include "custom_board.h"


static ble_bas_t m_bas;                                   /**< Structure used to identify the battery service. */    
APP_TIMER_DEF(m_battery_timer_id);                        /**< Timer to send notifications of BAS service. */


/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t * p_evt);

/**@brief Function for handling the Battery Service events.
 *
 * @details This function will be called for all Battery Service events which are passed to the
 |          application.
 *
 * @param[in] p_bas  Battery Service structure.
 * @param[in] p_evt  Event received from the Battery Service.
 */
static void on_bas_evt(ble_bas_t * p_bas, ble_bas_evt_t * p_evt) {
    uint32_t err_code;

    switch (p_evt->evt_type)
    {
        case BLE_BAS_EVT_NOTIFICATION_ENABLED:
            // Start battery timer
            err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_BAS_EVT_NOTIFICATION_ENABLED

        case BLE_BAS_EVT_NOTIFICATION_DISABLED:
            err_code = app_timer_stop(m_battery_timer_id);
            APP_ERROR_CHECK(err_code);
            break; // BLE_BAS_EVT_NOTIFICATION_DISABLED

        default:
            // No implementation needed.
            break;
    }
}

/**
    @brief Init of battery service
*/
uint32_t bas_service_init() {
    
    uint32_t err_code;
    
    ble_bas_init_t bas_init_obj;

    memset(&bas_init_obj, 0, sizeof(bas_init_obj));

    bas_init_obj.evt_handler          = on_bas_evt;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref         = NULL;
    bas_init_obj.initial_batt_level   = 100;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init_obj.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_report_read_perm);

    err_code = ble_bas_init(&m_bas, &bas_init_obj);
    return err_code;       
}

void bas_on_ble_evt(ble_evt_t * p_ble_evt) {
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);  
}

/**
    @brief Callback to update battery level value characteristic in database
*/
uint32_t bas_battery_level_update(const uint8_t new_value_percent) {
    return ble_bas_battery_level_update(&m_bas, new_value_percent);
}
