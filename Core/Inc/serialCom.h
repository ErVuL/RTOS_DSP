#ifndef __SERIALCOM_H__
#define __SERIALCOM_H__

#include "cbuffer.h"
#include "usbd_cdc_if.h"
#include "cmsis_os2.h"

#define END_CMD_CHAR ' '
#define MAX_CMD_LEN  128
#define N_CMD		 8

extern osMutexId_t CDC_RxMutexHandle;
extern osMutexId_t CDC_TxMutexHandle;

uint32_t SER_receive(uint8_t* Buf, uint32_t *Len);
_Bool    SER_flush(void);
uint32_t SER_getCmd(char** cmdList, uint32_t len, char* args);
void     _printf(const char *format, ...);
void     _prints(uint8_t* stream, uint32_t len);
void     _printc(const char *format, ...);
void     _scanf(const char *format, ...);
uint32_t _scans(uint8_t* stream);
void     SER_clear(void);
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


#endif /* __SERIALCOM_H__ */
