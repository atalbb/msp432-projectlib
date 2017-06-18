/*
 * systick.c
 *
 *  Created on: Mar 8, 2017
 *      Author: Atalville
 */
#include "driverlib.h"
#include "debug.h"
void setSystickTimeMs(uint32_t mclkMHz, uint32_t ms){
    MAP_SysTick_setPeriod(mclkMHz * 1000 * ms);
}
void systick_init(uint32_t mclkMhz, uint32_t ms){
    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(mclkMhz * 1000 * ms);
    MAP_SysTick_enableInterrupt();
}

