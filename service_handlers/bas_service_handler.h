#ifndef __BAS_SERVICE_HANDLER__
#define __BAS_SERVICE_HANDLER__

#include "ble_bas.h"

uint32_t bas_service_init(void);
void bas_on_ble_evt(ble_evt_t * p_ble_evt);
uint32_t bas_battery_level_update(const uint8_t new_value_percent);
#endif
