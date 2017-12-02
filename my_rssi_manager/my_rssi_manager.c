/**
    @brief This module used to measure rssi. This measuring starts
    in main.c, after every connect event and stops after disconnect.
    Every change of rssi value begin RSSI_CHANGED_EVENT. 
    This module is the handler for this event.
  
*/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
#include "my_rssi_manager.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define RSSI_BUFFER_SIZE    64U

/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */
static int8_t rssi_buffer[RSSI_BUFFER_SIZE] = {0x00};
static uint8_t current_offset = 0x00;

/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

static int8_t process_buffer(void);

static int8_t process_buffer(void) {
    
    int32_t sum = 0x00;
    uint8_t count_of_non_zero_values = 0x00;
    
    for (uint8_t i = 0; i < RSSI_BUFFER_SIZE; i++) {
        sum += rssi_buffer[i];
        if (rssi_buffer[i] != 0)
            count_of_non_zero_values += 1;
    }
    return sum / count_of_non_zero_values;    
}

/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */

/**
    @brief return average value of rssi buffer
*/
int8_t my_rssi_get_value(void) {
    return process_buffer();
}

/**
* @brief Add new value to rsi values buffer
*/
void my_rssi_push_value(const int8_t new_value) {
    rssi_buffer[0] = new_value;
    *(rssi_buffer + current_offset) = new_value;
    current_offset += 1;
    if (current_offset == RSSI_BUFFER_SIZE)
        current_offset = 0;
}
