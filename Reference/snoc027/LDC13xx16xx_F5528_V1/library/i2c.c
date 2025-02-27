

#include "i2c.h"

static uint8_t * rxPointer;
static uint8_t rxIndex;
static uint8_t rxBytes;
static uint8_t * txPointer;
static uint8_t txIndex;
static uint8_t txBytes;
//static uint16_t TimeoutCounter;
static uint8_t done;

static uint8_t buffer[3];

static uint32_t counter;

static void i2c_reset() {
	uint8_t i;
	// toggle SCL to clear state
	I2C_SEL &= ~(I2C_SCL_BIT);
	I2C_DIR |= (I2C_SCL_BIT);
	for (i = 0; i < 5; i++) {
		__delay_cycles(I2C_TOGGLE_DELAY);
		I2C_OUT &= ~(I2C_SCL_BIT);
		__delay_cycles(I2C_TOGGLE_DELAY);
		I2C_OUT |= (I2C_SCL_BIT);
	}
	I2C_SEL |= I2C_SCL_BIT;
	done = TRUE;
}

void i2c_setClock(i2c_clock_t clockSel, uint16_t clockDiv) {
	while (!i2c_ready());
	UCB1CTL1 = clockSel + UCSWRST;
	UCB1BR0 = clockDiv;
	UCB1BR1 = (clockDiv >> 8) & 0xFF;
	UCB1CTL1 &= ~UCSWRST;
}

void i2c_setup() {
	i2c_reset();
	I2C_SEL |= (I2C_SDA_BIT + I2C_SCL_BIT); // assign pins
	UCB1IE &= ~(UCNACKIE+UCALIE+UCTXIE+UCRXIE+UCSTTIE);
	UCB1CTL1 |= UCSWRST;
	UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC; // i2c master, synchronous mode
	i2c_setClock(SMCLK,I2C_CLK_DIV);
}

//static uint8_t i2c_ready() {
//	if ((UCB1CTL1 & UCTXSTP) || (UCB1STAT & UCBBUSY)) {
//		return FALSE;
//	}
//	return TRUE;
//}
//
uint8_t i2c_ready() {
	__delay_cycles(I2C_READY_DELAY);
	counter = 0;
	while ((done == FALSE) || (UCB1CTL1 & UCTXSTP)) {
		if (counter++ > I2C_EXEC_TIMEOUT) {
			i2c_reset();
			return FALSE;
		}
	}
	return TRUE;
}

static void i2c_setAddr(uint8_t addr) {
	UCB1CTL1 |= UCSWRST;
	UCB1I2CSA = addr;
	UCB1CTL1 &= ~UCSWRST;
}

// command does not write a command code or issue a restart.
// i2c read with address
uint8_t i2c_readBytes(uint8_t addr, uint8_t * data, uint8_t length) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	txIndex = 0;
	txBytes = 0;
	rxPointer = &data[0];
	rxBytes = length;
	rxIndex = 0;
	UCB1IE |= UCSTTIE + UCRXIE;					// enable receive and start interrupts
	UCB1CTL1 &= ~UCTR;							// set receive flag
	UCB1CTL1 |= UCTXSTT;						// issue start
	// block until ready
	if (i2c_ready() == FALSE) return FALSE;
	return TRUE;
}

uint8_t i2c_readBytesInBackground(uint8_t addr, uint8_t * data, uint8_t length) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	txIndex = 0;
	txBytes = 0;
	rxPointer = &data[0];
	rxBytes = length;
	rxIndex = 0;
	UCB1IE |= UCSTTIE + UCRXIE;					// enable receive and start interrupts
	UCB1CTL1 &= ~UCTR;							// set receive flag
	UCB1CTL1 |= UCTXSTT;						// issue start
	return TRUE;
}

// same as writebytes with length 1 but write data byte is copied and not referenced
uint8_t i2c_writeByte(uint8_t addr, uint8_t code) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	buffer[0] = code;
	txIndex = 0;
	txPointer = &buffer[0];
	txBytes = 1;
	rxBytes = 0;
	UCB1IE |= (UCALIE+UCTXIE);
	UCB1CTL1 |= UCTR + UCTXSTT;				// issue start
	// block until ready
	if (i2c_ready() == FALSE) return FALSE;
	return TRUE;
}

uint8_t i2c_writeBytes(uint8_t addr, uint8_t * data, uint8_t length) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	txIndex = 0;
	txPointer = &data[0];
	txBytes = length;
	rxBytes = 0;
	UCB1IE |= (UCALIE+UCTXIE);
	UCB1CTL1 |= UCTR + UCTXSTT;				// issue start
	if (i2c_ready() == FALSE) return FALSE;
	return TRUE;
}

uint8_t i2c_writeBytesInBackground(uint8_t addr, uint8_t * data, uint8_t length) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	txIndex = 0;
	txPointer = &data[0];
	txBytes = length;
	rxBytes = 0;
	UCB1IE |= (UCALIE+UCTXIE);
	UCB1CTL1 |= UCTR + UCTXSTT;				// issue start
	return TRUE;
}

// big endian
uint8_t smbus_readWord(uint8_t addr, uint8_t code, uint16_t * data) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	buffer[0] = code;
	txIndex = 0;
	txPointer = &buffer[0];
	txBytes = 1;
	rxPointer = &buffer[1];
	rxBytes = 2;
	UCB1IE |= (UCALIE+UCTXIE);
	UCB1CTL1 |= UCTR + UCTXSTT;				// issue start
	// block until ready
	if (i2c_ready() == FALSE) return FALSE;
	// save data
	* data = buffer[2] | (buffer[1] << 8);
	return TRUE;
}

// code is passed in as first data byte
uint8_t smbus_readBytes(uint8_t addr, uint8_t * data, uint8_t length) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	txIndex = 0;
	txPointer = &data[0];
	txBytes = 1;
	rxPointer = &data[1];
	rxBytes = length;
	UCB1IE |= (UCALIE+UCTXIE);
	UCB1CTL1 |= UCTR + UCTXSTT;				// issue start
	if (i2c_ready() == FALSE) return FALSE;
	return TRUE;
}

// code is passed in as first data byte
uint8_t smbus_readBytesInBackground(uint8_t addr, uint8_t * data, uint8_t length) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	txIndex = 0;
	txPointer = &data[0];
	txBytes = 1;
	rxPointer = &data[1];
	rxBytes = length;
	UCB1IE |= (UCALIE+UCTXIE);
	UCB1CTL1 |= UCTR + UCTXSTT;				// issue start
	return TRUE;
}

// big endian
uint8_t smbus_writeWord(uint8_t addr, uint8_t code, uint16_t data) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	buffer[0] = code;
	buffer[1] = (data >> 8) & 0xFF;
	buffer[2] = data & 0xFF;
	txIndex = 0;
	txPointer = &buffer[0];
	txBytes = 3;
	rxBytes = 0;
	UCB1IE |= (UCALIE+UCTXIE);
	UCB1CTL1 |= UCTR + UCTXSTT;				// issue start
	// block until ready
	if (i2c_ready() == FALSE) return FALSE;
	return TRUE;
}

uint8_t smbus_writeByte(uint8_t addr, uint8_t code, uint8_t data) {
	if (i2c_ready() == FALSE) return FALSE;
	done = FALSE;
	i2c_setAddr(addr);
	buffer[0] = code;
	buffer[1] = data;
	txIndex = 0;
	txPointer = &buffer[0];
	txBytes = 2;
	rxBytes = 0;
	UCB1IE |= (UCALIE+UCTXIE);
	UCB1CTL1 |= UCTR + UCTXSTT;				// issue start
	// block until ready
	if (i2c_ready() == FALSE) return FALSE;
	return TRUE;
}

#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
  switch(__even_in_range(UCB1IV,12))
  {
  case  0:
 	 break;

  case  2:
    // Vector  2: USCI_I2C_UCALIFG - Arbitration Lost interrupt Flag
    UCB1CTL1 |= UCTXSTP;                  			// I2C stop condition
	UCB1IE &= ~(UCTXIE+UCRXIE+UCSTTIE);             // Disable interrupts
	done = TRUE;
	UCB1IFG &= ~UCALIFG;
  	break;

  case  4:
    // Vector  4: USCI_I2C_UCNACKIFG - NAK Condition interrupt Flag
//    UCB1CTL1 |= UCTXSTP;                  			// I2C stop condition
//	UCB1IE &= ~(UCTXIE+UCRXIE+UCSTTIE);             // Disable interrupts
//	done = TRUE;
//	UCB1IFG &= ~UCNACKIFG;
  	break;

  case  6:
    // Vector  6: USCI_I2C_UCSTTIFG - START Condition interrupt Flag
	if ((rxBytes == 1 && txIndex == txBytes)) {		// issue stop flag for single byte reads
		UCB1CTL1 |= UCTXSTP;						//   before receiving data
	}
	UCB1IFG &= ~UCSTTIFG;
  	break;

  case  8:
      // Vector  8: USCI_I2C_UCSTPIFG - STOP Condition interrupt Flag
    break;

  case 10:
	// Vector 10: USCI_I2C_UCRXIFG - USCI Receive Interrupt Flag
    * rxPointer++ = UCB1RXBUF; 						// receive byte to buffer
    if (++rxIndex < rxBytes) {
    	if (rxIndex == (rxBytes - 1)) {				// issue stop
    		UCB1CTL1 |= UCTXSTP;
    	}
    }
    else {											// end receive
    	rxBytes = 0;
    	txIndex = 0;
    	UCB1IE &= ~(UCSTTIE+UCRXIE);
    	done = TRUE;
    }
    UCB1IFG &= ~UCRXIFG;
  	break;

  // Vector 12: USCI_I2C_UCTXIFG - USCI Transmit Interrupt Flag
  case 12:
	if (txIndex < txBytes) {						// transmit byte from buffer
		UCB1TXBUF = * txPointer++;
		txIndex++;
	}
	else if (rxBytes > 0) {								// begin receive
		rxIndex = 0;
		UCB1IE |= UCSTTIE + UCRXIE;					// enable receive and start interrupts
		UCB1CTL1 &= ~UCTR;							// set receive flag
		UCB1CTL1 |= UCTXSTT;						// repeated start
	}
	else {
		txIndex = 0;								// issue stop
		UCB1CTL1 |= UCTXSTP;
		done = TRUE;
	}
	UCB1IFG &= ~UCTXIFG;
    break;

  default:
  	break;
  }
}
