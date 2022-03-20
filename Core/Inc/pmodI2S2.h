
#ifndef INC_PMODI2S2_H_
#define INC_PMODI2S2_H_

#include "stm32f407xx.h"
#include "arm_math.h"

/* Define */
#define I2S2_BUFLEN 128				// I2S2 buffer length for PMODI2S2 audio data
#define BUFLEN		(I2S2_BUFLEN/8) // Audio buffers length
#define FREQ_SAMP	93750 			// Hz
#define NBITS		24				// Audio data number of bits (this value do not affect the code, audio decoding is hard coded !)

/* I2S CallBack states enumeration */
enum I2S_state {Busy, HalfCplt, Cplt};

/* Exported functions */
void PMODI2S2_stereoR_q31(q31_t * Lbuf, q31_t * Rbuf);
void PMODI2S2_stereoW_q31(q31_t * Lbuf, q31_t * Rbuf);

#endif /* INC_PMODI2S2_H_ */
