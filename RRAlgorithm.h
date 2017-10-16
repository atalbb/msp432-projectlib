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
#define RR_STABLE_FRAME_TIME_S     5


//#define BUF_SIZE    1200
#define MA4_SIZE    4
#define MA2_SIZE    2
#define HAM_SIZE    4
#define MIN_DIST    8
#define MAX_PEAK    200



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
uint32_t rr_find_mean(uint16_t *input);
void diff_from_mean(uint16_t *an_x,int16_t *an_y,uint32_t avg);
void four_pt_MA(int16_t *an_x);
void diff_btw_4pt_MA(int16_t * an_x);
void two_pt_MA(int16_t * an_dx);
void hamming_window(int16_t * an_dx);
int16_t threshold_calc(int16_t *an_dx);
void maxim_find_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x, int32_t n_size, int32_t n_min_height, int32_t n_min_distance, int32_t n_max_num);
void maxim_peaks_above_min_height(int32_t *pn_locs, int32_t *pn_npks, int32_t  *pn_x, int32_t n_size, int32_t n_min_height);
void maxim_remove_close_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x,int32_t n_min_distance);
void maxim_sort_ascend(int32_t *pn_x,int32_t n_size);
void maxim_sort_indices_descend(int32_t *pn_x, int32_t *pn_indx, int32_t n_size);
void peak_locations(int32_t *pn_locs, int32_t *pn_npks, int32_t  *pn_x);
int16_t myPeakCounter(int16_t  *pn_x, int32_t n_size, int32_t n_min_height);
int16_t scaled_hamming_window(float *input, int *output);

#endif /* RRALGORITHM_H_ */
