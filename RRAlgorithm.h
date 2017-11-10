/*
 * RRAlgorithm.h
 *
 *  Created on: Oct 15, 2017
 *      Author: Atalville
 */

#ifndef RRALGORITHM_H_
#define RRALGORITHM_H_

/* Configurable defines */
#define RR_SPS                     20//25//50
#define RR_INITIAL_FRAME_TIME_S    30
#define RR_STABLE_FRAME_TIME_S     4


//#define BUF_SIZE    1200
#define MA4_SIZE    4
//#define MIN_DIST    8



/* Non-Configurable defines */
#define RR_SAMPLE_TIME_MS          (1000/RR_SPS)
#define RR_BUF_SIZE               (RR_INITIAL_FRAME_TIME_S * RR_SPS)
#define RR_STABLE_BUF_SIZE        (RR_STABLE_FRAME_TIME_S * RR_SPS)
#define min(x,y) ((x) < (y) ? (x) : (y))

typedef enum{
    RR_INITIAL = 0,
    RR_STABLE
}_E_RR_STATE;


/* Function prototypes */
double rr_find_mean(double *input);
void diff_from_mean(double *an_x,double *an_y,double avg);
void four_pt_MA(double *an_x);
void ButterworthLowpassFilter0040SixthOrder(const double *src, double *dest, int size);
uint16_t myPeakCounter(double  *pn_x, int32_t n_size, double n_min_height);
double threshold_calc(double *an_dx);
void ButterworthLowpassFilter0100SixthOrder(const double src[], double dest[], int size);
#endif /* RRALGORITHM_H_ */
