#ifndef __SERIALCOM_H__
#define __SERIALCOM_H__

#include "cbuffer.h"
#include "usbd_cdc_if.h"
#include "cmsis_os2.h"

#define SERIAL_BLOCK_SIZE	  64
#define PRINTF_BLOCK_SIZE	  256
#define END_CMD_CHAR 		  ' '
#define N_CMD		 		  5

#define GETNAME(var)  #var
#define _PRINT32(var)  _printf("%s = %ld\r\n", GETNAME(var), var)

extern osMutexId_t CDC_RxMutexHandle;
extern osMutexId_t CDC_TxMutexHandle;

typedef struct
{
	char* Str;
	char* Help;
	uint8_t (*ExecFunction)(char* args);
}SER_cmdStruct;

/* Main serial commands */
uint8_t SER_info(char* args);
uint8_t SER_help(char* args);
uint8_t SER_clc(char* args);

/* Print and scan functions */
void     _printf(const char *format, ...);         // INF timeout
void     _prints(uint8_t* stream, uint32_t len);   // No  semaphore
void     _printd(const char *format, ...);         // INF timeout
void 	 _printn(const char *format, ...);         // 0   timeout
void 	 _printc(uint8_t FG, uint8_t BG,
		         const char *format, ...);		   // INF timeout
void     _scanf(const char *format, ...);		   // INF timeout
uint32_t _scans(uint8_t* stream);                  // No  semaphore

/* Serial special commands */
uint32_t SER_receive(uint8_t* buf, uint32_t *len);
uint32_t SER_getCmd(const SER_cmdStruct* cmdStructTab, uint32_t len, char* args);
void     SER_clearLine(void);
void     SER_setColor(uint8_t FG, uint8_t BG);
void     SER_setDefaultColor(void);
void     SER_setPos(uint16_t x, uint16_t y);
void     SER_move(int16_t x, int16_t y);
void 	 SER_open(void);
void 	 SER_close(void);
void 	 SER_printLock(void);
void 	 SER_scanLock(void);
void 	 SER_printUnlock(void);
void 	 SER_scanUnlock(void);
void 	 SER_flush(void);
void 	 SER_clear(void);


#endif /* __SERIALCOM_H__ */
