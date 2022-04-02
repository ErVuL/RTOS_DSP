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
#define TASKCMDLEN		16

typedef struct
{
	int32_t stdev;
	int32_t mean;
	int32_t task;
	q31_t* pCoeffsL;
	q31_t* pCoeffsR;
}AP_settingStruct;

/* Exported functions */
void AP_initTask(void);

/* SubTasks */
enum AudioProc_TASK
{
	AP_MUTE,
	AP_PROCESS,
	AP_WGN
};
uint8_t AP_mute(void);     // Audio to 0
uint8_t AP_process(void);  // Audio processing I/O
uint8_t AP_wgn(void);      // White Gaussian Noise generation

/* Functions answering to commands from serial communication */
uint8_t AP_setPROCESS(char* args);
uint8_t AP_setMUTE(char* args);
uint8_t AP_setWGN(char* args);

#endif /* INC_AUDIO_PROC_H_ */
