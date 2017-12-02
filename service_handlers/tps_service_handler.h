#ifndef __TPS_SERVICE_HANDLER__
#define __TPS_SERVICE_HANDLER__

#include "ble_tps.h"

uint32_t tps_service_init(void);
void tps_on_ble_evt(ble_evt_t * p_ble_evt);

#endif