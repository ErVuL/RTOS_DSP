#include "stm32f407xx.h"
#include "arm_math.h"
#include <stdlib.h>

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CBUFFER__
#define __CBUFFER__



/* Exported structures */

// Circular buffer of int32_t
typedef struct CB_int32_t {
	int32_t* start_ptr;
	int32_t* end_ptr;
	int32_t* write_ptr;
	int32_t* read_ptr;
}CB_int32_t;


// Circular buffer of uint8_t
typedef struct CB_uint8_t {
	uint8_t* start_ptr;
	uint8_t* end_ptr;
	uint8_t* write_ptr;
	uint8_t* read_ptr;
}CB_uint8_t;



/* Exported functions */

// Buffer read and write for int32_t
uint32_t CB_read_i32(int32_t* oBuf, CB_int32_t* cBuf, uint32_t maxLen);
uint32_t CB_write_i32(CB_int32_t* cBuf, int32_t* iBuf, uint32_t len);

// Buffer read and write for uint8_t
uint32_t CB_read_u8(uint8_t* oBuf, CB_uint8_t* cBuf, uint32_t maxLen);
uint32_t CB_write_u8(CB_uint8_t* cBuf, uint8_t* iBuf, uint32_t len);

uint32_t CB_rewindReadPtr_u8(CB_uint8_t* cBuf, uint32_t len);
uint32_t CB_rewindWritePtr_u8(CB_uint8_t* cBuf, uint32_t len);

uint32_t CB_readUntil_u8(uint8_t* oBuf, CB_uint8_t* cBuf, uint8_t val, uint32_t maxLen);

#endif /* __CBUFFER__ */
