/* DriverLib Includes */
#include "driverlib.h"
#include "adc.h"
#include "clk.h"
#include "debug.h"
#include "systick.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CPU_CLK_MHZ          48
#define ADC_SAMPLING_1MS     50

typedef enum{
    ADC_IDLE = 0,
    ADC_READY,
    ADC_CONV_FINISHED
}_E_ADC_STATE;

int adc_state = 0;
uint32_t SMCLKfreq;
uint32_t MCLKfreq;
uint32_t adcSamplingPeriod = ADC_SAMPLING_1MS;



float normalizedADCRes = 0.0;
uint32_t curADCResult = 0;

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
static uint32_t time = 0;
static uint32_t sample_count = 0;
int main(void)
{
    /* Halting WDT and disabling master interrupts */
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    /* 48 Mhz Crystal Oscillator, MCLK = 48 Mhz, SMCLK = 12 Mhz */
    clk_init();
    debug_init();
    systick_init(48,1);
    adc_init();
    led_init();
    MAP_Interrupt_enableMaster();
    while(1){
        if(adc_state == 1){
            sample_count++;
            //P1->OUT ^= 0x01;
           // printf("%.2f,\r\n",normalizedADCRes);
            printf("%d,\r\n",curADCResult);
            adc_state = 2;
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
         curADCResult = MAP_ADC14_getResult(ADC_MEM0);
//         sample_count++;
         //normalizedADCRes = curADCResult * (ADC_VREF/ADC_MAX_QUANT)*1000;
         adc_state = 1;
    }
}


void SysTick_Handler(void)
{
    static uint32_t i = 0;

    if(adc_state == 2){
        if(++i >= adcSamplingPeriod){
            i = 0;
            time += 50;
            if(time == 60000){
                i = 0;
            }
            MAP_ADC14_toggleConversionTrigger();
            adc_state = 0;
            P1->OUT ^= 0x01;
        }
    }
}
