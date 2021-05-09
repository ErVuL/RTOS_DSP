/*
 * TASK_SerialUI.h
 *
 *  Created on: May 8, 2021
 *      Author: theo
 */

#ifndef INC_TASK_AUDIOPROC_H_
#define INC_TASK_AUDIOPROC_H_

/* Define */
#define FIRQ31_NTAP 	64			// Number of FIR coefs
#define N_SUBTASK		5 			// Number of main task
#define TASKCMDLEN		16

/* Main task enumeration */
enum MainTask {WAIT, PROCESS, GENERATE, INFO, HELP};

/* Exported functions */
void initTask(void);
uint8_t wait(void);
uint8_t process(void);
uint8_t generate(void);
uint8_t info(void);
uint8_t help(void);

#endif /* INC_TASK_AUDIOPROC_H_ */
