/** @file
 * @brief Implementation of functions for SPI1.1
 */

#include "spi_1p1_port.h"
#include "spi_1p1.h"

static SPI_1P1_CS_Pin * cs_default = NULL;
static SPI_1P1_CS_Pin * cs_active = NULL;
static uint16_t ctr;

// transmit / receive variables
static uint8_t done;
static uint8_t * rxbuf;
static uint8_t rxlen;
static uint8_t * txbuf;
static uint8_t txlen;
static uint8_t txaddr;

// for data word
static uint8_t wordbuf[2];

#if defined(SPI_INT_VECTOR)

static enum {
	IDLE,
	TRANSMITTING,
	RECEIVING
} state;

// SPI functions
// TODO: better timeout function
// spi_exec() execute SPI command (primes data for interrupt)
// includes a timeout function that should be commented-out
// if the user wants to check using a different method, like
// via a scheduler
static uint8_t spi_exec(SPI_1P1_CS_Pin * cs) {
	cs_active = (cs == NULL) ? cs_default : cs;
	if (cs_active == NULL) return FALSE;
	done = FALSE;
	state = TRANSMITTING;
	(cs_active->select)();
	SPI_TXBUF = txaddr;
	ctr = 0;
	while (done != TRUE) {
		if (ctr++ >= SPI_TIMEOUT_IN_CYCLES) {
			state = IDLE;
			(cs_active->deselect)();
			done = TRUE;
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * This interrupt uses TXIFG and RXIFG as state-machine states.
 * States are transitioned through enabling and disabling interrupts.
 * Timeout is done externally through read/write functions.
 * TODO: Error Handling
 */
#pragma vector=SPI_INT_VECTOR
__interrupt void SPI_ISR(void)
{
  switch(__even_in_range(SPI_IV,4))
  {
    case 0: break;                          // Vector 0 - no interrupt
    case 2:                                 // Vector 2 - RXIFG
      if (state == RECEIVING) { 					// if frame is not finished processing
    	* rxbuf = SPI_RXBUF;				// read data to buffer
    	if (rxlen > 0) {					// continue
    		rxbuf++;
    		rxlen--;
    		SPI_TXBUF = 0x00;
    	}
    	else {
    		(cs_active->deselect)();
    		state = IDLE;
    		done = TRUE;					// finished
    	}
      }
      break;
    case 4:                          		// Vector 4 - TXIFG
      if (state == TRANSMITTING) { 					// if frame is not finished processing
    	if (txlen > 0) {
    		SPI_TXBUF = * txbuf;
    		txbuf++;
    		txlen--;
    	}
    	else if (rxlen > 0) {
    		state = RECEIVING;
    		SPI_TXBUF = 0x00;
    		rxlen--;
    	}
    	else {
    		(cs_active->deselect)();
    		state = IDLE;
    		done = TRUE;					// finished
    	}
      }
      break;
    default: break;
  }
}
#elif defined(SPI_USCI) || defined(SPI_USART)
/** SPI functions, TODO: better timeout function */
/*  SPI_TXRDY() wait until transfer is empty 	 */
#define SPI_TXRDY() 							\
do { 											\
	ctr = 0; 									\
	while (!SPI_TXBUF_EMPTY) {					\
		if (ctr++ >= SPI_TIMEOUT_IN_CYCLES) {	\
			(cs_active->deselect)();			\
			done = TRUE;						\
			return FALSE;						\
		}										\
	}											\
} while (0)
/* SPI_IDLE() wait buffer is idle */
#define SPI_IDLE()	 							\
do { 											\
	ctr = 0; 									\
	while (SPI_BUSY) {							\
		if (ctr++ >= SPI_TIMEOUT_IN_CYCLES) {	\
			(cs_active->deselect)();			\
			done = TRUE;						\
			return FALSE;						\
		}										\
	}											\
} while (0)

// spi_exec() execute SPI command (no interrupt)
static uint8_t spi_exec(SPI_1P1_CS_Pin * cs) {
	cs_active = (cs == NULL) ? cs_default : cs;
	if (cs_active == NULL) return FALSE;
	done = FALSE;
	(cs_active->select)();
	SPI_TXRDY();
	SPI_TXBUF = txaddr;
	while (txlen > 0) {
		SPI_TXRDY();
		SPI_TXBUF = * txbuf;
		txbuf++;
		txlen--;
	}
	while (rxlen > 0) {
		SPI_TXRDY();
		SPI_TXBUF = 0;
		SPI_IDLE();
		* rxbuf = SPI_RXBUF;
		rxbuf++;
		rxlen--;
	}
	SPI_IDLE();
	(cs_active->deselect)();
	done = TRUE;
	return TRUE;
}
#elif defined(SPI_USI)
// SPI functions, TODO: better timeout function
/* SPI_IDLE() wait buffer is idle */
#define SPI_IDLE()	 							\
do { 											\
	ctr = 0; 									\
	while (SPI_BUSY) {							\
		if (ctr++ >= SPI_TIMEOUT_IN_CYCLES) {	\
			(cs_active->deselect)();			\
			done = TRUE;						\
			return FALSE;						\
		}										\
	}											\
} while (0)
// spi_exec() execute SPI command (no interrupt)
static uint8_t spi_exec(SPI_1P1_CS_Pin * cs) {
	cs_active = (cs == NULL) ? cs_default : cs;
	if (cs_active == NULL) return FALSE;
	done = FALSE;
	(cs_active->select)();
	SPI_TXBUF = txaddr;
	USICNT = 8;
	SPI_IDLE();
	while (txlen > 0) {
		SPI_TXBUF = * txbuf;
		USICNT = 8;
		SPI_IDLE();
		txbuf++;
		txlen--;
	}
	while (rxlen > 0) {
		SPI_TXBUF = 0;
		USICNT = 8;
		SPI_IDLE();
		* rxbuf = SPI_RXBUF;
		rxbuf++;
		rxlen--;
	}
	(cs_active->deselect)();
	done = TRUE;
	return TRUE;
}
#else
#error "Support for selected SPI port not found"
#endif

#if defined(SPI_USCI)
void spi_setup() {
	done = TRUE;
	SPI_CTL1 |= UCSWRST;                    // **Disable USCI state machine**
	SPI_CTL0 |= UCMST+UCMSB+UCSYNC+UCCKPL;  // 3-pin, 8-bit SPI master
	SPI_CTL1 |= SPI_UCSSEL;                 // set clock
	SPI_BR0 = SPI_BRLOVAL;                  // set prescaler value
	SPI_BR1 = SPI_BRHIVAL;
	SPI_MCTL = 0;							// no modulation

	SPI_MOSI_SEL |= SPI_MOSI_BIT;			// set SPI pin function
	SPI_MISO_SEL |= SPI_MISO_BIT;
	SPI_CLK_SEL  |= SPI_CLK_BIT;

	SPI_CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
	#if defined(SPI_INT_VECTOR)
	state = IDLE;
	SPI_IE |= UCRXIE + UCTXIE;				// Enable Interrupts
	#endif
}
#elif defined(SPI_USART)
void spi_setup() {
	SPI_CTL0 |= SWRST;                    // **Disable USCI state machine**
	SPI_CTL0 |= CHAR+SYNC+MM; 			  // 8-bit SPI master synchronous
	SPI_CTLT |= SPI_USARTSEL+STC; 	      // 3-pin SPI, set clock source

	SPI_BR0 = SPI_BRLOVAL;                // set prescaler value
	SPI_BR1 = SPI_BRHIVAL;
	SPI_MCTL = 0;						  // no modulation

	SPI_MOSI_SEL |= SPI_MOSI_BIT;		  // set SPI pin function
	SPI_MISO_SEL |= SPI_MISO_BIT;
	SPI_CLK_SEL  |= SPI_CLK_BIT;

	SPI_CTL0 &= ~SWRST;                   // **Initialize USCI state machine**
}
#elif defined(SPI_USI)
void spi_setup() {
	SPI_CTL0 |= USISWRST;                 	  // **Disable USCI state machine**
	SPI_CTL0 |= USIPE7+USIPE6+USIPE5+USIMST+USIOE;  // 8-bit SPI master synchronous
//	SPI_CTL1 |= USIIE;
	SPI_CKCTL |= SPI_USISEL + SPI_USIDIV;  	  // set clock source, prescaler

	SPI_MOSI_SEL |= SPI_MOSI_BIT;		  // set SPI pin function
	SPI_MISO_SEL |= SPI_MISO_BIT;
	SPI_CLK_SEL  |= SPI_CLK_BIT;

	SPI_CTL0 &= ~USISWRST;                // **Initialize USCI state machine**
	USICNT = 8;                               // init-load counter
}
#else
#error "Support for selected SPI port not found"
#endif

uint8_t spi_cs_setup(SPI_1P1_CS_Pin * * csArray, uint8_t len) {
	uint8_t i;
	if (len == 0) return FALSE;
	for (i=0; i < len; i++) {
		if (csArray[i]->setup == NULL) return FALSE;
		(csArray[i]->setup)();
	}
	return TRUE;
}
uint8_t spi_cs_setDefault(SPI_1P1_CS_Pin * cs) {
	if (cs == NULL) return FALSE;
	cs_default = cs;
	return TRUE;
}

uint8_t spi_readByte(SPI_1P1_CS_Pin * cs, uint8_t addr, uint8_t * data) {
	if (done != TRUE) return FALSE;
	txlen = 0;
	rxlen = 1;
	rxbuf = data;
	txaddr = addr | SPI_RWBIT;
	if (!spi_exec(cs)) return FALSE;
	return TRUE;
}
uint8_t spi_readWord(SPI_1P1_CS_Pin * cs, uint8_t addr, uint16_t * data) {
	if (done != TRUE) return FALSE;
	txlen = 0;
	rxlen = 2;
	rxbuf = &wordbuf[0];
	txaddr = addr | SPI_RWBIT;
	if (!spi_exec(cs)) return FALSE;
	* data = (wordbuf[0] << 8) | wordbuf[1];  // Big Endian
	return TRUE;
}
uint8_t spi_readBytes(SPI_1P1_CS_Pin * cs, uint8_t addr, uint8_t * buffer, uint8_t len) {
	if (done != TRUE) return FALSE;
	txlen = 0;
	rxlen = len;
	rxbuf = buffer;
	txaddr = addr | SPI_RWBIT;
	if (!spi_exec(cs)) return FALSE;
	return TRUE;
}
uint8_t spi_writeByte(SPI_1P1_CS_Pin * cs, uint8_t addr, uint8_t data) {
	if (done != TRUE) return FALSE;
	wordbuf[0] = data;          // copy from stack to memory
	txlen = 1;
	txbuf = &wordbuf[0];
	rxlen = 0;
	txaddr = addr & ~SPI_RWBIT;
	if (!spi_exec(cs)) return FALSE;
	return TRUE;
}
uint8_t spi_writeWord(SPI_1P1_CS_Pin * cs, uint8_t addr, uint16_t data) {
	if (done != TRUE) return FALSE;
	wordbuf[0] = data >> 8;    // Big Endian
	wordbuf[1] = data & 0xFF;
	txlen = 2;
	txbuf = &wordbuf[0];
	rxlen = 0;
	txaddr = addr & ~SPI_RWBIT;
	if (!spi_exec(cs)) return FALSE;
	return TRUE;
}
uint8_t spi_writeBytes(SPI_1P1_CS_Pin * cs, uint8_t addr, uint8_t * buffer, uint8_t len) {
	if (done != TRUE) return FALSE;
	txlen = len;
	txbuf = buffer;
	rxlen = 0;
	txaddr = addr & ~SPI_RWBIT;
	if (!spi_exec(cs)) return FALSE;
	return TRUE;
}

