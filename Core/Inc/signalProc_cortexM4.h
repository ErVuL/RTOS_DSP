#include "stm32f407xx.h"
#include "arm_math.h"
#include <stdlib.h>

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SP_CORTEXM4__
#define __SP_CORTEXM4__

#define Q63MAX 9223372036854775808L
#define Q31MAX 2147483648
#define Q23MAX 8388608
#define Q15MAX 32768

#define TRUE  1
#define FALSE 0

/* Exported functions */
void randGauss_q31(q31_t std_dev, q31_t mean, q31_t *outputBuffer, uint32_t n);
void norm(double std_dev, double mean, q31_t *outputBuffer, uint32_t n);
double rand_val(int seed);






#endif /* __SP_CORTEXM4__ */
