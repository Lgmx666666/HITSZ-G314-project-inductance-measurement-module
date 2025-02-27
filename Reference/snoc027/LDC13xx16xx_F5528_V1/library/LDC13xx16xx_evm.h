/** @file
 * @brief Interface functions for LDC1000 evm.
 * @defgroup ldc1000evm ldc1000evm
 * @{
 * @ingroup hardware
 *
 * @brief <b>LDC1000 Interface Library Code</b> @n@n
 * This file contains functions that allow control of the peripherals attached
 * to the LDC1000 EVM interface.  Parameters in evm_init() and evm_test() are
 * designed for the coil in the official LDC1000 EVM.  Users desiring to add
 * multi-channel functionality or different coils should use LDC13xx16xx_evm.c as a
 * primer for interfacing the SPI 1.1 Library.
 *
 * @author
 * Charles Cheung (chuck@ti.com)
 */

#ifndef LDC13xx16xx_EVM_H_
#define LDC13xx16xx_EVM_H_

#include "i2c.h"
//#include "spi_1p1.h"
#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/** @name - TIMING Definition - */
//@{
#define EVM_TIME_1MS   24000    /**< Number of Cycles for 1ms */
//@}

/** @name - SHUTDOWN Pin Definition - */
//@{
#define EVM_SHUTDOWN_OUT 	P1OUT	/**< Define SHUTDOWN pin output register */
#define EVM_SHUTDOWN_DIR 	P1DIR	/**< Define SHUTDOWN pin direction register */
#define EVM_SHUTDOWN_SEL 	P1SEL	/**< Define SHUTDOWN pin selection register */
#define EVM_SHUTDOWN_BIT 	BIT2	/**< Define SHUTDOWN pin bit register */
#define EVM_SHUTDOWN_REN    P1REN
#define EVM_SHUTDOWN_IN     P1IN
//@}

/** @name - ADDR Pin Definition - */
//@{
#define EVM_ADDR_OUT 	P1OUT	/**< Define ADDR pin output register */
#define EVM_ADDR_DIR 	P1DIR	/**< Define ADDR pin direction register */
#define EVM_ADDR_SEL 	P1SEL	/**< Define ADDR pin selection register */
#define EVM_ADDR_BIT 	BIT3	/**< Define ADDR pin bit register */
//@}

/** @name - INT Pin Definition - */
//@{
#define EVM_INT_DIR 	P1DIR	/**< Define output clock pin direction register */
#define EVM_INT_IN	    P1IN    /**< Define output clock input status */
#define EVM_INT_SEL 	P1SEL	/**< Define output clock pin selection register */
#define EVM_INT_BIT 	BIT1	/**< Define output clock pin bit register */
#define EVM_INT_IE      P1IE
#define EVM_INT_IES     P1IES
#define EVM_INT_IFG		P1IFG
#define EVM_INT_VECTOR  PORT1_VECTOR
//@}

/** @name - GREEN LED Pin Definition - */
//@{
#define EVM_GRN_LEDOUT            P5OUT	/**< Define green led pin output register */
#define EVM_GRN_LEDDIR            P5DIR	/**< Define green led pin direction register */
#define EVM_GRN_LEDBIT            BIT0	/**< Define green led pin bit register */
//@}
/** @name - RED LED Pin Definition - */
//@{
#define EVM_RED_LEDOUT            P5OUT	/**< Define red led pin output register */
#define EVM_RED_LEDDIR            P5DIR	/**< Define red led pin direction register */
#define EVM_RED_LEDBIT            BIT1	/**< Define red led pin bit register */
//@}

/** @name - LED Function Macros - */
//@{
#define EVM_GRN_LED_ON()	      (EVM_GRN_LEDOUT |= EVM_GRN_LEDBIT)  /**< Green LED ON */
#define EVM_GRN_LED_OFF()         (EVM_GRN_LEDOUT &= ~EVM_GRN_LEDBIT) /**< Green LED OFF */
#define EVM_RED_LED_ON()	      (EVM_RED_LEDOUT |= EVM_RED_LEDBIT)  /**< Red LED ON */
#define EVM_RED_LED_OFF()         (EVM_RED_LEDOUT &= ~EVM_RED_LEDBIT) /**< Red LED OFF */
//@}

/** @name - Defaults Settings - */
//@{
#define EVM_MIN_I2CADDR     0x2A
#define EVM_MAX_I2CADDR     0x2B
#define EVM_DEFAULT_I2CADDR EVM_MIN_I2CADDR
#define EVM_DEFAULTS_SIZE 24 // 13 registers, 0x08 - 0x14
//@}

//typedef enum {
//	OFF = 0x0000,
//	DIV4 = EVM_LDCLK_DIV4,
//	DIV8 = EVM_LDCLK_DIV8,
//	DIV16 = EVM_LDCLK_DIV16,
//	DIV32 = EVM_LDCLK_DIV32
//} evm_ldclk_state_t;

typedef enum {
	ALLTOGGLE = 0x04,
	GREEN = 0x01,
	RED = 0x02,
	ALLON = 0x03,
	ALLOFF = 0x00,
	CYCLE = 0x05
} evm_led_state_t;

/** Set LED State
Set the current LED state: TOGGLE, GREEN, RED, ON, OFF, CYCLE
@param LED state
@return the internal state of the LEDs as right-aligned bits (maximum 8)
@remarks
The current state is a static variable internal to this library.
*/
uint8_t evm_LED_Set(evm_led_state_t state);
/** Read LED State
Read the current LED state
@return the internal state of the LEDs as right-aligned bits (maximum 8)
@remarks
The current state is a static variable internal to this library.
*/
uint8_t evm_LED_State();

/** Initialize EVM
Initialize LDCLK, SPI, LDC1000
@remarks
Requires that TRUE == 1 and FALSE == 0
@return TRUE if initialized, FALSE if an error occurred
*/
uint8_t evm_init();
/** Process DRDY interrupt flag.
@remarks
Should be called whenever program is idle.  User can modify as task for scheduler.
*/
void evm_processDRDY();
/** Read Frequency Count
Read the last updated data ready values
@param ch channel
@param buffer to save value
@return length of buffer written
@remarks
Data is raw and includes error codes
*/
uint8_t evm_readFreq(uint8_t ch, uint8_t * buffer);
/** Save default values to flash memory
@param buffer values to save, expect length to be EVM_DEFAULTS_SIZE
@remarks
Does not do crc check.  Last byte must be CRC
*/
void evm_saveDefaults(uint8_t * buffer);
/** Read default values
@param buffer destination for default values
@return length of bytes written to destination
*/
uint8_t evm_readDefaults(uint8_t offset,uint8_t * buffer,uint8_t size);
/** Gets the chip select pin structure
Gets the chip select pin structure from channel number lookup
@param channel chip select channel
@return pointer to chip select pin structure if found, else NULL
*/
//SPI_1P1_CS_Pin * evm_get_cs (uint8_t channel);

uint8_t evm_changeAddr(uint8_t addr);

//pre PLL MCU Clock
void init_Clock_prePLL();

#endif /* LDC13xx16xx_EVM_H_ */

/** @} */
