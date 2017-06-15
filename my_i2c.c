/*
 * i2c.c
 *
 *  Created on: Jun 15, 2017
 *      Author: Atal Bajracharya
 */

#include "my_i2c.h"

#include "driverlib.h"
void I2C_Init(void){
  // initialize eUSCI
  UCB3CTLW0 = 0x0001;                // hold the eUSCI module in reset mode
  // configure UCB1CTLW0 for:
  // bit15      UCA10 = 0; own address is 7-bit address
  // bit14      UCSLA10 = 0; address slave with 7-bit address
  // bit13      UCMM = 0; single master environment
  // bit12      reserved
  // bit11      UCMST = 1; master mode
  // bits10-9   UCMODEx = 3; I2C mode
  // bit8       UCSYNC = 1; synchronous mode
  // bits7-6    UCSSELx = 2; eUSCI clock SMCLK
  // bit5       UCTXACK = X; transmit ACK condition in slave mode
  // bit4       UCTR = X; transmitter/receiver
  // bit3       UCTXNACK = X; transmit negative acknowledge in slave mode
  // bit2       UCTXSTP = X; transmit stop condition in master mode
  // bit1       UCTXSTT = X; transmit start condition in master mode
  // bit0       UCSWRST = 1; reset enabled
  UCB0CTLW0 = 0x0F81;
  // configure UCB1CTLW1 for:
  // bits15-9   reserved
  // bit8       UCETXINT = X; early UCTXIFG0 in slave mode
  // bits7-6    UCCLTO = 3; timeout clock low after 165,000 SYSCLK cycles
  // bit5       UCSTPNACK = 0; send negative acknowledge before stop condition in master receiver mode
  // bit4       UCSWACK = 0; slave address acknowledge controlled by hardware
  // bits3-2    UCASTPx = 2; generate stop condition automatically after UCB1TBCNT bytes
  // bits1-0    UCGLITx = 0 deglitch time of 50 ns
  UCB0CTLW1 = 0x00C8;
  UCB0TBCNT = 2;                     // generate stop condition after this many bytes
  // set the baud rate for the eUSCI which gets its clock from SMCLK
  // Clock_Init48MHz() from ClockSystem.c sets SMCLK = HFXTCLK/4 = 12 MHz
  // if the SMCLK is set to 12 MHz, divide by 120 for 100 kHz baud clock
  UCB0BRW = 120;
// P6SEL0 |= 0xC0;                   //
// P6SEL1 &= ~0xC0;                   // configure P6.6 and P6.7 as primary module function
  P6SEL0 |= 0x30;
  P6SEL1 &= ~0x30;                   // configure P6.4 and P6.5 as primary module function
  UCB0CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB0IE = 0x0000;                   // disable interrupts
}


volatile uint16_t *CTLW0;
volatile uint16_t *CTLW1;
volatile uint16_t *BRW;
volatile uint16_t *TBCNT;
volatile uint16_t *RXCNT;
volatile uint16_t *TXBUF;
volatile uint16_t *IE;
volatile uint16_t *IFG;
volatile uint16_t *IV;

void init_i2c(_S_I2C_CONFIG *i2c_config){
    // initialize eUSCI
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(i2c_config->pin_info.sda_scl_port,
            i2c_config->pin_info.sda_pin | i2c_config->pin_info.scl_pin, i2c_config->pin_info.sda_scl_port);
    if(i2c_config->i2c_num == 0){
        CTLW0 = ((volatile uint16_t *)(0x40002000));
        CTLW1 = ((volatile uint16_t *)(0x40002002));
        BRW = ((volatile uint16_t *)(0x40002006));
        TBCNT = ((volatile uint16_t *)(0x4000200A));
        //RXCNT = ((volatile uint16_t *)(0x40002400));
        //TXBUF = ((volatile uint16_t *)(0x40002400));
        IE = ((volatile uint16_t *)(0x4000202A));
        //IFG = ((volatile uint16_t *)(0x40002400));
        //IV = ((volatile uint16_t *)(0x40002400));
    }else if(i2c_config->i2c_num == 1){
        CTLW0 = ((volatile uint16_t *)(0x40004C00));
        CTLW1 = ((volatile uint16_t *)(0x40002402));
        BRW = ((volatile uint16_t *)(0x40002406));
        TBCNT = ((volatile uint16_t *)(0x4000240A));
        //RXCNT = ((volatile uint16_t *)(0x40002400));
        //TXBUF = ((volatile uint16_t *)(0x40002400));
        IE = ((volatile uint16_t *)(0x4000242A));
        //IFG = ((volatile uint16_t *)(0x40002400));
        //IV = ((volatile uint16_t *)(0x40002400));
    }else if(i2c_config->i2c_num == 3){
        CTLW0 = ((volatile uint16_t *)(0x40002C00));
        CTLW1 = ((volatile uint16_t *)(0x4000Cc02));
        BRW = ((volatile uint16_t *)(0x40002C06));
        TBCNT = ((volatile uint16_t *)(0x40002C0A));
        //RXCNT = ((volatile uint16_t *)(0x40002400));
        //TXBUF = ((volatile uint16_t *)(0x40002400));
        IE = ((volatile uint16_t *)(0x40002C2A));
        //IFG = ((volatile uint16_t *)(0x40002400));
        //IV = ((volatile uint16_t *)(0x40002400));
    }
    *CTLW0 = 0;
    *CTLW0 = 0x0F1 | i2c_config->clkSrc;
    *CTLW1 = 0x00C8;
    *TBCNT = 2;
    *BRW = i2c_config->div;
//    P6SEL0 |= 0x30;
//    P6SEL1 &= ~0x30;                   // configure P6.4 and P6.5 as primary module function
    *CTLW0 = *CTLW0 & 0xFFFE;              // enable eUSCI module
    *IE = i2c_config->intr;                   // disable interrupts

}


