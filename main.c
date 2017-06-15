//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "driverlib.h"
#include "my_i2c.h"
#include "my_clk.h"
_S_I2C_CONFIG g_i2c0 = {1,
                        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,
                        3000000,
                        100,
                        {GPIO_PORT_P6,GPIO_PIN4,GPIO_PIN5,GPIO_PRIMARY_MODULE_FUNCTION},
                        0x12
                        };
void main(void)
{
	int i = 0;
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
    init_i2c(&g_i2c0);
    //I2C_Init();
    i = 1;
    while(1);
	
}
