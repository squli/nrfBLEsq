/*!
    @brief Module for manage of SAADC in nrf52. 
           It uses timer for checking all adc channels after timeout.
           In simple case there is only one channel for battery measurment service.           
    
*/


#ifndef __MY_ADC_MANAGER__
#define __MY_ADC_MANAGER__

#include "app_timer.h"


uint32_t my_adc_timer_init(void);

uint32_t my_adc_timer_start(void);

void adc_configure(void);

#endif
