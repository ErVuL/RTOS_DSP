
#ifndef INC_PMODI2S2_H_
#define INC_PMODI2S2_H_

#include "stm32f407xx.h"
#include "arm_math.h"

/* Define */
#define I2S2_BUFLEN 	4096				// I2S2 buffer length for communication with PMODI2S2 (aka uint8_t*)
#define I2S2_AUDIOLEN	(I2S2_BUFLEN/8) 	// I2S2 audio samples by PMODI2S2 buffer and by channel (aka int32_t*)
#define I2S2_CBUFLEN	4096			    // I2S2 intermediate circular audio (aka int32_t*) buffer length by channel (x4)
#define FREQ_SAMP		93750 				// Hz
#define NBITS			24					// Audio bit depth (this value do not affect the code, audio decoding is hard coded !)

/* I2S CallBack states enumeration */
enum I2S_state {Busy, HalfCplt, Cplt};

/* Exported functions */
void PMODI2S2_stereoR_q31(q31_t * Lbuf, q31_t * Rbuf);
void PMODI2S2_stereoW_q31(q31_t * Lbuf, q31_t * Rbuf);

#endif /* INC_PMODI2S2_H_ */
