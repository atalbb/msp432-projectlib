// I2C0.h
// Runs on MSP432
// Provide a function that initializes, sends, and receives
// to the eUSCI module interface
// Daniel Valvano
// August 3, 2015
// Modified by RWB 2/27/16

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

void I2C_Init(void);

// receives one byte from specified slave

uint8_t I2C_Recv(int8_t slave);

// receives two bytes from specified slave
// Used to read the contents of a device register
uint16_t I2C_Recv2(int8_t slave);

// sends one byte to specified slave
// Returns 0 if successful, nonzero if error
uint16_t I2C_Send1(int8_t slave, uint8_t data1);

// sends two bytes to specified slave
// Returns 0 if successful, nonzero if error
uint16_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2);

// sends three bytes to specified slave
// Returns 0 if successful, nonzero if error
uint16_t I2C_Send3(int8_t slave, uint8_t data1, uint8_t data2, uint8_t data3);
