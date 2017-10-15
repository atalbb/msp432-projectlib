/*
 * adc.h
 *
 *  Created on: Mar 9, 2017
 *      Author: Atalville
 */

#ifndef ADC_H_
#define ADC_H_

#define ADC_VREF        (3.3)
#define ADC_MAX_QUANT   (16383.0)
#define ADC_CHANNELS    2
#define TEMP_CH          0
#define LIGHT_CH         1
#define ADC_SAMPLING_10MS     50
#define TEMP_BUF_SIZE   100
#define LDR_PULL_DOWN_OHM   240.0

typedef enum{
    LIGHT_SUNNY = 0,
    LIGHT_PARTLY_SUNNY,
    LIGHT_OVERCAST,
    LIGHT_TWILIGHT,
    LIGHT_VERY_DARK
}_E_LIGHT_CODE;

void adc_init();
void adc_start_sample();



#endif /* ADC_H_ */
