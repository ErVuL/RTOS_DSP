#ifndef __SERIALCOM_H__
#define __SERIALCOM_H__

#include "cbuffer.h"
#include "usbd_cdc_if.h"

#define END_CMD_CHAR ' '
#define MAX_CMD_LEN  128
#define N_CMD		 8

extern _Bool HOST_PORT_COM_OPEN;
extern osMutexId_t CDC_RxMutexHandle;
extern osMutexId_t CDC_TxMutexHandle;

uint32_t SER_receive(uint8_t* Buf, uint32_t *Len);
_Bool    SER_flush(void);
_Bool    SER_readCmd(void);
void     _printf(const char *format, ...);
void     _prints(uint8_t* stream, uint32_t len);
void     _printc(const char *format, ...);
void     _scanf(const char *format, ...);
void     SER_clear(void);
void     SER_clearLine(void);
void     SER_setColor(uint8_t FG, uint8_t BG);
void     SER_setDefaultColor(void);
void     SER_setPos(uint16_t x, uint16_t y);
void     SER_move(int16_t x, int16_t y);

#endif /* __SERIALCOM_H__ */
