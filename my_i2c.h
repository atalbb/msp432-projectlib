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

typedef struct{
    uint32_t i2c_num; // number for i2c0/1/3
    uint32_t clkSrc; //clk source to i2c block
    uint32_t clkHz; // clk going into i2c block
    uint32_t div; //clk divisor
    _S_I2C_PIN_INFO pin_info;// sda/scl pin info
    uint16_t intr; // interrupt enable bits
}_S_I2C_CONFIG;
void init_i2c(_S_I2C_CONFIG *i2c_config);
int8_t I2C_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);
int8_t I2C_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);


#endif /* I2C_H_ */
