/*
    File to define board specific constants
*/

#ifndef __CUSTOM_BOARD__
#define __CUSTOM_BOARD__

#ifdef __cplusplus
extern "C" {
#endif
    
/**< TX Power Level value. */
#define TX_POWER_LEVEL  (-8)  
    
    
#define ADC_INPUT_CHANNEL_NUM        NRF_SAADC_INPUT_AIN3    

/// Active state of led on board
#define LEDS_ACTIVE_STATE               1
/// Flag, that indicates using led
#define LED_INDICATE                    1
    
/// LED status stored in this bit in OUT_REG2    
#define LED_BIT_NUMBER               (1 << 4)
    
/// BUTTON status stored in this bit in INPUT_REG
#define BUT_SHORT_BIT_NUMBER               (1 << 0)
#define BUT_LONG_BIT_NUMBER                (1 << 1)

    
/// USED PIN NUMBERS, other pins defined in @ref out_reg1_pin_numbers and @ref out_reg2_pin_numbers
#define BUTTON_PIN_NUMBER               26
#define LED_PIN_NUMBER                  25

#define ADC_INPUT_HIGH_SIDE_PIN_NUMBER  24
#define ADC_INPUT_LOW_SIDE_PIN_NUMBER   19 
        
    
#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         5                                           /**< Size of timer operation queues. */
        // 1 - adc_timer
        // 2 - bat_notification_timer
        // 3 - led_timer
        // 4 - button timer
#define BATTERY_LEVEL_MEAS_INTERVAL       APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER) /**< Battery level measurement interval (ticks). This value corresponds to 120 seconds. */
    
// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#ifdef __cplusplus
}
#endif

#endif
