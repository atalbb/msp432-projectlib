/*
 * i2c.h
 *
 *  Created on: Jun 15, 2017
 *      Author: Atal Bajracharya
 */

#ifndef __MY_I2C_H_
#define __MY_I2C_H_
#include "driverlib.h"

#define I2C_BUF_SIZE    16
typedef struct{
    uint32_t sda_scl_port;
    uint32_t sda_pin;
    uint32_t scl_pin;
    uint32_t func;
}_S_I2C_PIN_INFO;
//typedef struct{
//    volatile uint16_t *ctlw0;
//    uint16_t *ctlw1;
//    uint16_t *brw;
//    uint16_t *tbcnt;
//    uint16_t *rxbuf;
//    uint16_t *txbuf;
//    uint16_t *ie;
//    uint16_t *ifg;
//    uint16_t *iv;
//}_S_I2C_REGS;
typedef struct{
    uint32_t i2c_num; // number for i2c0/1/3
    uint32_t clkSrc; //clk source to i2c block
    uint32_t clkHz; // clk going into i2c block
    uint32_t div; //clk divisor
    _S_I2C_PIN_INFO pin_info;// sda/scl pin info
    uint16_t intr; // interrupt enable bits
    //_S_I2C_REGS regs;
}_S_I2C_CONFIG;
void init_i2c(_S_I2C_CONFIG *i2c_config);


#endif /* I2C_H_ */
