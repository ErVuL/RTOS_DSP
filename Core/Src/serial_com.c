
#include <audio_proc.h>
#include <c_buffer.h>
#include <serial_com.h>
#include <serial_com.h>
#include <stdarg.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"


static _Bool HOST_PORT_COM_OPEN = false;
static uint32_t leftOffset = 0;
static int8_t availableCMD = 0;
extern uint32_t SER_UI_TASKDELAY;

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
		&txbuf[CBUFFER_TX_DATA_SIZE-1],
		&txbuf[0],
		&txbuf[0]
};

const SER_cmdStruct cmdStructTab[N_CMD] = {

		{
		"build",
		"- build \r\n\t * Display build information.",
		&SER_build
		},
		{
		"clear",
		"- clear \r\n\t * Clear the screen.",
		&SER_clc
		},
		{
		"help",
		"- help \r\n\t * Display all commands --help.",
		&SER_help
		},
		{
		"mute",
		"- mute \r\n\t * Mute audio.",
		&AP_setMUTE
		},
		{
		"process",
		"- process \r\n\t * Start audio processing.",
		&AP_setPROCESS
		},
		{
		"wgn",
		"- wgn --mean=%d --stdev=%d \r\n\t * Generate white gaussian noise with specified "
		                           "\r\n\t   mean and standard deviation.",
		&AP_setWGN
		}
};

uint8_t SER_build(char* args)
{
	_printf("v%d.%02d, %s @ %s UTC+1.\r\n",MAJ_VERSION, MIN_VERSION, __DATE__, __TIME__);
	return 0;
}

uint8_t SER_help(char* args)
{
	_printf("\r\n");
	for(int ii = 0; ii < N_CMD; ii++)
	{	_printf("%s\r\n\n", cmdStructTab[ii].Help);
	}
	_printf("\r\n");
	return 0;
}

uint8_t SER_clc(char* args)
{
	SER_clear();
	return 0;
}



void SER_open(void)
{
	HOST_PORT_COM_OPEN = true;
	SER_UI_TASKDELAY = 25;
}

void SER_close(void)
{
	HOST_PORT_COM_OPEN = false;
	SER_UI_TASKDELAY = 500;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}


uint32_t SER_receive(uint8_t* buf, uint32_t *len)
{

		uint8_t result = USBD_OK;
		static uint8_t VT100cmdSeq;

		for (uint8_t ii = 0; ii < (*len); ii++)
		{
			/* Avoid VT100 cmd sequences (4 * uint8_t)*/
			if(buf[ii] == '\033')
			{	VT100cmdSeq = 4;
			}

			if(!VT100cmdSeq) // avoid VT100cmd
			{
				/* If Backspace key: clear the last char */
				if (buf[ii] == '\b')
				{
					if(leftOffset)
					{
						CB_rewindWritePtr_u8(&rxCBuf, 1);
						CB_write_u8(&txCBuf, (uint8_t *) "\b \b", 3);
						leftOffset--;
					}
				}
				/* Else if Enter key: add a \n to terminal and extract output buffer */
				else if (buf[ii] == '\r' || buf[ii] == '\0' || buf[ii] == '\n')
				{

					//CDC_Transmit_FS((uint8_t*)"\r\n", 3);
					CB_write_u8(&txCBuf, (uint8_t *) "\r\n", 3);
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
					CB_write_u8(&rxCBuf, &buf[ii], 1);
					CB_write_u8(&txCBuf, (uint8_t *) &buf[ii], 1);
					leftOffset++;
				}
			}
			else
			{	VT100cmdSeq--;
			}
		}
		return result;
}


uint32_t SER_getCmd(const SER_cmdStruct* cmdStructTab, uint32_t len, char* args)
{
	uint8_t  UserRxBufferFS[PRINTF_MAX_SIZE];
	uint8_t  cmd[PRINTF_MAX_SIZE];
	uint8_t  fmt[4] = "%s ";
	uint32_t cmdFound = len;
	fmt[2] = END_CMD_CHAR;

	osMutexAcquire(CDC_RxMutexHandle, osWaitForever);
	if (availableCMD)
	{
		/* Read and record command */
		CB_readUntil_u8(UserRxBufferFS, &rxCBuf, END_CMD_CHAR, PRINTF_MAX_SIZE);
		sscanf((char*) UserRxBufferFS, (char*)fmt, (char*)cmd);
		args[0] = '\0';
		CB_readUntil_u8((uint8_t*)args, &rxCBuf, '\0', PRINTF_MAX_SIZE);

		/* Research for the corresponding Cmd string */
		for(uint8_t ii = 0; ii < len; ii++)
		{
			if(!strcmp((char*)cmd, cmdStructTab[ii].Str))
			{
				if(!strcmp(args, "--help"))
				{
					_printf("\r\n%s\r\n\n", cmdStructTab[ii].Help);
					availableCMD--;
					osMutexRelease(CDC_RxMutexHandle);
					return cmdFound;
				}
				else
				{
					cmdFound = ii;
					availableCMD--;
					break;
				}
			}
		}

		if(cmdFound == len)
		{
			_printd("/!\\ Command \"%s\" not found !\r\n", cmd);
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
	uint8_t UserTxBufferFS[PRINTF_MAX_SIZE];
	if (HOST_PORT_COM_OPEN)
	{
		va_start(arg, format);
		vsprintf((char*) UserTxBufferFS, format, arg);
		va_end(arg);

		osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
		CB_write_u8(&txCBuf, (uint8_t *) UserTxBufferFS, strlen((char*) UserTxBufferFS));
		osMutexRelease(CDC_TxMutexHandle);
	}
}

void _printc(uint8_t FG, uint8_t BG, const char *format, ...)
{
	va_list arg;
	uint8_t UserTxBufferFS[PRINTF_MAX_SIZE];

	if (HOST_PORT_COM_OPEN)
	{
		osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
		sprintf((char*) UserTxBufferFS, "\033[%2d;%2dm", FG, BG);
		CB_write_u8(&txCBuf, (uint8_t *) UserTxBufferFS, 8);
		va_start(arg, format);
		vsprintf((char*) UserTxBufferFS, format, arg);
		va_end(arg);
		CB_write_u8(&txCBuf, (uint8_t *) UserTxBufferFS, strlen((char*) UserTxBufferFS));
		CB_write_u8(&txCBuf, (uint8_t *) "\033[90;37m", 8);
		osMutexRelease(CDC_TxMutexHandle);
	}
}

void _printn(const char *format, ...)
{
	va_list arg;
	uint8_t UserTxBufferFS[PRINTF_MAX_SIZE];
	if (HOST_PORT_COM_OPEN)
	{
		va_start(arg, format);
		vsprintf((char*) UserTxBufferFS, format, arg);
		va_end(arg);

		if(osMutexAcquire(CDC_TxMutexHandle, 0) == osOK)
		{
			CB_write_u8(&txCBuf, (uint8_t *) UserTxBufferFS, strlen((char*) UserTxBufferFS));
			osMutexRelease(CDC_TxMutexHandle);
		}
	}
}



void _prints(uint8_t* stream, uint32_t len)
{
	if (HOST_PORT_COM_OPEN)
	{
		CDC_Transmit_FS(stream, len);
	}
}

void _printd(const char *format, ...)
{
	va_list arg;
	uint32_t clktime;
	uint8_t UserTxBufferFS[PRINTF_MAX_SIZE];

	if (HOST_PORT_COM_OPEN)
	{
		clktime = HAL_GetTick();
		sprintf((char*) UserTxBufferFS, "[%02lu:%02lu:%02lu.%03lu] ", (clktime/3600000)%100, (clktime/60000)%60, (clktime/1000)%60, clktime%1000);
		va_start(arg, format);
		vsprintf((char*) &UserTxBufferFS[15], format, arg);
		va_end(arg);

		osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
		CB_write_u8(&txCBuf, (uint8_t *) UserTxBufferFS, strlen((char*) UserTxBufferFS));
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
		CB_readUntil_u8(UserRxBufferFS, &rxCBuf, END_CMD_CHAR, PRINTF_MAX_SIZE);
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

void SER_flush(void)
{
	uint8_t UserTxBufferFS[SERIAL_BLOCK_SIZE];
	uint32_t len;
	osMutexAcquire(CDC_TxMutexHandle, osWaitForever);
	len = CB_read_u8(UserTxBufferFS, &txCBuf, SERIAL_BLOCK_SIZE);
	if(len)
	{
		if (!(CDC_Transmit_FS(UserTxBufferFS, len) == USBD_OK))
		{	CB_rewindWritePtr_u8(&txCBuf, len);
		}
	}
	osMutexRelease(CDC_TxMutexHandle);
}
