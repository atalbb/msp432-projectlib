/*
 * i2c.c
 *
 *  Created on: Jun 15, 2017
 *      Author: Atal Bajracharya
 */

#include "my_i2c.h"

#include "driverlib.h"

volatile uint16_t *CTLW0;
volatile uint16_t *CTLW1;
volatile uint16_t *BRW;
volatile uint16_t *TBCNT;
volatile uint16_t *RXCNT;
volatile uint16_t *TXBUF;
volatile uint16_t *IE;
volatile uint16_t *IFG;
volatile uint16_t *IV;
volatile uint16_t *STATW;
volatile uint16_t *I2CSA;
volatile uint16_t *RXBUF;

void init_i2c(_S_I2C_CONFIG *i2c_config){
    // initialize eUSCI
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(i2c_config->pin_info.sda_scl_port,
            i2c_config->pin_info.sda_pin | i2c_config->pin_info.scl_pin, i2c_config->pin_info.func);
    if(i2c_config->i2c_num == 0){
        CTLW0 = ((volatile uint16_t *)(0x40002000));
        CTLW1 = ((volatile uint16_t *)(0x40002002));
        BRW = ((volatile uint16_t *)(0x40002006));
        TBCNT = ((volatile uint16_t *)(0x4000200A));
        STATW = ((volatile uint16_t *)(0x40002008));
        I2CSA = ((volatile uint16_t *)(0x40002020));
        TXBUF = ((volatile uint16_t *)(0x4000200E));
        IE = ((volatile uint16_t *)(0x4000202A));
        IFG = ((volatile uint16_t *)(0x4000202C));
        RXBUF = ((volatile uint16_t *)(0x4000200C));
    }else if(i2c_config->i2c_num == 1){
        CTLW0 = ((volatile uint16_t *)(0x40002400));
        CTLW1 = ((volatile uint16_t *)(0x40002402));
        BRW = ((volatile uint16_t *)(0x40002406));
        TBCNT = ((volatile uint16_t *)(0x4000240A));
        STATW = ((volatile uint16_t *)(0x40002408));
        I2CSA = ((volatile uint16_t *)(0x40002420));
        RXBUF = ((volatile uint16_t *)(0x4000240C));
        TXBUF = ((volatile uint16_t *)(0x4000240E));
        IE = ((volatile uint16_t *)(0x4000242A));
        IFG = ((volatile uint16_t *)(0x4000242C));
    }else if(i2c_config->i2c_num == 3){
        CTLW0 = ((volatile uint16_t *)(0x40002C00));
        CTLW1 = ((volatile uint16_t *)(0x40002C02));
        BRW = ((volatile uint16_t *)(0x40002C06));
        TBCNT = ((volatile uint16_t *)(0x40002C0A));
        STATW = ((volatile uint16_t *)(0x40002C08));
        I2CSA = ((volatile uint16_t *)(0x40002C20));
        TXBUF = ((volatile uint16_t *)(0x40002C0E));
        RXBUF = ((volatile uint16_t *)(0x40002C0C));
        IE = ((volatile uint16_t *)(0x40002C2A));
        IFG = ((volatile uint16_t *)(0x40002C2C));
    }
    *CTLW0 = 0x0000;
    *CTLW0 = 0x0F01 | i2c_config->clkSrc;
    *CTLW1 = 0x00C8;
    *TBCNT = 2;
    *BRW = i2c_config->div;
    *CTLW0 = *CTLW0 & 0xFFFE;              // enable eUSCI module
    *IE = i2c_config->intr;                   // disable interrupts

}
/*-------------------------------------------------------------------*
*   This is a sample code for read and write the data by using I2C/SPI
*   Use either I2C or SPI based on your need
*   The device address defined in the bme280.h file
*-----------------------------------------------------------------------*/
 /* \Brief: The function is used as I2C bus write
 *  \Return : Status of the I2C write
 *  \param dev_addr : The device address of the sensor
 *  \param reg_addr : Address of the first register, will data is going to be written
 *  \param reg_data : It is a value hold in the array,
 *      will be used for write the value into the register
 *  \param cnt : The no of byte of data to be write
 */
int8_t I2C_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    int8_t iError = 0;
    uint8_t array[I2C_BUF_SIZE]={0};
    uint8_t stringpos = 0;
    uint8_t i;
    //uint16_t rtnval, debugdump;
    array[0] = reg_addr;

    for (stringpos = 0; stringpos < cnt; stringpos++) {
        array[stringpos + 1] = *(reg_data + stringpos);
    }
    /*
    * Please take the below function as your reference for
    * write the data using I2C communication
    * "IERROR = I2C_WRITE_STRING(DEV_ADDR, array, cnt+1)"
    * add your I2C write function here
    * iError is an return value of I2C read function
    * Please select your valid return value
    * In the driver SUCCESS defined as 0
    * and FAILURE defined as -1
    * Note :
    * This is a full duplex operation,
    * The first read data is discarded, for that extra write operation
    * have to be initiated. For that cnt+1 operation done in the I2C write string function
    * For more information please refer data sheet SPI communication:
    */

    while((*STATW)&0x0010){};         // wait for I2C ready
    *CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
    *TBCNT = cnt+1;                     // generate stop condition after this many bytes
    *CTLW0 &= ~0x0001;              // enable eUSCI module
    *I2CSA = dev_addr;              // I2CCSA[6:0] is slave address
    *CTLW0 = ((*CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                       // set bit1 (UCTXSTT) for transmit start condition
                  | 0x0012);           // set bit4 (UCTR) for transmit mode
    while((*IFG&0x0002) == 0){};    // wait for slave address sent

    for(i=0; i<cnt; i++) {
        *TXBUF = array[i]&0xFF;         // TXBUF[7:0] is data
        while((*IFG&0x0002) == 0){      // wait for data sent
            if(*IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
                //debugdump = UCB1IFG;           // snapshot flag register for calling program
                //I2C_Init();                    // reset to known state
                return iError=-1;
            }
        }
    }
    *TXBUF = array[i]&0xFF;         // TXBUF[7:0] is last data
    while(*STATW&0x0010){           // wait for I2C idle
      if(*IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
        //debugdump = UCB1IFG;           // snapshot flag register for calling program
        //I2C_Init();                    // reset to known state
        return iError=-1;
      }
    }
    return iError=0;
}

 /* \Brief: The function is used as I2C bus read
 *  \Return : Status of the I2C read
 *  \param dev_addr : The device address of the sensor
 *  \param reg_addr : Address of the first register, will data is going to be read
 *  \param reg_data : This data read from the sensor, which is hold in an array
 *  \param cnt : The no of data byte of to be read
 */
int8_t I2C_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    int8_t iError = 0;
    uint8_t i;
    //uint16_t rtnval, debugdump;
    uint8_t array[I2C_BUF_SIZE] = {0};
    //uint8_t stringpos = 0;
    array[0] = reg_addr;
    /* Please take the below function as your reference
     * for read the data using I2C communication
     * add your I2C read function here.
     * "IERROR = I2C_WRITE_READ_STRING(DEV_ADDR, ARRAY, ARRAY, 1, CNT)"
     * iError is an return value of write function
     * Please select your valid return value
     * In the driver SUCCESS defined as 0
     * and FAILURE defined as -1
     */

    // set pointer to register address
    while(*STATW&0x0010){};         // wait for I2C ready
    *CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
    *TBCNT = 1;                     // generate stop condition after this many bytes
    *CTLW0 &= ~0x0001;              // enable eUSCI module
    *I2CSA = dev_addr;                 // I2CCSA[6:0] is slave address
    *CTLW0 = ((*CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                       // set bit1 (UCTXSTT) for transmit start condition
                  | 0x0012);           // set bit4 (UCTR) for transmit mode
    while(*CTLW0&0x0002){};         // wait for slave address sent
    *TXBUF = reg_addr&0xFF;            // TXBUF[7:0] is data
    while(*STATW&0x0010){           // wait for I2C idle
      if(*IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
        //debugdump = UCB1IFG;           // snapshot flag register for calling program
        //I2C_Init();                    // reset to known state
        return iError=-1;
      }
    }
    // receive bytes from registers on BME280 device

    while(*STATW&0x0010){};         // wait for I2C ready
    *CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
    *TBCNT = cnt;                     // generate stop condition after this many bytes
    *CTLW0 &= ~0x0001;              // enable eUSCI module
    *I2CSA = dev_addr;                 // I2CCSA[6:0] is slave address
    *CTLW0 = ((*CTLW0&~0x0014)   // clear bit4 (UCTR) for receive mode
                                       // clear bit2 (UCTXSTP) for no transmit stop condition
                  | 0x0002);           // set bit1 (UCTXSTT) for transmit start condition
    for(i=0; i<cnt; i++) {
        while((*IFG&0x0001) == 0){      // wait for complete character received
          if(*IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
            //I2C_Init();                    // reset to known state
            return -1;
          }
        }
        *reg_data++= *RXBUF&0xFF;            // get the reply
    }

    return iError;
}


