#ifndef __MY_RSSI_MANAGER__
#define __MY_RSSI_MANAGER__

#include "stdint.h"

int8_t my_rssi_get_value(void);
void my_rssi_push_value(const int8_t new_value);

#endif
