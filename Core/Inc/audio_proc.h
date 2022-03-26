/*
 * TASK_SerialUI.h
 *
 *  Created on: May 8, 2021
 *      Author: theo
 */

#ifndef INC_AUDIO_PROC_H_
#define INC_AUDIO_PROC_H_

#include "stdint.h"

/* Define */
#define FIRQ31_NTAP 	64			// Number of FIR coefs
#define AP_NTASK		5 			// Number of main task
#define TASKCMDLEN		16

typedef struct
{
	int32_t stdev;
	int32_t mean;
	int32_t task;
}AP_settingStruct;

/* Exported functions */
void initTask_audioProc(void);

/* SubTasks */
enum AudioProc_TASK
{
	AP_PROCESS,
	AP_WAIT,
	AP_WGN
};
uint8_t process(void);  // Audio processing I/O
uint8_t wait(void);     // Audio to 0
uint8_t wgn(void);      // White Gaussian Noise generation

/* Functions answering to commands from serial communication */
uint8_t AP_setPROCESS(char* args);
uint8_t AP_setWGN(char* args);

#endif /* INC_AUDIO_PROC_H_ */
