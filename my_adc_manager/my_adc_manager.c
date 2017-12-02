/**
    @brief This module used to manage the ADC measurments
  
*/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "my_adc_manager.h"
#include "nrf_drv_saadc.h"
#include "custom_board.h"
#include "sq_service_handler.h"
#include "bas_service_handler.h"

#define NRF_LOG_MODULE_NAME "ADC"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

#define USED_ADC_CHANNELS   2   /**< Count of used adc chanels*/

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS     600                                          /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION      6                                            /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
/**< Typical forward voltage drop of the diode (Part no: SD103ATW-7-F) 
that is connected in series with the voltage supply. 
This is the voltage drop when the forward current is 1mA. 
Source: Data sheet of 'SURFACE MOUNT SCHOTTKY BARRIER DIODE ARRAY' available at www.diodes.com. */
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS    0 //270  - not used diode
#define ADC_REF_VBG_VOLTAGE_IN_MILLIVOLTS 1200                                         /**< Value in millivolts for voltage used as reference in ADC conversion on NRF51. */
#define ADC_INPUT_PRESCALER               3                                            /**< Input prescaler for ADC convestion on NRF51. */
#define ADC_RES_10BIT                     1024                                         /**< Maximum digital value for 10-bit ADC conversion. */

#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)


#define ADC_MEAS_INTERVAL   APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)       /**< ADC measurement interval (ticks). This value corresponds to 1000 ms. */


/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */

APP_TIMER_DEF(m_adc_timer_id);     /**< ADC measurement timer. */

/// Buffers for store adc values, two neds to make continious sampling if neded
nrf_saadc_value_t adc_buf_one[USED_ADC_CHANNELS] = {0x00};
nrf_saadc_value_t adc_buf_two[USED_ADC_CHANNELS] = {0x00};

/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

static void adc_meas_timeout_handler(void * p_context);
static void saadc_event_handler(nrf_drv_saadc_evt_t const * p_event);

/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *          This function will start the ADC.
 *
 * @param[in] p_context   Pointer used for passing some arbitrary information (context) from the
 *                        app_start_timer() call to the timeout handler.
 */
static void adc_meas_timeout_handler(void * p_context)
{
    NRF_LOG_INFO("adc_meas_timeout_handler\r\n");
    
    UNUSED_PARAMETER(p_context);
    uint32_t err_code;
    err_code = nrf_drv_saadc_sample();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the ADC interrupt.
 *
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */
static void saadc_event_handler(nrf_drv_saadc_evt_t const * p_event) {
    
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {       
        nrf_saadc_value_t adc_result, adc_result2;
        uint16_t          batt_lvl_in_milli_volts, adc_lvl_in_milli_volts;
        uint8_t           percentage_batt_lvl;
        uint32_t          err_code;

        adc_result = p_event->data.done.p_buffer[0];

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, USED_ADC_CHANNELS);
        APP_ERROR_CHECK(err_code);

        batt_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result) +
                                  DIODE_FWD_VOLT_DROP_MILLIVOLTS;
                       
        percentage_batt_lvl = battery_level_in_percent(batt_lvl_in_milli_volts);

        err_code = bas_battery_level_update(percentage_batt_lvl);
        
        if (
            (err_code != NRF_SUCCESS)
            &&
            (err_code != NRF_ERROR_INVALID_STATE)
           )
        {
            APP_ERROR_HANDLER(err_code);
        }
        
        adc_result2 = p_event->data.done.p_buffer[1];

        adc_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result2) +
                                  DIODE_FWD_VOLT_DROP_MILLIVOLTS;
        
        NRF_LOG_INFO("batt_lvl_in_milli_volts = %d\r\n", batt_lvl_in_milli_volts);
        NRF_LOG_INFO("adc_lvl_in_milli_volts = %d\r\n", adc_lvl_in_milli_volts);
        
        err_code = sq_service_update_adc_characteristic(adc_lvl_in_milli_volts);
        //if (
        //    (err_code != NRF_SUCCESS)
        //    &&
        //    (err_code != NRF_ERROR_INVALID_STATE)
        //    //&&
        //    //(err_code != BLE_ERROR_NO_TX_PACKETS)
        //    //&&
        //    //(err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        //   )
        //{
        //    APP_ERROR_HANDLER(err_code);
        //}
        
    }
}

/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */

/** @brief Init ADC timer 
  *
  */
uint32_t my_adc_timer_init(void)
{
    uint32_t err_code;
    
    // Create ADC timer.
    err_code = app_timer_create(&m_adc_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                adc_meas_timeout_handler);
    return err_code;
}

/** @brief Run ADC timer 
  *
  */
uint32_t my_adc_timer_start(void)
{
    uint32_t err_code;    
    err_code = app_timer_start(m_adc_timer_id, ADC_MEAS_INTERVAL, NULL);            
    return err_code;
}

/**
    @brief Function for configuring ADC
 */
void adc_configure(void) {
       
    ret_code_t err_code = nrf_drv_saadc_init(NULL, saadc_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_saadc_channel_config_t config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);
    err_code = nrf_drv_saadc_channel_init(0, &config);
    APP_ERROR_CHECK(err_code);
    
    nrf_saadc_channel_config_t config2 = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(ADC_INPUT_CHANNEL_NUM);
    err_code = nrf_drv_saadc_channel_init(1, &config2);
    APP_ERROR_CHECK(err_code);
        
    err_code = nrf_drv_saadc_buffer_convert(&adc_buf_one[0], USED_ADC_CHANNELS);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(&adc_buf_two[0], USED_ADC_CHANNELS);
    APP_ERROR_CHECK(err_code);
}
