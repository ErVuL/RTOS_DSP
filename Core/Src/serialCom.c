
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

static uint32_t leftOffset = 0;
static int8_t availableCMD = 0;
static uint32_t SER_receive(uint8_t* Buf, uint32_t *Len);
static _Bool  SER_flush(void);
static _Bool  SER_readCmd(void);

// Circular buffer declaration and initialization
uint8_t rxbuf[CBUFFER_RX_DATA_SIZE];
static CB_uint8_t rxCBuf = {
		&rxbuf[0],
		&rxbuf[CBUFFER_RX_DATA_SIZE-1],
		&rxbuf[0],
		&rxbuf[0]
};
uint8_t txbuf[CBUFFER_RX_DATA_SIZE];
static CB_uint8_t txCBuf = {
		&txbuf[0],
		&txbuf[CBUFFER_RX_DATA_SIZE-1],
		&txbuf[0],
		&txbuf[0]
};



static uint32_t SER_receive(uint8_t* Buf, uint32_t *Len)
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
						CB_write_u8(&txCBuf, (uint8_t *) "\b \b", 3);
						leftOffset--;
					}
				}
				/* Else if Enter key: add a \n to terminal and extract output buffer */
				else if (Buf[ii] == '\r' || Buf[ii] == '\0' || Buf[ii] == '\n')
				{

					CB_write_u8(&txCBuf, (uint8_t *) "\r\n", 2);
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
					CB_write_u8(&txCBuf, &Buf[ii], 1);
					leftOffset++;
				}
			}
			else
			{	VT100cmdSeq--;
			}
		}
		return result;
}


static _Bool SER_flush(void)
{
	uint32_t len;
	uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

	if(HOST_PORT_COM_OPEN)
	{
		osMutexAcquire(TxCBMutexHandle, osWaitForever);
		if((len = CB_read_u8(UserTxBufferFS, &txCBuf, APP_TX_DATA_SIZE-2)))
		{
			osMutexRelease(TxCBMutexHandle);
			if(UserTxBufferFS[len-1] == '\n')
			{
				UserTxBufferFS[len++] = '>';
				UserTxBufferFS[len++] = ' ';
				leftOffset = 0;
			}
			if(CDC_Transmit_FS(UserTxBufferFS, len) == USBD_OK)
			{	return true;
			}
			else
			{
				osMutexAcquire(TxCBMutexHandle, osWaitForever);
				CB_rewindReadPtr_u8(&txCBuf, len);
				osMutexRelease(TxCBMutexHandle);
			}
		}
		else
		{
			osMutexRelease(TxCBMutexHandle);
		}

	}
	return false;
}

static uint32_t SER_getCmd(char** cmdList, uint32_t len)
{
	uint8_t  UserRxBufferFS[MAX_CMD_LEN];
	uint8_t  cmd[MAX_CMD_LEN];
	uint8_t  fmt[4] = "%s ";
	uint32_t cmdFound = false;
	fmt[2] = END_CMD_CHAR;

	if (availableCMD)
	{
		/* Read and record command */
		CB_readUntil_u8(UserRxBufferFS, &rxCBuf, END_CMD_CHAR, MAX_CMD_LEN);
		sscanf((char*) UserRxBufferFS, fmt, cmd);

		for(uint8_t ii = 0; ii < len; ii++)
		{
			if(!strcmp(cmd, cmdList[ii]))
			{
				*execCmd[ii](UserRxBufferFS);
				cmdFound = ii;
				availableCMD--;
				break;
			}
		}

		if(!cmdFound)
		{
			_cprintf("/!\\ Command not found !\r\n");
			availableCMD--;
			return len;
		}
	}
	return cmdFound;
}


void _printf(const char *format, ...)
{
	va_list arg;
	uint8_t UserTxBufferFS[APP_RX_DATA_SIZE];
	if (HOST_PORT_COM_OPEN)
	{
		va_start(arg, format);
		vsprintf((char*) UserTxBufferFS, format, arg);
		va_end(arg);
		osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
		CB_write_u8(&txCBuf, UserTxBufferFS, strlen((char*) UserTxBufferFS));
		SER_flush();
		osMutexRelease(CDC_TxMutexHandle);
	}
}

void _prints(uint8_t* stream, uint32_t len)
{
	va_list arg;
	uint8_t UserTxBufferFS[APP_RX_DATA_SIZE];
	if (HOST_PORT_COM_OPEN)
	{
		va_start(arg, format);
		vsprintf((char*) UserTxBufferFS, format, arg);
		va_end(arg);
		CB_write_u8(&txCBuf, UserTxBufferFS, strlen((char*) UserTxBufferFS));
		SER_flush();
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
		va_start(arg, format);
		sprintf((char*) UserTxBufferFS, "\r[%02lu:%02lu:%02lu.%03lu] ", (clktime/3600000)%100, (clktime/60000)%60, (clktime/1000)%60, clktime%1000);
		vsprintf((char*) &UserTxBufferFS[16], format, arg);
		va_end(arg);
		osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
		CB_write_u8(&txCBuf, UserTxBufferFS, strlen((char*) UserTxBufferFS));
		SER_flush();
		osMutexRelease(CDC_TxMutexHandle);

	}
}

void _scanf(const char *format, ...)
{
	uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

	osMutexAcquire(CDC_RxMutexHandle, osWaitForever);
	while (!CDC_RX_DATA_PENDING && HOST_PORT_COM_OPEN)
	{
	}
	if (CDC_RX_DATA_PENDING)
	{
		CB_readUntil_u8((uint8_t*) UserRxBufferFS, &rxCBuf, '\0', APP_TX_DATA_SIZE);
		va_list arg;
		va_start(arg, format);
		vsscanf((char*) UserRxBufferFS, format, arg);
		va_end(arg);
		CDC_RX_DATA_PENDING--;
	}
	osMutexRelease(CDC_RxMutexHandle);
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

