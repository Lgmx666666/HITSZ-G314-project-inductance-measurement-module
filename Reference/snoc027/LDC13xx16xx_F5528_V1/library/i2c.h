
#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include <msp430.h>

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define I2C_OUT     P4OUT
#define I2C_IN      P4IN
#define I2C_DIR 	P4DIR
#define I2C_REN 	P4REN
#define I2C_SEL 	P4SEL
#define I2C_SDA_BIT BIT1
#define I2C_SCL_BIT BIT2

#define I2C_CLK 	UCSSEL_2 	// smclk
#define I2C_CLK_DIV 60 			// smclk / 60

#define I2C_EXEC_TIMEOUT 48000     // timeout in ms for i2c background execution
#define I2C_READY_DELAY 30         // delay before checking if state machine is finished
#define I2C_TOGGLE_DELAY 60        // delay between toggling i2c gpio

#define I2C_CLK_MASK UCSSEL_3

typedef enum {
	UCLK = UCSSEL__UCLK,
	ACLK = UCSSEL__ACLK,
	SMCLK = UCSSEL__SMCLK
} i2c_clock_t;

// I2C methods
void i2c_setup();
uint8_t i2c_ready();
void i2c_setClock(i2c_clock_t clockSel, uint16_t clockDiv);

// read/write registers at specified addr
uint8_t i2c_readBytes(uint8_t addr, uint8_t * data, uint8_t length);
uint8_t i2c_readBytesInBackground(uint8_t addr, uint8_t * data, uint8_t length);
uint8_t i2c_writeByte(uint8_t addr, uint8_t code);
uint8_t i2c_writeBytes(uint8_t addr, uint8_t * data, uint8_t length);
uint8_t i2c_writeBytesInBackground(uint8_t addr, uint8_t * data, uint8_t length);

uint8_t smbus_readWord(uint8_t addr, uint8_t code, uint16_t * data);  // Big Endian
uint8_t smbus_readBytes(uint8_t addr, uint8_t * data, uint8_t length);
uint8_t smbus_readBytesInBackground(uint8_t addr, uint8_t * data, uint8_t length);
uint8_t smbus_writeWord(uint8_t addr, uint8_t code, uint16_t data);  // Big Endian
uint8_t smbus_writeByte(uint8_t addr, uint8_t code, uint8_t data);

#endif /* I2C_H_ */

/** @} */
