/*
 * TASK_SerialUI.h
 *
 *  Created on: May 8, 2021
 *      Author: theo
 */

#ifndef INC_AUDIO_PROC_H_
#define INC_AUDIO_PROC_H_

#include "stdint.h"
#include "stm32f407xx.h"
#include "arm_math.h"


/* Define */
#define FIRQ31_NTAP 	64			// Number of FIR coefs
#define AP_NTASK		5 			// Number of main task
#define TASKCMDLEN		17
#define MSG_LEN         128         // Communication protocols message length

typedef struct
{
	int32_t stdev;
	int32_t mean;
	int32_t task;
	q31_t* pCoeffsL;
	q31_t* pCoeffsR;
}AP_settingStruct;

typedef struct
{
	uint32_t cF;   // Center frequency
	uint32_t dF;   // Frequency deviation
	uint32_t bF;   // Bit rate in Hz
	uint32_t M;    // Number of symbols
}EMFSK_settingStruct;

/* Exported functions */
void AP_initTask(void);

/* SubTasks */
enum AudioProc_TASK
{
	AP_BYPASS,
	AP_EMFSK,
	AP_MUTE,
	AP_PROCESS,
	AP_WGN
};
uint8_t AP_mute(void);     // Audio to 0
uint8_t AP_process(void);  // Audio processing I/O
uint8_t AP_wgn(void);      // White Gaussian Noise generation
uint8_t AP_bypass(void);   // Bypass audio stream
uint8_t AP_emfsk(void);    // Send user keyboard input by MFSK


/* Functions answering to commands from serial communication */
uint8_t AP_setPROCESS(char* args);
uint8_t AP_setEMFSK(char* args);
uint8_t AP_setMUTE(char* args);
uint8_t AP_setWGN(char* args);
uint8_t AP_setBYPASS(char* args);



#endif /* INC_AUDIO_PROC_H_ */
