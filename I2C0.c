// I2C0.c
// Runs on MSP432
// Provide a function that initializes, sends, and receives
// the eUSCI1B module interfaced with A BMP180 pressure sensor
//
// Daniel Valvano
// October 12, 2015
//
// modified by RWB 2/26/2016

/*
 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include "msp.h"

void I2C_Init(void){
  // initialize eUSCI
  UCB1CTLW0 = 0x0001;                // hold the eUSCI module in reset mode
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
  UCB1CTLW0 = 0x0F81;
  // configure UCB1CTLW1 for:
  // bits15-9   reserved
  // bit8       UCETXINT = X; early UCTXIFG0 in slave mode
  // bits7-6    UCCLTO = 3; timeout clock low after 165,000 SYSCLK cycles
  // bit5       UCSTPNACK = 0; send negative acknowledge before stop condition in master receiver mode
  // bit4       UCSWACK = 0; slave address acknowledge controlled by hardware
  // bits3-2    UCASTPx = 2; generate stop condition automatically after UCB1TBCNT bytes
  // bits1-0    UCGLITx = 0 deglitch time of 50 ns
  UCB1CTLW1 = 0x00C8;
  UCB1TBCNT = 2;                     // generate stop condition after this many bytes
  // set the baud rate for the eUSCI which gets its clock from SMCLK
  // Clock_Init48MHz() from ClockSystem.c sets SMCLK = HFXTCLK/4 = 12 MHz
  // if the SMCLK is set to 12 MHz, divide by 120 for 100 kHz baud clock
  UCB1BRW = 120;
// P6SEL0 |= 0xC0;                   //
// P6SEL1 &= ~0xC0;                   // configure P6.6 and P6.7 as primary module function
  P6SEL0 |= 0x30;
  P6SEL1 &= ~0x30;                   // configure P6.4 and P6.5 as primary module function
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1IE = 0x0000;                   // disable interrupts
}

// receives one byte from specified slave
// Used to read the contents of a one byte register

uint8_t I2C_Recv(int8_t slave){
  int8_t data1;
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 1;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0014)   // clear bit4 (UCTR) for receive mode
                                     // clear bit2 (UCTXSTP) for no transmit stop condition
                | 0x0002);           // set bit1 (UCTXSTT) for transmit start condition
  while((UCB1IFG&0x0001) == 0){      // wait for complete character received
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      I2C_Init();                    // reset to known state
      return 0xFF;
    }
  }
  data1 = UCB1RXBUF&0xFF;            // get the reply
  return data1;
}

// receives two bytes from specified slave
// Used to read the contents of a register

uint16_t I2C_Recv2(int8_t slave){
  uint8_t data1, data2;
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 2;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0014)   // clear bit4 (UCTR) for receive mode
                                     // clear bit2 (UCTXSTP) for no transmit stop condition
                | 0x0002);           // set bit1 (UCTXSTT) for transmit start condition
  while((UCB1IFG&0x0001) == 0){      // wait for complete character received
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      I2C_Init();                    // reset to known state
      return 0xFFFF;
    }
  }
  data1 = UCB1RXBUF&0xFF;            // get the reply
  while((UCB1IFG&0x0001) == 0){      // wait for complete character received
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      I2C_Init();                    // reset to known state
      return 0xFFFF;
    }
  }
  data2 = UCB1RXBUF&0xFF;            // get the reply
//  return (data1<<8)+data2;  // this got reversed in order from data in the BMP180 PROM! ???
  return (data2<<8)+data1;
}

// sends one byte to specified slave
// Used to change the pointer register
// Returns 0 if successful, nonzero if error

uint16_t I2C_Send1(int8_t slave, uint8_t data1){
  uint16_t debugdump;                // save status register here in case of error
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 1;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while(UCB1CTLW0&0x0002){};         // wait for slave address sent
  UCB1TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while(UCB1STATW&0x0010){           // wait for I2C idle
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return debugdump;
    }
  }
  return 0;
}

// sends two bytes to specified slave
// Returns 0 if successful, nonzero if error

uint16_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2){
  uint16_t debugdump;                // save status register here in case of error
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 2;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while(UCB1CTLW0&0x0002){};         // wait for slave address sent
  UCB1TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while((UCB1IFG&0x0002) == 0){      // wait for first data sent
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return debugdump;
    }
  }
  UCB1TXBUF = data2&0xFF;            // TXBUF[7:0] is data
  while(UCB1STATW&0x0010){           // wait for I2C idle
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return debugdump;
    }
  }
  return 0;
}

// sends three bytes to specified slave
// Returns 0 if successful, nonzero if error

uint16_t I2C_Send3(int8_t slave, uint8_t data1, uint8_t data2, uint8_t data3){
  uint16_t debugdump;                // save status register here in case of error
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 3;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while((UCB1IFG&0x0002) == 0){};    // wait for slave address sent
  UCB1TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while((UCB1IFG&0x0002) == 0){      // wait for first data sent
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return debugdump;
    }
  }
  UCB1TXBUF = data2&0xFF;            // TXBUF[7:0] is data
  while((UCB1IFG&0x0002) == 0){      // wait for second data sent
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return debugdump;
    }
  }
  UCB1TXBUF = data3&0xFF;            // TXBUF[7:0] is data
  while(UCB1STATW&0x0010){           // wait for I2C idle
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return debugdump;
    }
  }
  return 0;
}
