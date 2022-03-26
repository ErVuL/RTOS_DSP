#include <c_buffer.h>
#include "stm32f4xx_hal.h"
#include "pmodI2S2.h"
#include "usbd_cdc_if.h"

/* I2S2 CallBack state */
uint8_t I2S2_rxState = Busy;				// I2S2 rx CallBack state
uint8_t I2S2_txState = Busy;				// I2S2 tx CallBack state

/* I2S2 buffer and pointer */
extern uint16_t I2S2_txBuf[I2S2_BUFLEN];	// Pointer to I2S2 tx buffer
extern uint16_t I2S2_rxBuf[I2S2_BUFLEN];	// Pointer to I2S2 tx buffer
uint16_t *pI2S2_txBuf = I2S2_txBuf;			// Automatic pointer for I2S2 tx buffer
uint16_t *pI2S2_rxBuf = I2S2_rxBuf;			// Automatic pointer for I2S2 rx buffer

int32_t LrxI2S2[I2S2_CBUFLEN];
CB_int32_t CB_LrxI2S2 = {
		&LrxI2S2[0],
		&LrxI2S2[I2S2_CBUFLEN-1],
		&LrxI2S2[0],
		&LrxI2S2[0]
};

int32_t RrxI2S2[I2S2_CBUFLEN];
CB_int32_t CB_RrxI2S2 = {
		&RrxI2S2[0],
		&RrxI2S2[I2S2_CBUFLEN-1],
		&RrxI2S2[0],
		&RrxI2S2[0]
};

int32_t LtxI2S2[I2S2_CBUFLEN];
CB_int32_t CB_LtxI2S2 = {
		&LtxI2S2[0],
		&LtxI2S2[I2S2_CBUFLEN-1],
		&LtxI2S2[0],
		&LtxI2S2[0]
};

int32_t RtxI2S2[I2S2_CBUFLEN];
CB_int32_t CB_RtxI2S2 = {
		&RtxI2S2[0],
		&RtxI2S2[I2S2_CBUFLEN-1],
		&RtxI2S2[0],
		&RtxI2S2[0]
};
/* Functions definition */

/*
 * Read stereo audio signal from PMODI2S2 ADC/DAC in q31_t (24 LSBs),
 * the audio buffer length of each channel is equal to
 * I2S2_BUFLEN/8. This is a blocking function, it will
 * wait for new audio data before return. This function
 * will always return the last audio buffer available,
 * and never return twice the same.
 *
 *	ARG:
 * 		- q31_t *Lbuf : Left  channel audio buffer.
 * 		- q31_t *Rbuf : Right channel audio buffer.
 *
 * 	RETURN:
 * 		- void
 *
 * 	TODO :
 * 		- Add a timmeout and an error return ?
 * 		- Add possibility to read a precise amount of data
 */
void PMODI2S2_stereoR_q31(q31_t *Lbuf, q31_t *Rbuf)
{
	while(I2S2_rxState == Busy)
	{
	}
	I2S2_rxState = Busy;
	for (uint32_t i = 0; i+3 < I2S2_BUFLEN/2; i += 4)
	{
		Lbuf[i/4] = (q31_t) ((pI2S2_rxBuf[i]   << 16) | pI2S2_rxBuf[i+1]);
		Rbuf[i/4] = (q31_t) ((pI2S2_rxBuf[i+2] << 16) | pI2S2_rxBuf[i+3]);
	}
}

/*
 * Write stereo audio signal to PMODI2S2 ADC/DAC in q31_t (24 LSBs),
 * the audio buffer length of each channel is equal to
 * I2S2_BUFLEN/8. This is a blocking function, it will
 * wait for audio device before return. This function
 * will always send data to the last audio buffer available,
 * and never write twice to the same buffer.
 *
 *	ARG:
 * 		- q31_t *Lbuf : Left  channel audio buffer.
 * 		- q31_t *Rbuf : Right channel audio buffer.
 *
 * 	RETURN:
 * 		- void
 *
 * 	TODO :
 * 		- Add a timmeout and an error return ?
 * 		- Add possibility to write a precise amount of data
 */
void PMODI2S2_stereoW_q31(q31_t *Lbuf, q31_t *Rbuf)
{
	while(I2S2_txState == Busy)
	{
	}
	I2S2_txState = Busy;
	for (uint32_t i = 0; i+3 < I2S2_BUFLEN/2; i += 4)
	{
		pI2S2_txBuf[i]   = (Lbuf[i/4] >> 16) & 0xFFFF;
		pI2S2_txBuf[i+1] =  Lbuf[i/4] & 0xFFFF;
		pI2S2_txBuf[i+2] = (Rbuf[i/4] >> 16) & 0xFFFF;
		pI2S2_txBuf[i+3] =  Rbuf[i/4] & 0xFFFF;
	}
}

/*
 * I2S2 Callbacks set the pointers pI2S2_txBuf and pI2S2_rxBuf
 * to the correct memory location for read and write audio
 * functions. It also update the I2S2 buffers state.
 */
void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	int32_t Lbuf[I2S2_AUDIOLEN];
	int32_t Rbuf[I2S2_AUDIOLEN];


	/* Read input data from CAN24 */
	pI2S2_rxBuf = &I2S2_rxBuf[0];
	for (uint32_t i = 0; i+3 < I2S2_BUFLEN/2; i += 4)
	{
		Lbuf[i/4] = (q31_t) ((pI2S2_rxBuf[i]   << 16) | pI2S2_rxBuf[i+1]);
		Rbuf[i/4] = (q31_t) ((pI2S2_rxBuf[i+2] << 16) | pI2S2_rxBuf[i+3]);
	}
	CB_write_i32(&CB_LrxI2S2, Lbuf, I2S2_AUDIOLEN);
	CB_write_i32(&CB_RrxI2S2, Rbuf, I2S2_AUDIOLEN);

	/* Write output data to CAN24 */
	pI2S2_txBuf = &I2S2_txBuf[0];
	CB_read_i32(Lbuf, &CB_LtxI2S2, I2S2_AUDIOLEN);
	CB_read_i32(Rbuf, &CB_RtxI2S2, I2S2_AUDIOLEN);
	for (uint32_t i = 0; i+3 < I2S2_BUFLEN/2; i += 4)
	{
		pI2S2_txBuf[i]   = (Lbuf[i/4] >> 16) & 0xFFFF;
		pI2S2_txBuf[i+1] =  Lbuf[i/4] & 0xFFFF;
		pI2S2_txBuf[i+2] = (Rbuf[i/4] >> 16) & 0xFFFF;
		pI2S2_txBuf[i+3] =  Rbuf[i/4] & 0xFFFF;
	}
}
void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	int32_t Lbuf[I2S2_AUDIOLEN];
	int32_t Rbuf[I2S2_AUDIOLEN];

	/* Read input data from CAN24 */
	pI2S2_rxBuf = &I2S2_rxBuf[I2S2_BUFLEN/2];
	for (uint32_t i = 0; i+3 < I2S2_BUFLEN/2; i += 4)
	{
		Lbuf[i/4] = (q31_t) ((pI2S2_rxBuf[i]   << 16) | pI2S2_rxBuf[i+1]);
		Rbuf[i/4] = (q31_t) ((pI2S2_rxBuf[i+2] << 16) | pI2S2_rxBuf[i+3]);
	}
	CB_write_i32(&CB_LrxI2S2, Lbuf, I2S2_AUDIOLEN);
	CB_write_i32(&CB_RrxI2S2, Rbuf, I2S2_AUDIOLEN);

	/* Write output data to CAN24 */
	pI2S2_txBuf = &I2S2_txBuf[I2S2_BUFLEN/2];
	CB_read_i32(Lbuf, &CB_LtxI2S2, I2S2_AUDIOLEN);
	CB_read_i32(Rbuf, &CB_RtxI2S2, I2S2_AUDIOLEN);
	for (uint32_t i = 0; i+3 < I2S2_BUFLEN/2; i += 4)
	{
		pI2S2_txBuf[i]   = (Lbuf[i/4] >> 16) & 0xFFFF;
		pI2S2_txBuf[i+1] =  Lbuf[i/4] & 0xFFFF;
		pI2S2_txBuf[i+2] = (Rbuf[i/4] >> 16) & 0xFFFF;
		pI2S2_txBuf[i+3] =  Rbuf[i/4] & 0xFFFF;
	}
}
