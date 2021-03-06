/* DriverLib Includes */
#include "driverlib.h"
#include "adc.h"
#include "clk.h"
#include "debug.h"
#include "systick.h"
#include "RRAlgorithm.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CPU_CLK_MHZ          48
#define ADC_SAMPLING_1MS     RR_SAMPLE_TIME_MS
//#define CIRCULAR_BUFF_SIZE   500


uint8_t  g_adc_state = 0;
uint32_t SMCLKfreq;
uint32_t MCLKfreq;
uint32_t g_adcSamplingPeriod = ADC_SAMPLING_1MS;
_E_RR_STATE g_rr_state = RR_INITIAL;
double g_rr_buff[RR_BUF_SIZE]={0.0};
double g_rr_temp_buff[RR_BUF_SIZE]={0.0};
double g_rr_filter_values[RR_BUF_SIZE]={0.0};
//uint16_t g_circular_buff_ptr = 0;
uint16_t g_rr_sample_count = 0;
uint8_t g_rr_cal_signal = 0;


//uint32_t g_curADCResult = 0;

/* On board LED initialization Function
 * Blinking LED is used to verify sampling rate
 * */
static void led_init(){
    /* Selecting P1.0 as output (LED). */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
    GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
}
uint16_t calculate_RR(double *samples){
    double threshold= 0;
    int16_t peaks = 0;
    double avg = rr_find_mean(samples);
    diff_from_mean(samples,g_rr_temp_buff,avg);
    four_pt_MA(g_rr_temp_buff);
    //ButterworthLowpassFilter0040SixthOrder(g_rr_temp_buff,g_rr_filter_values,RR_BUF_SIZE-MA4_SIZE+1);
    ButterworthLowpassFilter0100SixthOrder(g_rr_temp_buff,g_rr_filter_values,RR_BUF_SIZE-MA4_SIZE+1);
    threshold = threshold_calc(g_rr_filter_values);
    printf("Threshold = %lf\r\n", threshold);
    peaks= myPeakCounter(g_rr_filter_values,RR_BUF_SIZE-MA4_SIZE+1,threshold);
    printf("Peaks = %d\r\n, ",peaks);
    return (60/RR_INITIAL_FRAME_TIME_S) * peaks;
    //return 3 * peaks;

}
int main(void)
{
    uint32_t i = 0;
    uint16_t respiratory_rate = 0;
    /* Halting WDT and disabling master interrupts */
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    /* 48 Mhz Crystal Oscillator, MCLK = 48 Mhz, SMCLK = 12 Mhz */
    clk_init();
    debug_init();
    systick_init(48,1);
    adc_init();
    led_init();
    MAP_Interrupt_enableMaster();
    printf("Program started!\r\n");
    while(1){
        if(g_adc_state == 1){
            //printf("%d,\r\n",g_curADCResult);
            if(g_rr_cal_signal == 1){ // calculate Respiratory Rate
                g_rr_cal_signal = 0;
                /* Have to calculate RR*/
                respiratory_rate = calculate_RR(g_rr_buff);
                printf("Br is %d\r\n",respiratory_rate);
                //dumping the first X sets of samples and shift the last RR_BUF_SIZE-X sets of samples to the top
                for(i=RR_STABLE_BUF_SIZE;i<RR_BUF_SIZE;i++){
                    g_rr_buff[i-RR_STABLE_BUF_SIZE]=g_rr_buff[i];
                }
            }
            g_adc_state = 2;
        }
    }
}


void ADC14_IRQHandler(void)
{
    uint64_t status;
    status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);
    if (ADC_INT0 & status)
    {
        g_rr_buff[g_rr_sample_count++]= (MAP_ADC14_getResult(ADC_MEM0)* (3300.0/16383.0));
        g_adc_state = 1;
        switch(g_rr_state){
            case RR_INITIAL:
                if(g_rr_sample_count == RR_BUF_SIZE){
                    g_rr_sample_count = RR_BUF_SIZE - RR_STABLE_BUF_SIZE;
                    g_rr_state = RR_STABLE;
                    g_rr_cal_signal = 1;
                }
                break;
            case RR_STABLE:
                if(g_rr_sample_count == RR_BUF_SIZE){
                    g_rr_sample_count = RR_BUF_SIZE - RR_STABLE_BUF_SIZE;
                    g_rr_cal_signal = 1;
                }
                break;

            default:

                break;

        }



    }
}


void SysTick_Handler(void)
{
    static uint32_t i = 0;
    if(g_adc_state == 2){
        if(++i >= g_adcSamplingPeriod){
            i = 0;
            MAP_ADC14_toggleConversionTrigger();
            P1->OUT ^= 0x01;
            g_adc_state = 0;
        }
    }
}
