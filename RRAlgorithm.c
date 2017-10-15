/*
 * RRAlgorithm.c
 *
 *  Created on: Oct 15, 2017
 *      Author: Atalville
 */
#include <stdint.h>
#include <stdio.h>
#include "RRAlgorithm.h"
//int gauw_hamm[31];// = {41,276,512,276,41};
//float ghamm[31]={0.08,0.253195,0.64236,0.954446,0.954446,0.64236,0.253195,0.08};
const uint16_t auw_hamm[31]={ 41,    276,    512,    276,     41 }; //Hamm=  long16(512* hamming(5)');

uint32_t rr_find_mean(uint16_t *input){
    uint32_t mean = 0;
    uint16_t i = 0;
    for(i=0;i<RR_BUF_SIZE;i++){
        mean += input[i];
    }
    mean /= RR_BUF_SIZE;
    return mean;

}
/* To remove DC Offset of the signal */
void diff_from_mean(uint16_t *an_x,int16_t *an_y,uint32_t avg){
    int i = 0;
    for(i=0;i<RR_BUF_SIZE;i++){
        an_y[i] = an_x[i] - avg;
    }
}
/* 4 point moving average to smoothen the signal */
void four_pt_MA(int16_t *an_x){
    uint16_t i = 0;
    for(i=0;i<RR_BUF_SIZE-MA4_SIZE+1;i++){
        an_x[i] = an_x[i] + an_x[i+1] + an_x[i+2]+ an_x[i+3];
        an_x[i] /= MA4_SIZE;
    }
}

/* Differentiation of 4 pt MA Signal */
void diff_btw_4pt_MA(int16_t * an_x){
    uint16_t i = 0;
    for(i=0;i<RR_BUF_SIZE-MA4_SIZE;i++){
        an_x[i] = an_x[i+1] - an_x[i];
    }
}

/* 2 point moving average to smoothen the signal */
void two_pt_MA(int16_t * an_dx){
    uint16_t i = 0;
    for(i=0;i< RR_BUF_SIZE - MA2_SIZE + 1;i++){
        an_dx[i] = (an_dx[i] + an_dx[i+1])/MA2_SIZE;
    }
}

// hamming window
   // flip wave form so that we can detect valley with peak detector
void hamming_window(int16_t * an_dx){
    int32_t i = 0,k=0,s=0;
    for ( i=0 ; i<RR_BUF_SIZE-HAM_SIZE-MA4_SIZE-2+1 ;i++){
        s= 0;
        for( k=i; k<i+ HAM_SIZE ;k++){
            s -= an_dx[k] *auw_hamm[k-i] ;
                     }
        an_dx[i]= s/ (int32_t)1146; // divide by sum of auw_hamm
    }
}

int16_t threshold_calc(int16_t *an_dx){
    int i=0, n_th1 = 0;
    for(i=0;i<RR_BUF_SIZE-HAM_SIZE;i++){
        n_th1 += (an_dx[i] > 0)? an_dx[i] : ((int)0-an_dx[i]);
        //printf("n_th[%d] = %d\r\n",i,n_th1);
    }
    n_th1 /= (RR_BUF_SIZE-HAM_SIZE);
    return n_th1;
}
void maxim_find_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x, int32_t n_size, int32_t n_min_height, int32_t n_min_distance, int32_t n_max_num)
/**
* \brief        Find peaks
* \par          Details
*               Find at most MAX_NUM peaks above MIN_HEIGHT separated by at least MIN_DISTANCE
*
* \retval       None
*/
{
    maxim_peaks_above_min_height( pn_locs, pn_npks, pn_x, n_size, n_min_height );
    maxim_remove_close_peaks( pn_locs, pn_npks, pn_x, n_min_distance );
    *pn_npks = min( *pn_npks, n_max_num );
}

void maxim_peaks_above_min_height(int32_t *pn_locs, int32_t *pn_npks, int32_t  *pn_x, int32_t n_size, int32_t n_min_height)
/**
* \brief        Find peaks above n_min_height
* \par          Details
*               Find all peaks above MIN_HEIGHT
*
* \retval       None
*/
{
    int32_t i = 1, n_width;
    *pn_npks = 0;

    while (i < n_size-1){
        if (pn_x[i] > n_min_height && pn_x[i] > pn_x[i-1]){            // find left edge of potential peaks
            n_width = 1;
            while (i+n_width < n_size && pn_x[i] == pn_x[i+n_width])    // find flat peaks
                n_width++;
            if (pn_x[i] > pn_x[i+n_width]){// && (*pn_npks) < 15 ){                            // find right edge of peaks
                pn_locs[(*pn_npks)++] = i;
                // for flat peaks, peak location is left edge
                i += n_width+1;
            }
            else
                i += n_width;
        }
        else
            i++;
    }
}


void maxim_remove_close_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x,int32_t n_min_distance)
/**
* \brief        Remove peaks
* \par          Details
*               Remove peaks separated by less than MIN_DISTANCE
*
* \retval       None
*/
{

    int32_t i, j, n_old_npks, n_dist;

    /* Order peaks from large to small */
    maxim_sort_indices_descend( pn_x, pn_locs, *pn_npks );

    for ( i = -1; i < *pn_npks; i++ ){
        n_old_npks = *pn_npks;
        *pn_npks = i+1;
        for ( j = i+1; j < n_old_npks; j++ ){
            n_dist =  pn_locs[j] - ( i == -1 ? -1 : pn_locs[i] ); // lag-zero peak of autocorr is at index -1
            if ( n_dist > n_min_distance || n_dist < -n_min_distance )
                pn_locs[(*pn_npks)++] = pn_locs[j];
        }
    }

    // Resort indices longo ascending order
    maxim_sort_ascend( pn_locs, *pn_npks );
}

void maxim_sort_ascend(int32_t *pn_x,int32_t n_size)
/**
* \brief        Sort array
* \par          Details
*               Sort array in ascending order (insertion sort algorithm)
*
* \retval       None
*/
{
    int32_t i, j, n_temp;
    for (i = 1; i < n_size; i++) {
        n_temp = pn_x[i];
        for (j = i; j > 0 && n_temp < pn_x[j-1]; j--)
            pn_x[j] = pn_x[j-1];
        pn_x[j] = n_temp;
    }
}

void maxim_sort_indices_descend(int32_t *pn_x, int32_t *pn_indx, int32_t n_size)
/**
* \brief        Sort indices
* \par          Details
*               Sort indices according to descending order (insertion sort algorithm)
*
* \retval       None
*/
{
    int32_t i, j, n_temp;
    for (i = 1; i < n_size; i++) {
        n_temp = pn_indx[i];
        for (j = i; j > 0 && pn_x[n_temp] > pn_x[pn_indx[j-1]]; j--)
            pn_indx[j] = pn_indx[j-1];
        pn_indx[j] = n_temp;
    }
}
void peak_locations(int32_t *pn_locs, int32_t *pn_npks, int32_t  *pn_x){
    int i = 0;
    for(i=0;i<*pn_npks;i++){
        printf("(%d,%d)\r\n",pn_locs[i],pn_x[i]);
    }
}
int16_t myPeakCounter(int16_t  *pn_x, int32_t n_size, int32_t n_min_height){
    uint32_t i = 0;
    uint8_t flag = 0;
    uint32_t count = 0;
    for(i=0;i<n_size;i++){
        if(!flag){
            if(pn_x[i] > n_min_height){
                flag = 1;
            }
        }else{
            if(pn_x[i] < n_min_height){
                flag = 0;
                count++;
            }
        }
    }
    return count;

}


int16_t scaled_hamming_window(float *input, int *output){
    int i =0;
    int sum = 0;
    for(i=0;i<HAM_SIZE;i++){
        output[i] = (int)((512 * input[i]));
        sum += output[i];
    }
    return sum;
}
