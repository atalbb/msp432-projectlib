//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "driverlib.h"
#include "my_i2c.h"
#include "my_clk.h"
#include "uart.h"
#include "systick.h"
#include "bme280_sensor.h"
#include "algorithm.h"
#include "MAX30102.h"


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define MAX_BRIGHTNESS 255

uint32_t aun_ir_buffer[500]; //IR LED sensor data
int32_t n_ir_buffer_length;    //data length
uint32_t aun_red_buffer[500];    //Red LED sensor data
int32_t n_sp02; //SPO2 value
int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;   //heart rate value
int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;

//Serial pc(USBTX, USBRX);    //initializes the serial port
//#ifdef TARGET_KL25Z
//PwmOut led(PTB18);  //initializes the pwm output that connects to the on board LED
//DigitalIn INT(PTD1);  //pin PTD1 connects to the interrupt output pin of the MAX30102
//#endif
//#ifdef TARGET_K64F
//DigitalIn INT(PTD1);  //pin PTD1 connects to the interrupt output pin of the MAX30102
//#endif
//#ifdef TARGET_MAX32600MBED
//PwmOut led(LED_RED);    //initializes the pwm output that connects to the on board LED
//DigitalIn INT(P2_0);  //pin P20 connects to the interrupt output pin of the MAX30102
//#endif
//_S_I2C_CONFIG g_i2c0 = {1,
//                        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,
//                        12000000,
//                        8,
//                        {GPIO_PORT_P6,GPIO_PIN4,GPIO_PIN5,GPIO_PRIMARY_MODULE_FUNCTION},
//                        0x12
//                        };
//void main(void)
//{
//	int i = 0;
//    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
//    init_i2c(&g_i2c0);
//    i = 1;
//    while(1);
//
//}
const eUSCI_UART_Config uartAConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,   // SMCLK Clock Source at 12000000 (divided down from MCLK)
        6,                                // BRDIV (clock prescaler) = INT((12000000/115200=104.167)/16)
        8,                                // UCxBRF INT([(104.167/16)–INT(104.167/16)]×16))=(6.51-6)x16=8.16)
        0x11,                             // UCxBRS = 0x11 (fractional part of N from table on p.721 of tech ref)
        EUSCI_A_UART_NO_PARITY,           // No Parity
        EUSCI_A_UART_LSB_FIRST,           // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,        // One stop bit
        EUSCI_A_UART_MODE,                // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};
float g_temperature = 0.0, g_pressure = 0.0, g_humidity = 0.0,g_temperature_f = 0.0;
void main(){
    int i = 0;
    uint8_t blink_flag = 0;

    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
    /* 48 Mhz Crystal Oscillator, MCLK = 24 Mhz, SMCLK = 12 Mhz */
    clk_init();
    /* UARTA0(115.2 kbps baudrate) Debug Module initialization*/
    uartA0_init(&uartAConfig,0,0);

    /* Systick 1ms tick at 48 Mhz. NOTE Systick Handler is defined in bme280_sensor.c */
    systick_init(48,1);
    /* BME Sensor Initialization */
    bme280_sensor_init();
    /* Global Interrupt enable */
    MAP_Interrupt_enableMaster();
    printf("Pgm Started!\r\n");
//    displayStringH(0,0,"Connecting...",FONTCOLOR,BGCOLOR,LCD_DATA_SIZE);
    while(1){
        get_bme280_values(&g_temperature,&g_pressure,&g_humidity);
        printf("temp = %.2f,press = %.2f, humidity = %.2f\r\n",g_temperature,g_pressure,g_humidity);
        while(++i != 100000);
        i=0;
    }
}
