/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include <stdarg.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include <stdbool.h>
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
static _Bool HOST_PORT_COM_OPEN 	= false;
static _Bool CDC_RX_DATA_PRINT 		= true;
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
_Bool CDC_RX_DATA_PENDING = false;		// Serial data pending flag
static uint8_t CmdStr[APP_RX_DATA_SIZE];      	// Full input command line for argument reading
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:
    {
		USBD_SetupReqTypedef *req = (USBD_SetupReqTypedef*) pbuf;
		if ((req->wValue & 0x0001) != 0)
		{
			HOST_PORT_COM_OPEN = true;
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
		}
		else
		{
			HOST_PORT_COM_OPEN = false;
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		}
		break;
    }
	case CDC_SEND_BREAK:
	{
		HOST_PORT_COM_OPEN = false;
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		break;
	}
  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
		uint8_t result = USBD_OK;;
		static uint8_t txLen;
		static uint8_t rxLen;
		static uint8_t VT100cmdSeq;
		static uint8_t rxBufferFS[APP_RX_DATA_SIZE];
		static uint8_t txBufferFS[APP_TX_DATA_SIZE];
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);

		/* Get data from serial com */
		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, Buf);
		if ((result = USBD_CDC_ReceivePacket(&hUsbDeviceFS)) != USBD_OK)
		{
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			return result;
		}

		for (uint8_t i = 0; i < (*Len); i++)
		{
			/* Avoid buffer overflow */
			if (rxLen == APP_RX_DATA_SIZE)
			{	rxLen = 0;
			}
			if (txLen + 17 >= APP_RX_DATA_SIZE)
			{	txLen = 0;
			}

			/* Avoid VT100 cmd sequences (4 * uint8_t)*/
			if(Buf[i] == '\033')
			{	VT100cmdSeq = 4;
			}

			if(!VT100cmdSeq) // avoid VT100cmd
			{
				/* If Backspace key: clear the last char */
				if (Buf[i] == '\b')
				{
					if(rxLen && CDC_RX_DATA_PRINT)
					{
						memcpy(&txBufferFS[txLen], "\b \b", 3);
						txLen += 3;
						rxLen--;
					}
				}
				/* Else if Enter key: add a \n to terminal and extract output buffer */
				else if (Buf[i] == '\r' || Buf[i] == '\0')
				{
					if(rxLen)
					{
						rxBufferFS[rxLen++] = '\r';
						rxBufferFS[rxLen++] = '\0';
						memcpy(UserRxBufferFS, rxBufferFS, rxLen);
						CDC_RX_DATA_PENDING = 1;
						rxLen = 0;
					}
					if (CDC_RX_DATA_PRINT)
					{
						memcpy(&txBufferFS[txLen], "\r\n\033[34;40m@> \033[0m", 17);
						txLen += 17;
					}
				}
				/* Else get the character */
				else
				{
					if (CDC_RX_DATA_PRINT)
					{
						txBufferFS[txLen++] = Buf[i];
					}
					rxBufferFS[rxLen++] = Buf[i];
				}
			}
			else
			{	VT100cmdSeq--;
			}
		}

		/* Send result to terminal */
		if(CDC_RX_DATA_PRINT && HOST_PORT_COM_OPEN)
		{
			if(CDC_Transmit_FS(txBufferFS, txLen) == USBD_OK)
			{	txLen = 0;
			}
		}

		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

		return result;
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
  /* USER CODE END 7 */
  return result;
}

/**
  * @brief  CDC_TransmitCplt_FS
  *         Data transmited callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  /* USER CODE END 13 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

void _printf(const char *format, ...)
{
	va_list arg;
	if (HOST_PORT_COM_OPEN)
	{
		va_start(arg, format);
		vsprintf((char*) UserTxBufferFS, format, arg);
		va_end(arg);
		CDC_Transmit_FS(UserTxBufferFS, strlen((char*) UserTxBufferFS));
	}
}

void _cprintf(const char *format, ...)
{
	va_list arg;
	uint32_t clktime;
	if (HOST_PORT_COM_OPEN)
	{
		clktime = HAL_GetTick();
		va_start(arg, format);
		sprintf((char*) UserTxBufferFS, "\r[%02lu:%02lu:%02lu.%03lu] ", (clktime/3600000)%100, (clktime/60000)%60, (clktime/1000)%60, clktime%1000);
		vsprintf((char*) &UserTxBufferFS[16], format, arg);
		va_end(arg);
		CDC_Transmit_FS(UserTxBufferFS, strlen((char*) UserTxBufferFS));
		if(CDC_RX_DATA_PRINT)
		{
			CDC_Transmit_FS((uint8_t *) "\033[34;40m@> \033[0m", 15);
		}
	}
}

void _wprintf(const char *format, ...)
{
	va_list arg;
	uint32_t clktime;
	if (HOST_PORT_COM_OPEN)
	{
		CDC_SetColor(33, 40);
		clktime = HAL_GetTick();
		va_start(arg, format);
		sprintf((char*) UserTxBufferFS, "\r[%02lu:%02lu:%02lu.%03lu] ", (clktime/3600000)%100, (clktime/60000)%60, (clktime/1000)%60, clktime%1000);
		vsprintf((char*) &UserTxBufferFS[16], format, arg);
		va_end(arg);
		CDC_Transmit_FS(UserTxBufferFS, strlen((char*) UserTxBufferFS));
		if(CDC_RX_DATA_PRINT)
		{
			CDC_Transmit_FS((uint8_t *) "\033[34;40m@> ", 11);
		}
		CDC_SetDefaultColor();
	}
}

void _eprintf(const char *format, ...)
{
	va_list arg;
	uint32_t clktime;
	if (HOST_PORT_COM_OPEN)
	{
		CDC_SetColor(31, 40);
		clktime = HAL_GetTick();
		va_start(arg, format);
		sprintf((char*) UserTxBufferFS, "\r[%02lu:%02lu:%02lu.%03lu] ", (clktime/3600000)%100, (clktime/60000)%60, (clktime/1000)%60, clktime%1000);
		vsprintf((char*) &UserTxBufferFS[16], format, arg);
		va_end(arg);
		CDC_Transmit_FS(UserTxBufferFS, strlen((char*) UserTxBufferFS));
		if(CDC_RX_DATA_PRINT)
		{
			CDC_Transmit_FS((uint8_t *) "\033[34;40m@> ", 11);
		}
		CDC_SetDefaultColor();
	}
}

void _scanf(const char *format, ...)
{
	while (!CDC_RX_DATA_PENDING && HOST_PORT_COM_OPEN)
	{
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	}
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	if (CDC_RX_DATA_PENDING)
	{
		va_list arg;
		va_start(arg, format);
		vsscanf((char*) UserRxBufferFS, format, arg);
		va_end(arg);
		CDC_RX_DATA_PENDING = false;
	}
}

void CDC_Clear(void)
{
	if(CDC_RX_DATA_PRINT && HOST_PORT_COM_OPEN)
	{	CDC_Transmit_FS((uint8_t *)"\033[2J\r\033[34;40m@> \033[0m", 20);
	}
	else if(HOST_PORT_COM_OPEN)
	{	CDC_Transmit_FS((uint8_t *)"\033[2J", 4);
	}
}

void CDC_clearLine(void)
{
	if(HOST_PORT_COM_OPEN)
	{	CDC_Transmit_FS((uint8_t *)"\033[K", 3);
	}
}

void CDC_SetColor(uint8_t FG, uint8_t BG)
{
	_printf("\033[%2d;%2dm", FG, BG);
}

void CDC_SetDefaultColor(void)
{
	if(HOST_PORT_COM_OPEN)
	{	CDC_Transmit_FS((uint8_t *)"\033[0m", 4);
	}
}

void CDC_SetPos(uint16_t x, uint16_t y)
{
	_printf("\033[%d;%dH", y, x);
}

void CDC_Move(int16_t x, int16_t y)
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

void CDC_rxPrintf_ON(void)
{
	if(HOST_PORT_COM_OPEN)
	{	CDC_Transmit_FS((uint8_t *)"\r\033[34;40m@> \033[0m", 16);
	}
	CDC_RX_DATA_PRINT = true;
}

void CDC_rxPrintf_OFF(void)
{
	if(HOST_PORT_COM_OPEN)
	{	CDC_Transmit_FS((uint8_t *)"\r   \r", 5);
	}
	CDC_RX_DATA_PRINT = false;
}

uint8_t CDC_getCmdArg(const char *format, ...)
{
	uint8_t nArg;
	va_list arg;
	va_start(arg, format);
	nArg = vsscanf((char*) CmdStr, format, arg);
	va_end(arg);
	return nArg;
}

void CDC_getCmd(uint8_t *Task, char **cmdList, uint8_t nTask)
{
	char cmd[APP_RX_DATA_SIZE];
	static _Bool OLD_HOST_PORT_COM_OPEN = false;
	_Bool cmdFnd = false;
	if(HOST_PORT_COM_OPEN && CDC_RX_DATA_PENDING)
	{
		/* Read and record command */
		strcpy((char *)CmdStr, (char *)UserRxBufferFS);
		sscanf((char *)CmdStr, "%s", cmd);
		CDC_RX_DATA_PENDING = false;

		/* Select Task */
		if(!strcmp(cmd, cmdList[0]))
		{
			*Task = 0;
			cmdFnd = true;
		}
		else
		{
			for(uint8_t i = 1; i < nTask; i++)
			{
				if(!strcmp(cmd, cmdList[i]) && !(*Task))
				{
					*Task = i;
					cmdFnd = true;
				}
			}
		}
		if(!cmdFnd)
		{
			if (!strcmp(cmd, "clear"))
			{	CDC_Clear();
			}
			else
			{	_wprintf("/?\\ Command \"%s\" not found !\r\n", cmd);
			}
		}
	}
	else if (!OLD_HOST_PORT_COM_OPEN &&  HOST_PORT_COM_OPEN)
	{
		*Task = 0; // Default Task when serial com port switch to open
		CDC_rxPrintf_OFF();
		CDC_Clear();
		_printf("\033[0m"); // reset color to default VT100 cmd
		_printf("\r\n\033[19C======================"
				"\r\n\033[19C***\033[30;47m RTOS DSP V%d.%02d \033[0m***"
				"\r\n\033[19C======================\r\n\n", MAJ_VERSION, MIN_VERSION);
		CDC_rxPrintf_ON();
	}
	else if (OLD_HOST_PORT_COM_OPEN && !HOST_PORT_COM_OPEN)
	{
		*Task = 1; // Default Task when serial comport switch to close
	}
	OLD_HOST_PORT_COM_OPEN = HOST_PORT_COM_OPEN;
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
