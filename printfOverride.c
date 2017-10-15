/*
 * printfOverride.c
 *
 *  Created on: May 27, 2015
*   Author: Gerard Sequeira, bluehash@43oh
 *
 *  With input from:
 *  http://processors.wiki.ti.com/index.php/
 *       Printf_support_for_MSP430_CCSTUDIO_compiler
 */

/* DriverLib Includes */
#include "driverlib.h"
#include "debug.h"

/* Standard Includes */
#include <stdio.h>
#include <string.h>

int fputc(int _c, register FILE *_fp);
int fputs(const char *_ptr, register FILE *_fp);

int fputc(int _c, register FILE *_fp)
{
#ifdef __DEBUG__
        while(!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = (unsigned char) _c;
        return((unsigned char)_c);
#endif
}

int fputs(const char *_ptr, register FILE *_fp)
{

#ifdef __DEBUG__

  unsigned int i, len;
  len = strlen(_ptr);

  for(i=0 ; i<len ; i++)
  {
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = (unsigned char) _ptr[i];
  }

  return len;
#endif
}
