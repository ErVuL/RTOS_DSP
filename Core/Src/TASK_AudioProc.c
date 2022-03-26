#include <stdint.h>
#include "stm32f407xx.h"
#include "arm_math.h"
#include "pmodI2S2.h"
#include "usbd_cdc_if.h"
#include <signalProc_cortexM4.h>
#include <TASK_AudioProc.h>
#include "serialCom.h"

extern CB_int32_t CB_LtxI2S2;
extern CB_int32_t CB_RtxI2S2;
extern CB_int32_t CB_LrxI2S2;
extern CB_int32_t CB_RrxI2S2;

const uint8_t (*ExecAudioProcessing[AP_NTASK])(void) =
										{
										&process,
										&wait,
										&wgn
										};
static AP_settingStruct AP_settings =
{
		0, 		// mean
		10000,  // stdev
		AP_WAIT // task
};

/* Particular FIR Coeffs */

// 5 kHz Low-Pass with 64 coeffs
q31_t pCoeffs1[FIRQ31_NTAP] = {
		-1570804,-1317613,-918200,-301919,
		600114,1817524,3303294,4906358,
		6363311,7315692,7354958,6091972,
		3242478,-1284388,-7310130,-14339176,
		-21552576,-27851158,-31949198,-32511768,
		-28321347,-18453264,-2436166,19626339,
		46993191,78296112,111626976,144696444,
		175048015,200303471,218410584,227862698,
		227862698,218410584,200303471,175048015,
		144696444,111626976,78296112,46993191,
		19626339,-2436166,-18453264,-28321347,
		-32511768,-31949198,-27851158,-21552576,
		-14339176,-7310130,-1284388,3242478,
		6091972,7354958,7315692,6363311,
		4906358,3303294,1817524,600114,
		-301919,-918200,-1317613,-1570804
};

// 5 kHz Low-Pass with 64 coeffs
q31_t pCoeffs2[FIRQ31_NTAP] = {
		-1570804,-1317613,-918200,-301919,
		600114,1817524,3303294,4906358,
		6363311,7315692,7354958,6091972,
		3242478,-1284388,-7310130,-14339176,
		-21552576,-27851158,-31949198,-32511768,
		-28321347,-18453264,-2436166,19626339,
		46993191,78296112,111626976,144696444,
		175048015,200303471,218410584,227862698,
		227862698,218410584,200303471,175048015,
		144696444,111626976,78296112,46993191,
		19626339,-2436166,-18453264,-28321347,
		-32511768,-31949198,-27851158,-21552576,
		-14339176,-7310130,-1284388,3242478,
		6091972,7354958,7315692,6363311,
		4906358,3303294,1817524,600114,
		-301919,-918200,-1317613,-1570804
};

/* ARM q31 FIR struct and main task FIR struct */
q31_t pState1[FIRQ31_NTAP+I2S2_AUDIOLEN-1];
q31_t pState2[FIRQ31_NTAP+I2S2_AUDIOLEN-1];
arm_fir_instance_q31 FIR1_q31;
arm_fir_instance_q31 FIR2_q31;

/* Audio buffers and struct */
q31_t Lbuf[I2S2_AUDIOLEN];	// Left  channel
q31_t Rbuf[I2S2_AUDIOLEN];  // Right channel

/* Function definition */
void initTask_audioProc(void)
{
	/* FIR initialization */
	arm_fir_init_q31(&FIR1_q31, FIRQ31_NTAP, pCoeffs1, pState1, I2S2_AUDIOLEN);
	arm_fir_init_q31(&FIR2_q31, FIRQ31_NTAP, pCoeffs2, pState2, I2S2_AUDIOLEN);
}


uint8_t wait(void)
{
	/* Send zero to pmodI2S2 audio output */
	memset(Lbuf, 0, sizeof(q31_t)*I2S2_AUDIOLEN);
	memset(Rbuf, 0, sizeof(q31_t)*I2S2_AUDIOLEN);
	CB_write_i32(&CB_LtxI2S2, Lbuf, I2S2_AUDIOLEN);
	CB_write_i32(&CB_RtxI2S2, Rbuf, I2S2_AUDIOLEN);

	/* Toggle green LED */
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

	return 0;
}

uint8_t process(void)
{
	/* Read audio data */
	CB_read_i32(Lbuf, &CB_LrxI2S2, I2S2_AUDIOLEN);
	CB_read_i32(Rbuf, &CB_RrxI2S2, I2S2_AUDIOLEN);

	/* Signal Processing */
	arm_fir_q31(&FIR1_q31, Lbuf, Lbuf, I2S2_AUDIOLEN);
	arm_fir_q31(&FIR2_q31, Rbuf, Rbuf, I2S2_AUDIOLEN);

	/* Write audio data */
	CB_write_i32(&CB_LtxI2S2, Lbuf, I2S2_AUDIOLEN);
	CB_write_i32(&CB_RtxI2S2, Rbuf, I2S2_AUDIOLEN);

	/* Toggle green LED */
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

	return 0;
}

uint8_t wgn(void)
{
	/* Compute random gaussian signal */
	randGauss_q31(AP_settings.stdev, AP_settings.mean, Lbuf, I2S2_AUDIOLEN);
	randGauss_q31(AP_settings.stdev, AP_settings.mean, Rbuf, I2S2_AUDIOLEN);

	/* Write audio data */
	CB_write_i32(&CB_LtxI2S2, Lbuf, I2S2_AUDIOLEN);
	CB_write_i32(&CB_RtxI2S2, Rbuf, I2S2_AUDIOLEN);

	/* Toggle green LED */
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

	return 0;
}



uint8_t AP_setPROCESS(char* args)
{
	AP_settings.task = AP_PROCESS;
	_printf("Audio processing mode set.\r\n");
	return 0;
}

uint8_t AP_setWGN(char* args)
{
	int32_t valtmp;
	char*   strtmp;

	AP_settings.task = AP_WGN;
	_printf("Noise generation mode set.\r\n");

	/* Parse argument mean */
	valtmp = AP_settings.mean;
	strtmp = strstr(args, "--mean=");
	sscanf(strtmp, "--mean=%ld", &valtmp);
	if(valtmp != AP_settings.mean)
	{
		AP_settings.mean = valtmp;
		_PRINT32(AP_settings.mean);
	}

	/* Parse argument stdev */
	valtmp = AP_settings.stdev;
	strtmp = strstr(args, "--stdev=");
	sscanf(strtmp, "--stdev=%ld", &valtmp);
	if(valtmp != AP_settings.stdev)
	{
		AP_settings.stdev = valtmp;
		_PRINT32(AP_settings.stdev);
	}

	return 0;
}

int32_t AP_getTask(void)
{
	return AP_settings.task;
}
