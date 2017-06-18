/*
 * systick.h
 *
 *  Created on: Mar 8, 2017
 *      Author: Atalville
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

void setSystickTimeMs(uint32_t mclkMHz, uint32_t ms);
void systick_init(uint32_t mclkMhz, uint32_t ms);



#endif /* SYSTICK_H_ */
