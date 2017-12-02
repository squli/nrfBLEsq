#ifndef __SQ_SERVICE_HANDLER__
#define __SQ_SERVICE_HANDLER__

#include <stdint.h>
#include "sq_service.h"

/**
    @brief Function for initializing the sq service.
 */
 
uint32_t sq_service_init(void);

void sq_on_ble_evt(ble_evt_t * p_ble_evt);

uint32_t sq_service_update_adc_characteristic(const uint16_t adc_value);
uint32_t sq_service_update_input_characteristic(uint8_t new_value);
uint32_t sq_service_update_rssi_value(const int8_t rssi_val);
#endif
