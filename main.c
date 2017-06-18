/*
 * -------------------------------------------
 *    MSP432 DriverLib - v3_21_00_05
 * -------------------------------------------
 *
 * --COPYRIGHT--,BSD,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*******************************************************************************
 * MSP432 ADC14 - Multiple Channel Sample without Repeat
 *
 * Description: In this code example, the feature of being able to scan multiple
 * ADC channels is demonstrated by the user a the DriverLib APIs.  Conversion
 * memory registers ADC_MEM0 - ADC_MEM7 are configured to read conversion
 * results from A0-A7 respectively. Conversion is enabled and then sampling is
 * toggled using a software toggle. Repeat mode is not enabled and sampling only
 * occurs once (and it is expected that the user pauses the debugger to observe
 * the results). Once the final sample has been taken, the interrupt for
 * ADC_MEM7 is triggered and the result is stored in the resultsBuffer buffer.
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST         P5.5  |<--- A0 (Analog Input)
 *            |            P5.4  |<--- A1 (Analog Input)
 *            |            P5.3  |<--- A2 (Analog Input)
 *            |            P5.2  |<--- A3 (Analog Input)
 *            |            P5.1  |<--- A4 (Analog Input)
 *            |            P5.0  |<--- A5 (Analog Input)
 *            |            P4.7  |<--- A6 (Analog Input)
 *            |            P4.6  |<--- A7 (Analog Input)
 *            |                  |
 *            |                  |
 *
 * Author: Timothy Logan
 ******************************************************************************/
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

#define CPU_CLK_MHZ     48
#define SYSTICK_TICK_MS         10
//#define ADC_VREF        (3.3)
//#define ADC_MAX_QUANT   (16384.0)
//#define ADC_CHANNELS    2
//#define TEMP_CH          0
//#define LIGHT_CH         1
#define ADC_SAMPLING_1MS     20
//#define TEMP_BUF_SIZE   100
//#define LDR_PULL_DOWN_OHM   240.0
static uint16_t resultsBuffer[ADC_CHANNELS];
static float analogResultsBuffer[ADC_CHANNELS];
typedef struct {
    float temperature_c;
    float temperature_f;
    float ldr_resistance;
}_S_WEATHER_DATA;
_S_WEATHER_DATA _weather_data = {0.0,0.0};
typedef enum{
    ADC_IDLE = 0,
    ADC_READY,
    ADC_CONV_FINISHED
}_E_ADC_STATE;
int adc_state = 0;
uint32_t SMCLKfreq;
uint32_t MCLKfreq;
uint32_t adcSamplingPeriod = ADC_SAMPLING_1MS;
void clockInit48MHzXTL(void) {  // sets the clock module to use the external 48 MHz crystal

    /* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    CS_setExternalClockSourceFrequency(32000,48000000); // enables getMCLK, getSMCLK to know externally set frequencies

    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false);  // false means that there are no timeouts set, will return when stable

    /* Initializing MCLK to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}
static void setSystickTimeMs(uint32_t mclkMHz, uint32_t ms){
    MAP_SysTick_setPeriod(mclkMHz * 1000 * ms);
}
static void displayStringH(uint32_t rowN, uint32_t colN,char * string, uint16_t tcolor, uint16_t bcolor, uint8_t size){
    uint8_t i = 0;
    for(i=0;string[i]!='\0';i++){
      ST7735_DrawCharS(colN * 8 + 6 * i * size, rowN * 6 * size, string[i], tcolor, bcolor, size);
    }
}

float normalizedADCRes = 0.0;
int main(void)
{
    /* Halting WDT and disabling master interrupts */
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT
    /* Selecting P1.0 as output (LED). */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
    GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    /* 48 Mhz Crystal Oscillator, MCLK = 48 Mhz, SMCLK = 12 Mhz */
    clk_init();
    debug_init();
    systick_init(48,1);
    adc_init();
    MAP_Interrupt_enableMaster();
    while(1){
        if(adc_state == 1){
            printf("%.2f\r\n",normalizedADCRes);
            adc_state = 2;
        }
    }
}

/* This interrupt is fired whenever a conversion is completed and placed in
 * ADC_MEM0 and ADC_MEM1. This signals the end of conversion and the results array is
 * grabbed and placed in resultsBuffer */
uint32_t curADCResult = 0;

void ADC14_IRQHandler(void)
{
    uint64_t status;
    static uint8_t i = 0;
    static float temp_buffer = 0.0;
    status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);
    if (ADC_INT0 & status)
    {
         curADCResult = MAP_ADC14_getResult(ADC_MEM0);
         normalizedADCRes = curADCResult * (ADC_VREF/ADC_MAX_QUANT)*1000;
         adc_state = 1;
    }
}


void SysTick_Handler(void)
{
    static uint32_t i = 0;
    if(adc_state == 2){
        if(++i >= adcSamplingPeriod){
            i = 0;
            MAP_ADC14_toggleConversionTrigger();
            adc_state = 0;
        }
    }
}

