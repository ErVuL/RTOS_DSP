#include <audio_proc.h>
#include <serial_com.h>
#include <signal_proc_cortexM4.h>
#include <stdint.h>
#include "stm32f407xx.h"
#include "arm_math.h"
#include "pmodI2S2.h"
#include "usbd_cdc_if.h"


static uint32_t AP_TASK;
// 5 kHz Low-Pass with 64 coeffs
q31_t pCoeffsL[FIRQ31_NTAP] = {
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
q31_t pCoeffsR[FIRQ31_NTAP] = {
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

const uint8_t (*ExecAudioProcessing[AP_NTASK])(void) =
{
		&AP_bypass,
		&AP_emfsk,
		&AP_mute,
		&AP_process,
		&AP_wgn
};

static AP_settingStruct AP_settings =
{
		0, 		    // mean
		10000,      // stdev
		AP_PROCESS, // task
		pCoeffsL,   // left channel filter coeffs
		pCoeffsR    // right channel filter coeffs
};

static EMFSK_settingStruct EMFSK_s =
{
		5000,
		500,
		100,
		4
};

/* ARM q31 FIR struct and main task FIR struct */
q31_t pStateL[FIRQ31_NTAP+I2S2_AUDIOLEN-1];
q31_t pStateR[FIRQ31_NTAP+I2S2_AUDIOLEN-1];
arm_fir_instance_q31 FIR_Lq31;
arm_fir_instance_q31 FIR_Rq31;

/* Audio buffers and struct */
q31_t bufL[I2S2_AUDIOLEN];	// Left  channel
q31_t bufR[I2S2_AUDIOLEN];  // Right channel
q31_t ACF[FIRQ31_NTAP+1];   // Autocorrelation function

/* Function definition */
void AP_initTask(void)
{
	/* FIR initialization */
	arm_fir_init_q31(&FIR_Lq31, FIRQ31_NTAP, AP_settings.pCoeffsL, pStateL, I2S2_AUDIOLEN);
	arm_fir_init_q31(&FIR_Rq31, FIRQ31_NTAP, AP_settings.pCoeffsR, pStateR, I2S2_AUDIOLEN);
}


uint8_t AP_mute(void)
{
	/* Send zero to pmodI2S2 audio output */
	memset(bufL, 0, sizeof(q31_t)*I2S2_AUDIOLEN);
	memset(bufR, 0, sizeof(q31_t)*I2S2_AUDIOLEN);
	PMODI2S2_stereoW_q31(bufL, bufR);

	return 0;
}

uint8_t AP_process(void)
{
	/* Read audio input */
	PMODI2S2_stereoR_q31(bufL, bufR);

	/* Signal Processing */
	arm_correlate_q31(bufL, FIRQ31_NTAP/2+2, bufL, FIRQ31_NTAP/2+2, ACF);
	arm_levinson_durbin_q31(ACF, AP_settings.pCoeffsR, NULL, FIRQ31_NTAP);
	arm_fir_q31(&FIR_Rq31, bufL, bufL, I2S2_AUDIOLEN);
	arm_fir_q31(&FIR_Rq31, bufR, bufR, I2S2_AUDIOLEN);

	/* Write audio output */
	PMODI2S2_stereoW_q31(bufL, bufR);

	return 0;
}

uint8_t AP_bypass(void)
{
	PMODI2S2_stereoR_q31(bufL, bufR);
	PMODI2S2_stereoW_q31(bufL, bufR);
	return 0;
}

uint8_t AP_wgn(void)
{
	/* Compute random gaussian signal */
	randGauss_q31(AP_settings.stdev, AP_settings.mean, bufL, I2S2_AUDIOLEN);
	randGauss_q31(AP_settings.stdev, AP_settings.mean, bufR, I2S2_AUDIOLEN);

	/* Write audio output */
	PMODI2S2_stereoW_q31(bufL, bufR);

	return 0;
}

uint8_t AP_emfsk(void)
{
	char msg[MSG_LEN];
	uint32_t shift_sym, shift_msg, ii, jj, msgLen;
	float32_t phi = 0;
	int32_t d = 0;
	int8_t sym          = 0;
	uint8_t nBits_symbol = log2(EMFSK_s.M);
	memset(bufL, 0, sizeof(q31_t)*I2S2_AUDIOLEN);
	memset(bufR, 0, sizeof(q31_t)*I2S2_AUDIOLEN);

	/* Print parameters and specs */
	SER_UIlock();
	_printf("\r\n\tMFSK EMITTER\r\n\t============\r\n\n");
	_printf("M                = %d symbols \r\n", EMFSK_s.M);
	_printf("Bit depth        = %d b       \r\n", nBits_symbol);
	_printf("Bit rate         = %d b/s     \r\n", EMFSK_s.bF);
	_printf("Center freq      = %d Hz      \r\n", EMFSK_s.cF);
	_printf("Deviation freq   = %d Hz      \r\n", EMFSK_s.dF);
	_printf("BandWidth        = %d Hz      \r\n", EMFSK_s.M*(EMFSK_s.dF+EMFSK_s.bF));
	_printf("Modulation index = %d         \r\n", EMFSK_s.dF/EMFSK_s.bF);

	if(EMFSK_s.cF < EMFSK_s.M*(EMFSK_s.dF+EMFSK_s.bF)/2)
	{
		_printf("/!\\ ERROR : Not enough bandwidth available !\r\n");
		AP_setMUTE(NULL);
		return 1;
	}
	if((float32_t) 2*EMFSK_s.dF/EMFSK_s.bF < 1.0)
	{	_printd("/!\\ Modulation index < 1 !\r\n");
	}

	for(;;)
	{
		/* Read message from user and exit if asked */
		_printf("\r\n@ ");
		msgLen = _scansf(msg, MSG_LEN);
		_printf("\r\n");


		if(msgLen)
		{
			if((!strcmp(msg, "exit")) || (!strcmp(msg, "quit")))
			{
				_printd("Exiting MFSK Emitter.\r\n");
				memset(bufL, 0, sizeof(q31_t)*I2S2_AUDIOLEN);
				break;
			}
			SER_InputLock();

			/* Encode message over nBits_symbol/symbol (8 bits/symbol MAX) */
			_printf("Sending message...\r\n");
			sym       = 0;
			shift_sym = 0;
			shift_msg = 0;
			ii        = 0;
			jj        = 0;
			phi       = 0;
			for(;ii < msgLen;)
			{
				/* If not even, 1st bit is set, so write it to symbol */
				if((msg[ii] >> shift_msg)%2 != 0)
				{	sym |= 0x01 << shift_sym;
				}

				/* Update shift */
				shift_msg  = (shift_msg+1)%8;
				shift_sym  = (shift_sym+1)%nBits_symbol;
				if(shift_msg == 0)
				{	ii++;
				}

				/* If new symbol is complete */
				if (shift_sym == 0)
				{
					/* Frequency shift factor */
					if(sym < EMFSK_s.M/2)
					{	d = sym -EMFSK_s.M/2;
					}
					else
					{	d = sym - EMFSK_s.M/2 + 1;
					}

					/* For the length of a symbol */
					for(uint32_t kk = 0; kk < FREQ_SAMP/EMFSK_s.bF/nBits_symbol; kk++)
					{
						/* Create modulated signal */
						bufL[jj++] = Q23MAX/2.0*sin(2.0*PI*(EMFSK_s.cF+d*EMFSK_s.dF)*kk/FREQ_SAMP+phi);
						jj %= I2S2_AUDIOLEN;

						if(kk == FREQ_SAMP/EMFSK_s.bF/nBits_symbol-1)
						{	 phi = (2.0*PI*(EMFSK_s.cF+d*EMFSK_s.dF)*(kk+1)/FREQ_SAMP+phi);
						}

						/* If buffer is full write audio output */
						if(jj == 0)
						{
							PMODI2S2_stereoW_q31(bufL, bufR);
							memset(bufL, 0, sizeof(q31_t)*I2S2_AUDIOLEN);
						}
					}
					sym = 0;
				}

				/* If reach end of message */
				if(ii == msgLen)
				{
					PMODI2S2_stereoW_q31(bufL, bufR);
					memset(bufL, 0, sizeof(q31_t)*I2S2_AUDIOLEN);
				}
			}
			_printf("Done.\r\n");
			PMODI2S2_stereoW_q31(bufL, bufR);
			PMODI2S2_stereoW_q31(bufL, bufR);
			SER_InputUnlock();
		}

	}

	SER_UIunlock();
	AP_setMUTE(NULL);
	_printf("Done.\r\n");
	return 0;
}

uint8_t AP_setPROCESS(char* args)
{
	AP_TASK = AP_PROCESS;
	_printd("Processing mode set.\r\n");
	return 0;
}

uint8_t AP_setEMFSK(char* args)
{
	AP_TASK = AP_EMFSK;
	_printd("Starting MFSK emmiter.\r\n");
	return 0;
}

uint8_t AP_setBYPASS(char* args)
{
	AP_TASK = AP_BYPASS;
	_printd("Bypass mode set.\r\n");
	return 0;
}

uint8_t AP_setMUTE(char* args)
{
	AP_TASK = AP_MUTE;
	_printd("Mute mode set.\r\n");
	return 0;
}

uint8_t AP_setWGN(char* args)
{
	int32_t valtmp;
	char*   strtmp;

	AP_settings.task = AP_WGN;
	_printd("WGN mode set.\r\n");

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
	return AP_TASK;
}
