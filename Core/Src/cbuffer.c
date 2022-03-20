#include "stm32f407xx.h"
#include "arm_math.h"
#include <stdlib.h>
#include "cbuffer.h"


uint32_t CB_read_i32(int32_t* oBuf, CB_int32_t* cBuf, uint32_t maxLen)
{
	uint32_t ii = 0;
	while(ii < maxLen && cBuf->read_ptr != cBuf->write_ptr)
	{
		oBuf[ii] = *cBuf->read_ptr;
		if(cBuf->read_ptr == cBuf->end_ptr)
		{	cBuf->read_ptr = cBuf->start_ptr;
		}
		else
		{	cBuf->read_ptr++;
		}
		ii++;
	}
	return ii;
}


uint32_t CB_write_i32(CB_int32_t* cBuf, int32_t* iBuf, uint32_t len)
{
	uint32_t ii = 0;
	uint32_t jj = 0;
	while(ii < len)
	{
		*cBuf->write_ptr = iBuf[ii];
		if(cBuf->write_ptr == cBuf->end_ptr)
		{	cBuf->write_ptr = cBuf->start_ptr;
		}
		else
		{	cBuf->write_ptr++;
		}
		if(cBuf->write_ptr == cBuf->read_ptr)
		{	jj = 0;
		}
		ii++;
		jj++;
	}
	if(jj<ii)
	{
		cBuf->read_ptr = cBuf->write_ptr+1;
	}
	return jj;
}


uint32_t CB_read_u8(uint8_t* oBuf, CB_uint8_t* cBuf, uint32_t maxLen)
{
	uint32_t ii = 0;
	while(ii < maxLen && cBuf->read_ptr != cBuf->write_ptr)
	{
		oBuf[ii] = *cBuf->read_ptr;
		if(cBuf->read_ptr == cBuf->end_ptr)
		{	cBuf->read_ptr = cBuf->start_ptr;
		}
		else
		{	cBuf->read_ptr++;
		}
		ii++;
	}
	return ii;
}


uint32_t CB_write_u8(CB_uint8_t* cBuf, uint8_t* iBuf, uint32_t len)
{
	uint32_t ii = 0;
	uint32_t jj = 0;
	while(ii < len)
	{
		*cBuf->write_ptr = iBuf[ii];
		if(cBuf->write_ptr == cBuf->end_ptr)
		{	cBuf->write_ptr = cBuf->start_ptr;
		}
		else
		{	cBuf->write_ptr++;
		}
		if(cBuf->write_ptr == cBuf->read_ptr)
		{	jj = 0;
		}
		ii++;
		jj++;
	}
	if(jj<ii)
	{
		cBuf->read_ptr = cBuf->write_ptr+1;
	}
	return jj;
}

uint32_t CB_rewindReadPtr_u8(CB_uint8_t* cBuf, uint32_t len)
{
	uint32_t ii = 0;
	while(ii < len && cBuf->read_ptr != cBuf->write_ptr)
	{
		if(cBuf->read_ptr == cBuf->start_ptr)
		{	cBuf->read_ptr = cBuf->end_ptr;
		}
		else
		{	cBuf->read_ptr--;
		}
		ii++;
	}
	return ii;
}


uint32_t CB_readUntil_u8(uint8_t* oBuf, CB_uint8_t* cBuf, uint8_t val, uint32_t maxLen)
{
	uint32_t ii = 0;
	while(cBuf->read_ptr != cBuf->write_ptr && ii < maxLen)
	{
		oBuf[ii] = *cBuf->read_ptr;
		if(cBuf->read_ptr == cBuf->end_ptr)
		{	cBuf->read_ptr = cBuf->start_ptr;
		}
		else
		{	cBuf->read_ptr++;
		}
		if(oBuf[ii] == val)
		{	return ii;
		}
		ii++;
	}
	return ii;
}

uint32_t CB_rewindWritePtr_u8(CB_uint8_t* cBuf, uint32_t len)
{
	uint32_t ii = 0;
	while(ii < len && cBuf->write_ptr != cBuf->read_ptr)
	{
		if(cBuf->write_ptr == cBuf->start_ptr)
		{	cBuf->write_ptr = cBuf->end_ptr;
		}
		else
		{	cBuf->write_ptr--;
		}
		ii++;
	}
	return ii;
}
