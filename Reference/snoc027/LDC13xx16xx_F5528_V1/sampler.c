
//#include <stdint.h>
//#include <cstring>
//#include <stdio.h>

#include "LDC1000_cmd.h"
#include "LDC1000_evm.h"
#include "spi_1p1.h"

#include "sampler.h"

extern uint8_t bufferX[], bufferY[];
extern uint8_t *mbuf, *ubuf;
extern uint8_t LDC1000_deviceMask;

static uint8_t done = 0;
static uint8_t isr_flag = 0;
static uint16_t index = 0;
static uint8_t tbuf[5] = {0x00, 0x00, 0x00, 0x00, 0x00} ;
//******************************************************************************

static void sampler_storeBufToArray(uint8_t *buffer, uint16_t idx, uint8_t *mb)
{
  mb[idx] = buffer[1]; // MSB
  mb[idx+1] = buffer[0]; // LSB
  // HACK to discard L byte
  mb[idx+(BUF_LENGTH/2)] = buffer[3]; // Midbyte
  mb[idx+(BUF_LENGTH/2)+1] = buffer[2]; // LSB
}

void sampler_init(uint16_t time)
{
	done = 0;
	SAMPLER_CCR0 = time;                                 // Initialize for 1sec interval for 1MHz clock
//  SAMPLER_EX0 = TAIDEX_3;                              // Divide clock (selected below) further by 8
	SAMPLER_CTL = TASSEL_2 + MC_1 + TACLR + ID_2;     // SMCLK, upmode, clear TAR, divide by 4 (6MHz)
}
void sampler_start() {
	isr_flag = 0;
	index = 0;
	SAMPLER_CCTL0 |= CCIE; // enable continuous sampling timer interrupt (TA0CCR0)
}
void sampler_stop() {
	SAMPLER_CCTL0 &= ~CCIE; // disable continuous sampling timer interrupt (TA0CCR0)
}
uint8_t sampler_isDone() {
	if (done == 1) {
		done = 0;
		return 1;
	}
	return 0;
}

// Timer interrupt service routine
#pragma vector=SAMPLER_VECTOR
__interrupt void SAMPLER_ISR (void)
{
	isr_flag = 1;
}

// called by main loop / scheduler
void sampler_process(void) {
//	uint8_t j;
	if (isr_flag) {
		//	  for (j = 0; j < EVM_CS_TOTAL; j++) {
		//		  if ((LDC1000_deviceMask >> j) & 0x1) {
		//			  if (spi_readBytes(evm_get_cs(j),LDC1000_CMD_PROXLSB,&tbuf[0],4) == TRUE) {
		//				  store_buffer_to_vcmbuf_array(&tbuf[0], mbx, mbuf);
		//			  }
		//		  }
		//	  }
		if (spi_readBytes(NULL,LDC1000_CMD_PROXLSB,&tbuf[0],5) == TRUE) {
			sampler_storeBufToArray(&tbuf[0], index, mbuf);
		}
		index += 2;
		// 6us delay: 1us = 24cycles at 24MHz system clock
		// Function store_buffer_to_vcmbuf_array takes 5-6us
		// __delay_cycles(144);
		if (index >= BUF_LENGTH/2)
		{
			index = 0;
			done = 1;
			ubuf = mbuf;
			if (mbuf == bufferX)
			  mbuf = bufferY;
			else
			  mbuf = bufferX;
		}
		isr_flag = 0;
	}
}
