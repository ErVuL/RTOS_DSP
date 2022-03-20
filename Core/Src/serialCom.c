
#include <stdarg.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "cbuffer.h"
#include "cmsis_os.h"
#include "serialCom.h"
#include "serialCom.h"

#define END_CMD_CHAR ' '
#define MAX_CMD_LEN  128
#define N_CMD		 8

static _Bool HOST_PORT_COM_OPEN = false;
static uint32_t leftOffset = 0;
static int8_t availableCMD = 0;

// Circular buffer declaration and initialization
uint8_t rxbuf[CBUFFER_RX_DATA_SIZE];
static CB_uint8_t rxCBuf = {
		&rxbuf[0],
		&rxbuf[CBUFFER_RX_DATA_SIZE-1],
		&rxbuf[0],
		&rxbuf[0]
};
uint8_t txbuf[CBUFFER_RX_DATA_SIZE];



void SER_open(void)
{
	HOST_PORT_COM_OPEN = true;
}

void SER_close(void)
{
	HOST_PORT_COM_OPEN = false;
}

uint32_t SER_receive(uint8_t* Buf, uint32_t *Len)
{

		uint8_t result = USBD_OK;
		static uint8_t VT100cmdSeq;

		for (uint8_t ii = 0; ii < (*Len); ii++)
		{
			/* Avoid VT100 cmd sequences (4 * uint8_t)*/
			if(Buf[ii] == '\033')
			{	VT100cmdSeq = 4;
			}

			if(!VT100cmdSeq) // avoid VT100cmd
			{
				/* If Backspace key: clear the last char */
				if (Buf[ii] == '\b')
				{
					if(leftOffset)
					{
						CB_rewindWritePtr_u8(&rxCBuf, 1);
						CDC_Transmit_NB((uint8_t*)"\b \b", 3);

						leftOffset--;
					}
				}
				/* Else if Enter key: add a \n to terminal and extract output buffer */
				else if (Buf[ii] == '\r' || Buf[ii] == '\0' || Buf[ii] == '\n')
				{

					CDC_Transmit_FS((uint8_t*)"\r\n> ", 5);
					if(leftOffset)
					{
						CB_write_u8(&rxCBuf, (uint8_t *) "\0", 1);
						leftOffset=0;
						availableCMD++;
					}
				}
				/* Else get the character */
				else
				{
					CB_write_u8(&rxCBuf, &Buf[ii], 1);
					CDC_Transmit_NB(&Buf[ii], 1);
					leftOffset++;
				}
			}
			else
			{	VT100cmdSeq--;
			}
		}
		return result;
}


uint32_t SER_getCmd(char** cmdList, uint32_t len, char* args)
{
	uint8_t  UserRxBufferFS[MAX_CMD_LEN];
	uint8_t  cmd[MAX_CMD_LEN];
	uint8_t  fmt[4] = "%s ";
	uint32_t cmdFound = len;
	fmt[2] = END_CMD_CHAR;

	osMutexAcquire(CDC_RxMutexHandle, osWaitForever);
	if (availableCMD)
	{
		/* Read and record command */
		CB_readUntil_u8(UserRxBufferFS, &rxCBuf, END_CMD_CHAR, MAX_CMD_LEN);
		sscanf((char*) UserRxBufferFS, (char*)fmt, (char*)cmd);
		args[0] = '\0';
		CB_readUntil_u8((uint8_t*)args, &rxCBuf, '\0', MAX_CMD_LEN);

		for(uint8_t ii = 0; ii < len; ii++)
		{
			if(!strcmp((char*)cmd, cmdList[ii]))
			{
				cmdFound = ii;
				availableCMD--;
				break;
			}
		}

		if(cmdFound == len)
		{
			_printc("/!\\ Command \"%s\" not found !\r\n", cmd);
			availableCMD--;
			osMutexRelease(CDC_RxMutexHandle);
			return cmdFound;
		}
	}
	osMutexRelease(CDC_RxMutexHandle);
	return cmdFound;
}

void SER_printLock(void)
{
	osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
}

void SER_printUnlock(void)
{
	osMutexRelease(CDC_TxMutexHandle);
}

void SER_scanLock(void)
{
	osMutexAcquire(CDC_RxMutexHandle, osWaitForever);
}

void SER_scanUnlock(void)
{
	osMutexRelease(CDC_RxMutexHandle);
}

void _printf(const char *format, ...)
{
	va_list arg;
	uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
	if (HOST_PORT_COM_OPEN)
	{
		va_start(arg, format);
		vsprintf((char*) UserTxBufferFS, format, arg);
		va_end(arg);

		osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
		CDC_Transmit_FS(UserTxBufferFS, strlen((char*) UserTxBufferFS));
		osMutexRelease(CDC_TxMutexHandle);
	}
}

void _prints(uint8_t* stream, uint32_t len)
{
	if (HOST_PORT_COM_OPEN)
	{
		CDC_Transmit_FS(stream, len);
	}
}

void _printc(const char *format, ...)
{
	va_list arg;
	uint32_t clktime;
	uint8_t UserTxBufferFS[APP_RX_DATA_SIZE];

	if (HOST_PORT_COM_OPEN)
	{
		clktime = HAL_GetTick();
		sprintf((char*) UserTxBufferFS, "\r[%02lu:%02lu:%02lu.%03lu] ", (clktime/3600000)%100, (clktime/60000)%60, (clktime/1000)%60, clktime%1000);
		va_start(arg, format);
		vsprintf((char*) &UserTxBufferFS[16], format, arg);
		va_end(arg);
		memcpy(&UserTxBufferFS[strlen((char*) UserTxBufferFS)-1], "\r\n> ", 5);

		osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
		CDC_Transmit_FS(UserTxBufferFS, strlen((char*) UserTxBufferFS));
		osMutexRelease(CDC_TxMutexHandle);

	}
}

void _scanf(const char *format, ...)
{
	uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
	va_list arg;

	osMutexAcquire(CDC_RxMutexHandle, osWaitForever);
	while (!availableCMD && HOST_PORT_COM_OPEN)
	{
	}
	if(availableCMD)
	{
		CB_readUntil_u8(UserRxBufferFS, &rxCBuf, END_CMD_CHAR, MAX_CMD_LEN);
		va_start(arg, format);
		vsscanf((char*) UserRxBufferFS, format, arg);
		va_end(arg);
		availableCMD--;
	}
	osMutexRelease(CDC_RxMutexHandle);
}

uint32_t _scans(uint8_t* stream)
{
	availableCMD = 0;
	return CB_read_u8(stream, &rxCBuf, APP_RX_DATA_SIZE);

}


void SER_clear(void)
{
	_printf("\033[2J");
}

void SER_clearLine(void)
{
	_printf("\033[K");
}

void SER_setColor(uint8_t FG, uint8_t BG)
{
	_printf("\033[%2d;%2dm", FG, BG);
}

void SER_setDefaultColor(void)
{
	_printf("\033[0m");
}

void SER_setPos(uint16_t x, uint16_t y)
{
	_printf("\033[%d;%dH", y, x);
}

void SER_move(int16_t x, int16_t y)
{
	if (x < 0)
	{	_printf("\033[%dD", abs(x));
	}
	else if (x > 0)
	{	_printf("\033[%dC", x);
	}

	if (y < 0)
	{	_printf("\033[%dA", abs(y));
	}
	else if (y > 0)
	{	_printf("\033[%dB", y);
	}
}

