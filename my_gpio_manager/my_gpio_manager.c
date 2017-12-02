/**
    @brief This module used to manage all gpios, led and button
  
*/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
#include "my_gpio_manager.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "app_timer.h"
#include "sq_service_handler.h"

#define NRF_LOG_MODULE_NAME "GPIO"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

#define COUNT_OF_BITS_IN_BYTE 8U

/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */
#ifdef LED_INDICATE   
APP_TIMER_DEF(m_led_timer_id); /**< led blinking timer. */
#endif

APP_TIMER_DEF(m_button_timer_id); /**< button timer. */

/**
    @brief struct to describe curent state of gpios
*/
typedef struct {
    uint8_t output_reg1;
    uint8_t output_reg2;    
    uint8_t input_reg;          
} my_gpio_state_t;

/** @brief main struct with actual values of gpios
*/
my_gpio_state_t gpio_state;

/** @brief button short press flag to detect long press
*/
static bool button_short_pres_made_flag = false;

/** @brief Numbers of pins in gpio_reg1:
*/
uint8_t out_reg1_pin_numbers[COUNT_OF_BITS_IN_BYTE] = {28, 0x00, 0x00, 0x00,
                                                       0x00, 0x00, 0x00, 0x00 };
uint8_t out_reg2_pin_numbers[COUNT_OF_BITS_IN_BYTE] = {0x00, 0x00, 0x00, 0x00,
                                                       0x00, 0x00, 0x00, 0x00 };
                                                       
/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

static void button_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
static void led_timer_callback(void * p_context);
static void led_on(void);
static void led_off(void);
static bool get_led_state(void);

/** @brief Turn LED on
*/
static void led_on(void) {
    gpio_state.output_reg2 |= LED_BIT_NUMBER;
    nrf_gpio_pin_write(LED_PIN_NUMBER, LEDS_ACTIVE_STATE ? 1 : 0);
}

/** @brief Turn LED off
*/
static void led_off(void) {
    gpio_state.output_reg2 &= ~LED_BIT_NUMBER;
    nrf_gpio_pin_write(LED_PIN_NUMBER, LEDS_ACTIVE_STATE ? 0 : 1);    
}

/** @brief Return state of LED
    @return boolean value of led
*/
static bool get_led_state(void) {
    if ((LED_BIT_NUMBER & gpio_state.output_reg2) != 0)
        return true;
    else 
        return false;
}
/**
    @brief Handler of gpiote (button) event
*/
static void button_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    
    if (pin == BUTTON_PIN_NUMBER) {        
        /// if button pushed event
        if (action == GPIOTE_CONFIG_POLARITY_HiToLo) {
            
            NRF_LOG_INFO("button_event_handler()\r\n");
            nrf_drv_gpiote_in_event_disable(BUTTON_PIN_NUMBER);          
            
            /// start timer and after it will overrun, check button state
            uint32_t err_code = app_timer_start(m_button_timer_id, BUT_SHORT_INTERVAL, NULL);
            APP_ERROR_CHECK(err_code);
        }
    }
}

/** @brief Callback overrun of led timer
*/
static void led_timer_callback(void * p_context) {
    /// if led not active - start it
    if (get_led_state()) {
        led_off();
    } else {
        led_on();        
    }    
}

/** @brief Callback overrun of button timer
*/
static void buton_timer_callback(void * p_context) {
    
    /// check short press or long push
    if (button_short_pres_made_flag == false) {
        /// waiting short press
        if (nrf_gpio_pin_read(BUTTON_PIN_NUMBER) == 1) {
            /// short push detected
            button_short_pres_made_flag = true;
            uint32_t err_code = app_timer_stop(m_button_timer_id);
            err_code = app_timer_start(m_button_timer_id,
                                                BUT_LONG_INTERVAL, NULL);
        }
        else        
            nrf_drv_gpiote_in_event_enable(BUTTON_PIN_NUMBER, true);
    } else {
        button_short_pres_made_flag = false;
        /// start waiting of long push
        if (nrf_gpio_pin_read(BUTTON_PIN_NUMBER) == 1) {
            /// long push detected
            if ((gpio_state.input_reg & BUT_LONG_BIT_NUMBER) == 1)
                gpio_state.input_reg &= ~BUT_LONG_BIT_NUMBER;
            else
                gpio_state.input_reg |= BUT_LONG_BIT_NUMBER;
            
            gpio_state.input_reg &= ~BUT_SHORT_BIT_NUMBER;
            NRF_LOG_INFO("buton_timer_callback() - BUT_LONG\r\n");
        }
        else {
            /// short push detected            
            if ((gpio_state.input_reg & BUT_SHORT_BIT_NUMBER) == 1)
                gpio_state.input_reg &= ~BUT_SHORT_BIT_NUMBER;
            else
                gpio_state.input_reg |= BUT_SHORT_BIT_NUMBER;
            
            gpio_state.input_reg &= ~BUT_LONG_BIT_NUMBER;
            NRF_LOG_INFO("buton_timer_callback() - BUT_SHORT\r\n");            
        }
        /// update characteristic
        sq_service_update_input_characteristic(gpio_state.input_reg);
        
        nrf_drv_gpiote_in_event_enable(BUTTON_PIN_NUMBER, true);
    }    
}

/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */

/**
    @brief Init gpios, timer for led and gpiote module for button
*/
uint32_t my_gpio_init(void) {
   
    /// init led and adc outputs as simple gpio
    nrf_gpio_cfg_output(LED_PIN_NUMBER);
    
    nrf_gpio_cfg_output(ADC_INPUT_HIGH_SIDE_PIN_NUMBER);
    nrf_gpio_cfg_output(ADC_INPUT_LOW_SIDE_PIN_NUMBER);
    
    led_off();
    
    nrf_gpio_pin_write(ADC_INPUT_HIGH_SIDE_PIN_NUMBER, 1);
    nrf_gpio_pin_write(ADC_INPUT_LOW_SIDE_PIN_NUMBER, 0);
    
    /// init button as gpiote
    uint32_t err_code;
    if(!nrf_drv_gpiote_is_init())
    {   // init of gpiote driver
        err_code = nrf_drv_gpiote_init();
    }
    // init button as gpiote
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(BUTTON_PIN_NUMBER, &config, button_event_handler); 
    uint32_t ppi_event_addr = nrf_drv_gpiote_in_event_addr_get(BUTTON_PIN_NUMBER);
    nrf_drv_gpiote_in_event_enable(BUTTON_PIN_NUMBER, true);
    
    #ifdef LED_INDICATE    
    /// init timer to blinking led    
    err_code = app_timer_create(&m_led_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                led_timer_callback);   
    #endif
    
    // init buton timer
    err_code = app_timer_create(&m_button_timer_id,
                            APP_TIMER_MODE_SINGLE_SHOT,
                            buton_timer_callback); 
        
    //init gpio_reg1
    uint8_t i = 0;
    
    for (i = 0; i < COUNT_OF_BITS_IN_BYTE; i++) {
        nrf_gpio_cfg_output(out_reg1_pin_numbers[i]);
    }    
    return err_code;        
}

#ifdef LED_INDICATE
/** @brief Manage led blinking
    @param new_state[IN] - type of blinking
*/
uint32_t led_indicate_manage(const led_indication_state_t new_state) {
    uint32_t err_code;  
    
    switch (new_state) 
    {
        case NOT_INDICATION:
            app_timer_stop(m_led_timer_id);
            led_off();
            NRF_LOG_INFO("led_indicate_manage() - NOT_INDICATION\r\n");
            break;
        case FAST_BLINK_IND:            
            err_code = app_timer_start(m_led_timer_id, LED_FAST_INTERVAL, NULL);                                
            NRF_LOG_INFO("led_indicate_manage() - FAST_BLINK_IND\r\n");
            break;
        case SLOW_BLINK_IND:
            err_code = app_timer_start(m_led_timer_id, LED_SLOW_INTERVAL, NULL);                                
            NRF_LOG_INFO("led_indicate_manage() - SLOW_BLINK_IND\r\n");
            break;
        case ADVERTISING_IND:
            err_code = app_timer_start(m_led_timer_id, LED_ADVERTISING_INTERVAL, NULL);                                
            NRF_LOG_INFO("led_indicate_manage() - ADVERTISING_IND\r\n");
            break;
        case CONNECTED_IND:
            err_code = app_timer_start(m_led_timer_id, LED_CONNECTED_INTERVAL, NULL);                                
            NRF_LOG_INFO("led_indicate_manage() - CONNECTED_IND\r\n");
            break;
        default: break;
    }    
    
    return err_code;
    
}
#endif

/**
    @brief Event handler for checking gpio-events
*/
void my_gpio_on_ble_evt(ble_evt_t * p_ble_evt) {
}

/**
    @brief Callback for writing to out_registers
*/
void my_gpio_out_change_state (const gpio_out_regs_t reg_number, 
                               const uint8_t new_state) {
                                          
    uint8_t i = 0;

    if (GPIO_OUT_REG1 == reg_number) {                                  
        for (i = 0; i < COUNT_OF_BITS_IN_BYTE; i++) {
            if ( ((1 << i) & gpio_state.output_reg1) != ((1 << i) & new_state) ) {
                if (((1 << i) & new_state) == 1) {
                    gpio_state.output_reg1 |= (1 << i);
                    nrf_gpio_pin_write(out_reg1_pin_numbers[i], 1);
                    NRF_LOG_INFO("my_gpio_out_change_state %d to 1\r\n", out_reg1_pin_numbers[i]);
                } else {
                    gpio_state.output_reg1 &= ~(1 << i);
                    nrf_gpio_pin_write(out_reg1_pin_numbers[i], 0);
                    NRF_LOG_INFO("my_gpio_out_change_state %d to 0\r\n", out_reg1_pin_numbers[i]);
                }
            }                
        }
    } else if (GPIO_OUT_REG2 == reg_number) {
        /// TODO
        /// ...
    } else {
        /// WARNING
    }
}
