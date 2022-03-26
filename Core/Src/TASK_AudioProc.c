#include <stdint.h>
#include "stm32f407xx.h"
#include "arm_math.h"
#include "pmodI2S2.h"
#include "usbd_cdc_if.h"
#include <signalProc_cortexM4.h>
#include <TASK_AudioProc.h>
#include "serialCom.h"

const uint8_t (*exec_audioProc_subTask[N_SUBTASK])() =
										{
										&wait,    // Task automatically activated when serial port com is open
										&process, // Default Task when no serial port com
										&generate,
										};

const char* audioProc_subTaskCmdList[N_SUBTASK] =
										{
										"q",
										"process",
										"generate",
										"help",
										"info"
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
q31_t pState1[FIRQ31_NTAP+BUFLEN-1];
q31_t pState2[FIRQ31_NTAP+BUFLEN-1];
arm_fir_instance_q31 FIR1_q31;
arm_fir_instance_q31 FIR2_q31;
static uint8_t oldTask = PROCESS;

/* Audio buffers and struct */
q31_t buffer1[BUFLEN];	// Left  channel
q31_t buffer2[BUFLEN];  // Right channel

/* Function definition */
void initTask_audioProc(void)
{
	/* FIR initialization */
	arm_fir_init_q31(&FIR1_q31, FIRQ31_NTAP, pCoeffs1, pState1, BUFLEN);
	arm_fir_init_q31(&FIR2_q31, FIRQ31_NTAP, pCoeffs2, pState2, BUFLEN);
}

uint8_t wait(void)
{
	char cmd;
	if(oldTask != WAIT)
	{
		if(cmd == 'q')
		{
			_printc("Process stopped by user.\r\n");
		}
	}

	/* Send zero to pmodI2S2 audio output */
	memset(buffer1, 0, sizeof(q31_t)*BUFLEN);
	memset(buffer2, 0, sizeof(q31_t)*BUFLEN);
	PMODI2S2_stereoW_q31(buffer1, buffer2);

	/* Turn off green LED */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
	oldTask = WAIT;
	return WAIT;
}

uint8_t process(void)
{
	char cmd[8];
	char arg0[4];
	uint8_t nArg;
	static int FIR_PROCESS;

	/* If necessary read command arguments */
	if(oldTask != PROCESS)
	{
		/* Read command args and disable keyboard printing */
		if(nArg > 1 && !strcmp(arg0, "-nf"))
		{
			_printc("FIR filter disabled by user.\r\n");
			FIR_PROCESS = 0;
		}
		else
		{
			_printc("Using default parameters.\r\n");
			FIR_PROCESS = 1;
		}
		_printc("Processing, type \"q\" to stop.\r\n");
	}

	/* Read audio data */
	PMODI2S2_stereoR_q31(buffer1, buffer2);

	/* Signal Processing */
	if(FIR_PROCESS)
	{
		arm_fir_q31(&FIR1_q31, buffer1, buffer1, BUFLEN);
		arm_fir_q31(&FIR2_q31, buffer2, buffer2, BUFLEN);
	}

	/* Write audio data */
	PMODI2S2_stereoW_q31(buffer1, buffer2);

	/* Toggle green LED */
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

	oldTask = PROCESS;
	return PROCESS;
}

uint8_t generate(void)
{
	char arg0[7];
	q31_t arg1;
	q31_t arg2;
	uint8_t nArg;
	static q31_t std_dev = 100000000;
	static q31_t mean = 0;

	/* If necessary read command arguments */
	if(oldTask != GENERATE)
	{
		/* Read command args and disable keyboard printing */
		if (nArg > 3 && !strcmp(arg0, "wgn"))
		{
			std_dev = arg1;
			mean = arg2;
			_printc("White Gaussian noise:\r\n");
			_printc("- std_dev = %d\r\n", std_dev);
			_printc("- mean    = %d\r\n", mean);
			_printc("Generating signal, type \"q\" to stop.\r\n");
		}
		else
		{
			std_dev = 100000000;
			mean = 0;
			_printc("Using default parameters.\r\n");
			_printc("White Gaussian noise:\r\n");
			_printc("- std_dev = %d\r\n", std_dev);
			_printc("- mean    = %d\r\n", mean);
			_printc("Generating signal, type \"q\" to stop.\r\n");
		}
	}

	/* Compute random gaussian signal */
	randGauss_q31(std_dev, mean, buffer1, BUFLEN);
	randGauss_q31(std_dev, mean, buffer2, BUFLEN);

	/* Write audio data */
	PMODI2S2_stereoW_q31(buffer1, buffer2);

	/* Toggle green LED */
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

	oldTask = GENERATE;
	return GENERATE;
}
