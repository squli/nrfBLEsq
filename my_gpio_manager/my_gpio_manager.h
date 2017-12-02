#ifndef __GPIO_MANAGER__
#define __GPIO_MANAGER__

#include <stdint.h>
#include "custom_board.h"
#include "ble.h"

#ifdef LED_INDICATE  

#define LED_FAST_INTERVAL           APP_TIMER_TICKS(250, APP_TIMER_PRESCALER)
#define LED_SLOW_INTERVAL           APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)
#define LED_ADVERTISING_INTERVAL    APP_TIMER_TICKS(500, APP_TIMER_PRESCALER)
#define LED_CONNECTED_INTERVAL      APP_TIMER_TICKS(750, APP_TIMER_PRESCALER)

// time to detecting long and short pushes of button
#define BUT_SHORT_INTERVAL          APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)
#define BUT_LONG_INTERVAL           APP_TIMER_TICKS(250, APP_TIMER_PRESCALER)

/**
    @brief Possible led states for indication processes
*/
typedef enum {
    
    NOT_INDICATION  = 0,
    FAST_BLINK_IND  = 1,
    SLOW_BLINK_IND  = 2,
    ADVERTISING_IND = 3,
    CONNECTED_IND   = 4,
    
} led_indication_state_t;

typedef enum {
    GPIO_OUT_REG1 = 0,
    GPIO_OUT_REG2 = 1,
} gpio_out_regs_t;


uint32_t my_gpio_init(void);

uint32_t led_indicate_manage(const led_indication_state_t new_state); 

void my_gpio_on_ble_evt(ble_evt_t * p_ble_evt);

void my_gpio_out_change_state (const gpio_out_regs_t reg_number, 
                               const uint8_t new_state);

#endif





#endif
