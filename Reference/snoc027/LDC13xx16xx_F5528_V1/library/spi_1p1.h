/** @file
 * @brief Interface functions for SPI1.1.
 * @defgroup spi_1p1 spi_1p1
 * @{
 * @ingroup peripherals
 *
 * @brief <b>MSP430 / SPI1.1 Interface Library Code</b> @n@n
 * This file contains functions that allow the MSP430 device to access the SPI
 * interface of devices supporting the internal SPI 1.1 Specification for
 * Precision Signal Path products.
 *
 * @remarks
 * Edit this file to configure SPI pin, port, baud rate, and timeout definitions.
 * If a peripheral on a different port is used, all Registers must be redefined.
 * Sets of register definitions are included in spi_1p1_port.h.
 * Multiple CS pins are supported.  Simply define them by passing a pointer
 * to an array or single SPI_1P1_CS_Pin structure using spi_cs_setup(cs,len).
 * If a single one is used, then it is possible to call SPI specific-peripheral
 * methods by passing in a NULL as cs, as long as the default cs is set using
 * spi_cs_setDefault(cs).
 *
 * @author
 * Charles Cheung (chuck@ti.com)
 */

#include <stdint.h>
#include <msp430.h>

#ifndef SPI_1P1_H_
#define SPI_1P1_H_

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/** @name - SPI1.1 Pin Configuration Registers - */
//@{
// this section should be edited manually
/* SDO */
#define SPI_MOSI_BIT  BIT4		/**< Bit */
#define SPI_MOSI_SEL  P4SEL     /**< Primary Pin Selection */
//#define SPI_MOSI_SEL2 P1SEL2    /**< Secondary Pin Selection (uncomment to set to 1) */
#define SPI_MOSI_DIR  P4DIR		/**< Port Direction */
#define SPI_MOSI_IN   P4IN		/**< Port Input */
/* SDI */
#define SPI_MISO_BIT  BIT5		/**< Bit */
#define SPI_MISO_SEL  P4SEL		/**< Primary Pin Selection */
//#define SPI_MISO_SEL2 P1SEL2    /**< Secondary Pin Selection (uncomment to set to 1) */
#define SPI_MISO_DIR  P4DIR		/**< Port Direction */
#define SPI_MISO_IN   P4IN		/**< Port Input */
/* SCLK */
#define SPI_CLK_BIT   BIT0		/**< Bit */
#define SPI_CLK_SEL   P4SEL		/**< Primary Pin Selection */
//#define SPI_CLK_SEL2  P1SEL2    /**< Secondary Pin Selection (uncomment to set to 1) */
#define SPI_CLK_DIR   P4DIR		/**< Port Direction */
//@}

/* comment out to disable interrupts */
//#define SPI_ENABLE_INTERRUPTS

/** @name - SPI1.1 Port Selection - */
/* un-comment only ONE port to select */
//@{
//#define SPI_USCIA0 /**< USCIA0 */
#define SPI_USCIA1 /**< USCIA1 */
//#define SPI_USCIA2 /**< USCIA2 */
//#define SPI_USCIA3 /**< USCIA3 */
//#define SPI_USCIB0 /**< USCIB0 */
//#define SPI_USCIB1 /**< USCIB1 */
//#define SPI_USCIB2 /**< USCIB2 */
//#define SPI_USCIB3 /**< USCIB3 */
//#define SPI_USART0 /**< USART0 */
//#define SPI_USART1 /**< USART1 */
//#define SPI_USI /**< USI */
//@}

/** @name - SPI1.1 Control Settings - */
//@{
#define SPI_USISEL     USISSEL_2			/**< Set the clock source */
#define SPI_USARTSEL   SSEL1 				/**< Set the clock source */
#ifdef UCSSEL__ACLK
#define SPI_UCSSEL 	   UCSSEL__ACLK 		/**< Set the clock source */
#else
#define SPI_UCSSEL	   UCSSEL_1     		/**< Set the clock source */
#endif
// settings below assume clock source = 24MHz
#define SPI_BRLOVAL    6 			 		/**< Set the low byte of the prescaler (div/6) */
#define SPI_BRHIVAL    0 			 		/**< Set the high byte of the prescaler */
#ifdef USIDIV_8
#define SPI_USIDIV     USIDIV_8				/**< Set the prescaler (div/8) */
#else
#define SPI_USIDIV     USIDIV0+USIDIV1		/**< Set the prescaler (div/8) */
#endif
//@}
/** @name - SPI1.1 Software Settings - */
//@{
#define SPI_RWBIT 		BIT7				/**< Write - Low, Read - High */
#define SPI_TIMEOUT_IN_CYCLES 65535			/**< Timeout, set to be around 13MS */
//@}

/** Serial Interface Chip Select (CS) Pin structure
* Must provide pointers to
* setup, select, and de-select functions.
*/
typedef struct {
	void (*setup) ();    /**< pointer to setup function (e.g. setup CS port to GPIO and bias) */
	void (*select) ();   /**< pointer to select function (asserts CS) */
	void (*deselect) (); /**< pointer to de-select function (de-asserts CS) */
} SPI_1P1_CS_Pin;

/** setups and registers CS pin(s)
@remarks
All CS pins must be initialized with this function.
This function only performs NULL validation.
@param csArray pointer to array of SPI_1P1_CS_Pin
@param len total number of pins
@return false if NULL cs pointers found, TRUE otherwise
@sa spi_cs_setDefault
*/
uint8_t spi_cs_setup(SPI_1P1_CS_Pin * * csArray, uint8_t len);

/** sets default CS pin
@return false if NULL cs pointer found, TRUE otherwise
@param cs pointer to default s_SPI_1P1_CS_Pin
@sa spi_cs_setup
*/
uint8_t spi_cs_setDefault(SPI_1P1_CS_Pin * cs);

/** Initializes SPI Registers
Sets the SPI to be an 8-bit Master clocked by user-defined Control Settings
@sa spi_cs_setup
*/
void spi_setup();

/** SPI read byte
Reads a byte from SPI into data
@remarks
CS pins and SPI must be setup prior to calling this function.  See spi_setup(),
spi_cs_setup(), and spi_cs_setDefault()
@param cs pointer to cs pin structure to call. if NULL, uses the default cs pointer
@param addr register address
@param data pointer to byte
@return false if FAILED, true if SUCCESS
@sa spi_readWord, spi_readBytes
*/
uint8_t spi_readByte(SPI_1P1_CS_Pin * cs, uint8_t addr, uint8_t * data);

/** SPI read word
Reads a 16-bit word from SPI into data
@remarks
CS pins and SPI must be setup prior to calling this function.  See spi_setup(),
spi_cs_setup(), and spi_cs_setDefault()
@param cs pointer to cs pin structure to call. if NULL, uses the default cs pointer
@param addr register address
@param data pointer to 16-bit word, Big Endian
@return false if FAILED, true if SUCCESS
@sa spi_readByte, spi_readBytes
*/
uint8_t spi_readWord(SPI_1P1_CS_Pin * cs, uint8_t addr, uint16_t * data);  // Big Endian

/** SPI read bytes
Reads multiple bytes from SPI into buffer
@remarks
CS pins and SPI must be setup prior to calling this function.  See spi_setup(),
spi_cs_setup(), and spi_cs_setDefault()
@param cs pointer to cs pin structure to call. if NULL, uses the default cs pointer
@param addr register address
@param buffer pointer to byte buffer
@param len number of bytes to read into buffer
@return false if FAILED, true if SUCCESS
@sa spi_readByte, spi_readWord
*/
uint8_t spi_readBytes(SPI_1P1_CS_Pin * cs, uint8_t addr, uint8_t * buffer, uint8_t len);

/** SPI write byte
Writes a byte from data to SPI
@remarks
CS pins and SPI must be setup prior to calling this function.  See spi_setup(),
spi_cs_setup(), and spi_cs_setDefault()
@param cs pointer to cs pin structure to call. if NULL, uses the default cs pointer
@param addr register address
@param data pointer to byte
@return false if FAILED, true if SUCCESS
@sa spi_writeWord, spi_writeBytes
*/
uint8_t spi_writeByte(SPI_1P1_CS_Pin * cs, uint8_t addr, uint8_t data);

/** SPI write word
Writes a 16-bit word from data to SPI
@remarks
CS pins and SPI must be setup prior to calling this function.  See spi_setup(),
spi_cs_setup(), and spi_cs_setDefault()
@param cs pointer to cs pin structure to call. if NULL, uses the default cs pointer
@param addr register address
@param data pointer to 16-bit word, Big Endian
@return false if FAILED, true if SUCCESS
@sa spi_writeByte, spi_writeBytes
*/
uint8_t spi_writeWord(SPI_1P1_CS_Pin * cs, uint8_t addr, uint16_t data);  // Big Endian

/** SPI write bytes
Writes multiple bytes from buffer to SPI
@remarks
CS pins and SPI must be setup prior to calling this function.  See spi_setup(),
spi_cs_setup(), and spi_cs_setDefault()
@param cs pointer to cs pin structure to call. if NULL, uses the default cs pointer
@param addr register address
@param buffer pointer to byte buffer
@param len number of bytes in buffer
@return false if FAILED, true if SUCCESS
@sa spi_writeByte, spi_writeWord
*/
uint8_t spi_writeBytes(SPI_1P1_CS_Pin * cs, uint8_t addr, uint8_t * buffer, uint8_t len);

#endif /* SPI_1P1_H_ */

/** @} */
