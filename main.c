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
#include "algorithm.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CPU_CLK_MHZ     48
#define SYSTICK_TICK_MS         10
#define ADC_SAMPLING_1MS     40

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


//if Adafruit Flora development board is chosen, include NeoPixel library and define an NeoPixel object
#if defined(ARDUINO_AVR_FLORA8)
#include "adafruit_neopixel.h"
#define BRIGHTNESS_DIVISOR 8  //to lower the max brightness of the neopixel LED
Adafruit_NeoPixel LED = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);
#endif

#define MAX_BRIGHTNESS 255

#if defined(ARDUINO_AVR_UNO)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated.  Samples become 16-bit data.
uint16_t aun_ir_buffer[100]; //infrared LED sensor data
uint16_t aun_red_buffer[100];  //red LED sensor data
#else
uint32_t aun_ir_buffer[100]; //infrared LED sensor data
uint32_t aun_red_buffer[100];  //red LED sensor data
#endif
int32_t n_ir_buffer_length; //data length
int32_t n_spo2;  //SPO2 value
int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
int32_t n_heart_rate; //heart rate value
int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;

uint8_t adc_conv_data(uint32_t *value1, uint32_t *value2){
    while(MAP_ADC14_isBusy()==true);
    MAP_ADC14_toggleConversionTrigger();
    while(MAP_ADC14_isBusy()==true);
    *value2 = *value1 = curADCResult = MAP_ADC14_getResult(ADC_MEM0);
    return 1;
}
int main(void)
{
    int i = 0;
    uint32_t un_min, un_max, un_prev_data, un_brightness;  //variables to calculate the on-board LED brightness that reflects the heartbeats
    int32_t prev_n_heart_rate=0;
    float f_temp;
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

    un_brightness=0;
    un_min=0x3FFFF;
    un_max=0;
    n_ir_buffer_length=BUFFER_SIZE;  //buffer length of 100 stores 4 seconds of samples running at 25sps

    //read the first 100 samples(4sec), and determine the signal range
      for(i=0;i<n_ir_buffer_length;i++)
      {
        //while(digitalRead(10)==1);  //wait until the interrupt pin asserts
        //maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));  //read from MAX30102 FIFO
        while(adc_conv_data(&aun_red_buffer[i],&aun_ir_buffer[i])!=1);
        if(un_min>aun_red_buffer[i])
          un_min=aun_red_buffer[i];  //update signal min
        if(un_max<aun_red_buffer[i])
          un_max=aun_red_buffer[i];  //update signal max
         printf("red=");
         printf("%i", aun_red_buffer[i]);
         printf(", ir=");
         printf("%i\n\r", aun_ir_buffer[i]);
         __delay_cycles(64000);//40ms
      }
      un_prev_data=aun_red_buffer[i-1];
      //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
      maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
      //send samples and calculation result to terminal program through UART

      while(1){
          i=0;
          un_min=0x3FFFF;
          un_max=0;

          //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
          for(i=25;i<100;i++)
          {
            aun_red_buffer[i-25]=aun_red_buffer[i];
            aun_ir_buffer[i-25]=aun_ir_buffer[i];

            //update the signal min and max
            if(un_min>aun_red_buffer[i])
              un_min=aun_red_buffer[i];
            if(un_max<aun_red_buffer[i])
              un_max=aun_red_buffer[i];
          }

          //take 25 sets of samples before calculating the heart rate.
          for(i=75;i<100;i++)
          {
            un_prev_data=aun_red_buffer[i-1];
            while(adc_conv_data(&aun_red_buffer[i],&aun_ir_buffer[i])!=1);
            __delay_cycles(64000);//40ms
  #if defined(TARGET_KL25Z) || defined(TARGET_MAX32600MBED)
              led.write(1-(float)n_brightness/256);
  #endif
              //send samples and calculation result to terminal program through UART
//              printf("red=");
//              printf("%i", aun_red_buffer[i]);
//              printf(", ir=");
//              printf("%i", aun_ir_buffer[i]);
//              printf(", HR=%i, ", n_heart_rate);
//              printf("HRvalid=%i, ", ch_hr_valid);
//              printf("SpO2=%i, ", n_spo2);
//              printf("SPO2Valid=%i\n\r", ch_spo2_valid);
          }
          maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
//        if(!ch_hr_valid){
//            n_heart_rate = prev_n_heart_rate;
//        }
         if(ch_hr_valid){
             printf("red=");
             printf("%i", aun_red_buffer[i-1]);
             printf(", ir=");
             printf("%i", aun_ir_buffer[i-1]);
             printf(", HR=%i, ", n_heart_rate);
             printf("HRvalid=%i, ", ch_hr_valid);
             printf("SpO2=%i, ", n_spo2);
             printf("SPO2Valid=%i\n\r", ch_spo2_valid);
         }
      }

}




void SysTick_Handler(void)
{
    static uint32_t i = 0;
    if(adc_state == 2){
        if(++i >= adcSamplingPeriod){
            i = 0;
            //MAP_ADC14_toggleConversionTrigger();
            adc_state = 0;
        }
    }
}
