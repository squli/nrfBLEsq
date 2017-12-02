/**
    
*/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include <string.h>
#include "tps_service_handler.h"
#include "custom_board.h"

/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */

static ble_tps_t m_tps;   /**< Structure used to identify the custom (sq_) service. */


/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */

/**
    @brief init function of service: set event handler and default values
*/
uint32_t tps_service_init(void) {
    
    uint32_t err_code;
    
    ble_tps_init_t    tps_init;    
    memset(&tps_init, 0, sizeof(tps_init));

    tps_init.initial_tx_power_level = TX_POWER_LEVEL;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&tps_init.tps_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&tps_init.tps_attr_md.write_perm);

    err_code = ble_tps_init(&m_tps, &tps_init);
                
    return err_code;
}


/**
    @brief BLE-event handler of sq-service
*/
void tps_on_ble_evt(ble_evt_t * p_ble_evt) {
    ble_tps_on_ble_evt(&m_tps, p_ble_evt);    
}

