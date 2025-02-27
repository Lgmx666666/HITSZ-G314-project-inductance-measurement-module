/** @file
 * @brief Device Port mapping for SPI1.1.
 * @defgroup spi_1p1 spi_1p1
 * @{
 * @ingroup peripherals
 *
 * @brief <b>MSP430 / SPI1.1 Interface Library Port Mapping</b> @n@n
 * This file contains port mappings for various devices in the msp430
 * family.
 *
 * @remarks
 * Only USCI devices with a single interrupt vector for TX and RX are
 * supported if interrupts are enabled (x5xx and x6xx family of devices).
 *
 * If a port is selected but not available on the device detected, an
 * error will be raised using the #error pre-processor.
 *
 * @author
 * Charles Cheung (chuck@ti.com)
 */

#include <msp430.h>
#include "spi_1p1.h"

#ifndef SPI_1P1_PORT_H_
#define SPI_1P1_PORT_H_

/** @name - SPI1.1 Registers - */
//@{
#if defined(__MSP430_HAS_USCI_A0__) && defined(SPI_USCIA0)
#define SPI_BASEADDR_USCI __MSP430_BASEADDRESS_USCI_A0__
#define SPI_INT_VECTOR 	USCI_A0_VECTOR	    /**< Set the interrupt vector */
#elif defined(__MSP430_HAS_USCI_A1__) && defined(SPI_USCIA1)
#define SPI_BASEADDR_USCI __MSP430_BASEADDRESS_USCI_A1__
#ifdef SPI_ENABLE_INTERRUPTS
#define SPI_INT_VECTOR 	USCI_A1_VECTOR	    /**< Set the interrupt vector */
#endif
#elif defined(__MSP430_HAS_USCI_A2__) && defined(SPI_USCIA2)
#define SPI_BASEADDR_USCI __MSP430_BASEADDRESS_USCI_A2__
#ifdef SPI_ENABLE_INTERRUPTS
#define SPI_INT_VECTOR 	USCI_A2_VECTOR	    /**< Set the interrupt vector */
#endif
#elif defined(__MSP430_HAS_USCI_A3__) && defined(SPI_USCIA3)
#define SPI_BASEADDR_USCI __MSP430_BASEADDRESS_USCI_A3__
#ifdef SPI_ENABLE_INTERRUPTS
#define SPI_INT_VECTOR 	USCI_A3_VECTOR	    /**< Set the interrupt vector */
#endif
#elif defined(__MSP430_HAS_USCI_B0__) && defined(SPI_USCIB0)
#define SPI_BASEADDR_USCI __MSP430_BASEADDRESS_USCI_B0__
#ifdef SPI_ENABLE_INTERRUPTS
#define SPI_INT_VECTOR 	USCI_B0_VECTOR	    /**< Set the interrupt vector */
#endif
#elif defined(__MSP430_HAS_USCI_B1__) && defined(SPI_USCIB1)
#define SPI_BASEADDR_USCI __MSP430_BASEADDRESS_USCI_B1__
#ifdef SPI_ENABLE_INTERRUPTS
#define SPI_INT_VECTOR 	USCI_B1_VECTOR	    /**< Set the interrupt vector */
#endif
#elif defined(__MSP430_HAS_USCI_B2__) && defined(SPI_USCIB2)
#define SPI_BASEADDR_USCI __MSP430_BASEADDRESS_USCI_B2__
#ifdef SPI_ENABLE_INTERRUPTS
#define SPI_INT_VECTOR 	USCI_B2_VECTOR	    /**< Set the interrupt vector */
#endif
#elif defined(__MSP430_HAS_USCI_B3__) && defined(SPI_USCIB3)
#define SPI_BASEADDR_USCI __MSP430_BASEADDRESS_USCI_B3__
#ifdef SPI_ENABLE_INTERRUPTS
#define SPI_INT_VECTOR 	USCI_B3_VECTOR	    /**< Set the interrupt vector */
#endif
#elif defined(__MSP430_HAS_USCI__) && defined(SPI_USCIA0)
#ifdef SPI_ENABLE_INTERRUPTS
#error "USCI Interrupt Mode not supported.  Please comment out SPI_ENABLE_INTERRUPTS."
#endif
#define SPI_USCI
#define SPI_CTL0   		UCA0CTL0			/**< Control Register 0 */
#define SPI_CTL1   		UCA0CTL1			/**< Control Register 1 */
#define SPI_BR0    		UCA0BR0				/**< Baud Rate Divisor LSB */
#define SPI_BR1	   		UCA0BR1				/**< Baud Rate Divisor MSB */
#define SPI_MCTL   		UCA0MCTL			/**< Modulation Control */
#define SPI_STAT		UCA0STAT			/**< Status register */
#define SPI_TXBUF      	UCA0TXBUF			/**< Transmit buffer */
#define SPI_RXBUF      	UCA0RXBUF			/**< Receive buffer */
#define SPI_IE			UCA0IE	    		/**< Interrupt enable */
#define SPI_IFG			UC0IFG				/**< Interrupt flag */
#define SPI_TXIFG		UCA0TXIFG			/**< Transmit Interrupt flag */
#define SPI_RXIFG		UCA0RXIFG			/**< Receive Interrupt flag */
#define SPI_BUSY		(SPI_STAT & UCBUSY)
#elif defined(__MSP430_HAS_USCI__) && defined(SPI_USCIB0)
#ifdef SPI_ENABLE_INTERRUPTS
#error "USCI Interrupt Mode not supported.  Please comment out SPI_ENABLE_INTERRUPTS."
#endif
#define SPI_USCI
#define SPI_CTL0   		UCB0CTL0			/**< Control Register 0 */
#define SPI_CTL1   		UCB0CTL1			/**< Control Register 1 */
#define SPI_BR0    		UCB0BR0				/**< Baud Rate Divisor LSB */
#define SPI_BR1	   		UCB0BR1				/**< Baud Rate Divisor MSB */
#define SPI_STAT		UCB0STAT			/**< Status register */
#define SPI_TXBUF      	UCB0TXBUF			/**< Transmit buffer */
#define SPI_RXBUF      	UCB0RXBUF			/**< Receive buffer */
#define SPI_IE			UCB0IE	    		/**< Interrupt enable */
#define SPI_IFG			UC0IFG				/**< Interrupt flag */
#define SPI_TXIFG		UCB0TXIFG			/**< Transmit Interrupt flag */
#define SPI_RXIFG		UCB0RXIFG			/**< Receive Interrupt flag */
#define SPI_BUSY		(SPI_STAT & UCBUSY)
#elif defined(__MSP430_HAS_USCI__) && defined(__MSP430_HAS_USCI_AB1__) && defined(SPI_USCIA1)
#ifdef SPI_ENABLE_INTERRUPTS
#error "USCI Interrupt Mode not supported.  Please comment out SPI_ENABLE_INTERRUPTS."
#endif
#define SPI_USCI
#define SPI_CTL0   		UCA1CTL0			/**< Control Register 0 */
#define SPI_CTL1   		UCA1CTL1			/**< Control Register 1 */
#define SPI_BR0    		UCA1BR0				/**< Baud Rate Divisor LSB */
#define SPI_BR1	   		UCA1BR1				/**< Baud Rate Divisor MSB */
#define SPI_MCTL   		UCA1MCTL			/**< Modulation Control */
#define SPI_STAT		UCA1STAT			/**< Status register */
#define SPI_TXBUF      	UCA1TXBUF			/**< Transmit buffer */
#define SPI_RXBUF      	UCA1RXBUF			/**< Receive buffer */
#define SPI_IE			UCA1IE	    		/**< Interrupt enable */
#define SPI_IFG			UC1IFG				/**< Interrupt flag */
#define SPI_TXIFG		UCA1TXIFG			/**< Transmit Interrupt flag */
#define SPI_RXIFG		UCA1RXIFG			/**< Receive Interrupt flag */
#define SPI_BUSY		(SPI_STAT & UCBUSY)
#elif defined(__MSP430_HAS_USCI__) && defined(__MSP430_HAS_USCI_AB1__) && defined(SPI_USCIB1)
#ifdef SPI_ENABLE_INTERRUPTS
#error "USCI Interrupt Mode not supported.  Please comment out SPI_ENABLE_INTERRUPTS."
#endif
#define SPI_USCI
#define SPI_CTL0   		UCB1CTL0			/**< Control Register 0 */
#define SPI_CTL1   		UCB1CTL1			/**< Control Register 1 */
#define SPI_BR0    		UCB1BR0				/**< Baud Rate Divisor LSB */
#define SPI_BR1	   		UCB1BR1				/**< Baud Rate Divisor MSB */
#define SPI_STAT		UCB1STAT			/**< Status register */
#define SPI_TXBUF      	UCB1TXBUF			/**< Transmit buffer */
#define SPI_RXBUF      	UCB1RXBUF			/**< Receive buffer */
#define SPI_IE			UCB1IE	    		/**< Interrupt enable */
#define SPI_IFG			UC1IFG				/**< Interrupt flag */
#define SPI_TXIFG		UCB1TXIFG			/**< Transmit Interrupt flag */
#define SPI_RXIFG		UCB1RXIFG			/**< Receive Interrupt flag */
#define SPI_BUSY		(SPI_STAT & UCBUSY)
#elif defined(__MSP430_HAS_UART0__) && defined(SPI_USART0)
#ifdef SPI_ENABLE_INTERRUPTS
#error "USCI Interrupt Mode not supported.  Please comment out SPI_ENABLE_INTERRUPTS."
#endif
#define SPI_USART
#define SPI_INT_RXVECTOR USART0RX_VECTOR
#define SPI_INT_TXVECTOR USART0TX_VECTOR
#define SPI_CTL0   		U0CTL			/**< Control Register 0 */
#define SPI_CTLT	   	U0TCTL			/**< Control Register Timing */
#define SPI_CTLR		U0RCTL			/**< Control Register */
#define SPI_BR0    		U0BR0			/**< Baud Rate Divisor LSB */
#define SPI_BR1	   		U0BR1			/**< Baud Rate Divisor MSB */
#define SPI_MCTL   		U0MCTL			/**< Modulation Control */
#define SPI_TXBUF      	U0TXBUF			/**< Transmit buffer */
#define SPI_RXBUF      	U0RXBUF			/**< Receive buffer */
#define SPI_ME          U0ME            /**< Module enable */
#define SPI_PIE         USPIE0			/**< Module enable flag */
#define SPI_IE			U0IE 			/**< Interrupt enable */
#define SPI_RXIE		URXIE0			/**< Receive Interrupt enable */
#define SPI_TXIE		UTXIE0			/**< Transmit Interrupt enable */
#define SPI_IFG			U0IFG			/**< Interrupt flag */
#define SPI_TXIFG		UTXIFG0			/**< Transmit Interrupt flag */
#define SPI_RXIFG		URXIFG0			/**< Receive Interrupt flag */
#define SPI_BUSY		!(SPI_CTLT & TXEPT)
#elif defined(__MSP430_HAS_UART1__) && defined(SPI_USART1)
#ifdef SPI_ENABLE_INTERRUPTS
#error "USCI Interrupt Mode not supported.  Please comment out SPI_ENABLE_INTERRUPTS."
#endif
#define SPI_USART
#define SPI_INT_RXVECTOR USART1RX_VECTOR
#define SPI_INT_TXVECTOR USART1TX_VECTOR
#define SPI_CTL0   		U1CTL			/**< Control Register 0 */
#define SPI_CTLT    	U1TCTL			/**< Control Register Timing */
#define SPI_CTLR		U1RCTL			/**< Control Register */
#define SPI_BR0    		U1BR0			/**< Baud Rate Divisor LSB */
#define SPI_BR1	   		U1BR1			/**< Baud Rate Divisor MSB */
#define SPI_MCTL   		U1MCTL			/**< Modulation Control */
#define SPI_TXBUF      	U1TXBUF			/**< Transmit buffer */
#define SPI_RXBUF      	U1RXBUF			/**< Receive buffer */
#define SPI_ME          U1ME            /**< Module enable */
#define SPI_PIE         USPIE1			/**< Module enable flag */
#define SPI_IE			U1IE 			/**< Interrupt enable */
#define SPI_RXIE		URXIE1			/**< Receive Interrupt enable */
#define SPI_TXIE		UTXIE1			/**< Transmit Interrupt enable */
#define SPI_IFG			U1IFG			/**< Interrupt flag */
#define SPI_TXIFG		UTXIFG1			/**< Transmit Interrupt flag */
#define SPI_RXIFG		URXIFG1			/**< Receive Interrupt flag */
#define SPI_BUSY		!(SPI_CTLT & TXEPT)
#elif defined(__MSP430_HAS_USI__) && defined(SPI_USI)
#ifdef SPI_ENABLE_INTERRUPTS
#error "USCI Interrupt Mode not supported.  Please comment out SPI_ENABLE_INTERRUPTS."
#endif
#define SPI_USI
#define SPI_CTL0   		USICTL0			/**< USI  Control Register 0 */
#define SPI_CTL1   		USICTL1			/**< USI  Control Register 1 */
#define SPI_CKCTL  		USICKCTL	 	/**< USI  Clock Control Register */
#define SPI_TXBUF  		USISRL	 		/**< USI  Low Byte Shift Register */
#define SPI_RXBUF  		USISRL	 		/**< USI  Low Byte Shift Register */
#define SPI_IFG			USICTL1			/**< Interrupt flag */
#define SPI_TXIFG		USIIFG			/**< Transmit Interrupt flag */
#define SPI_RXIFG		USIIFG			/**< Receive Interrupt flag */
#define SPI_BUSY		!(USICTL1 & USIIFG)
#else
#error "Support for selected SPI port not found"
#endif
// for x5xx and x6xx devices
#ifdef SPI_BASEADDR_USCI
typedef unsigned int 	Spi_AddressReg_t; /* size of MSP430 register address */
#define SPI_SFR_REG8(x)		(*((volatile unsigned char*)((Spi_AddressReg_t)(x))))
#define SPI_SFR_REG16(x)	(*((volatile unsigned int*)((Spi_AddressReg_t)(x))))
#define SPI_USCI
#define SPI_CTL0   		SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x01)		/**< Control Register 0 */
#define SPI_CTL1   		SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x00)		/**< Control Register 1 */
#define SPI_BR0    		SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x06)		/**< Baud Rate Divisor LSB */
#define SPI_BR1	   		SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x07)		/**< Baud Rate Divisor MSB */
#define SPI_MCTL   		SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x08)		/**< Modulation Control */
#define SPI_STAT		SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x0A)		/**< Status register */
#define SPI_RXBUF      	SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x0C)		/**< Receive buffer */
#define SPI_TXBUF      	SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x0E)		/**< Transmit buffer */
#define SPI_IE			SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x1C)	    /**< Interrupt enable */
#define SPI_IFG        	SPI_SFR_REG8(SPI_BASEADDR_USCI + 0x1D)		/**< Interrupt flag */
#define SPI_IV	       	SPI_SFR_REG16(SPI_BASEADDR_USCI + 0x1E)		/**< Interrupt vector */
#define SPI_TXIFG		UCTXIFG										/**< Transmit Interrupt flag */
#define SPI_RXIFG		UCRXIFG										/**< Receive Interrupt flag */
#define SPI_BUSY		(SPI_STAT & UCBUSY)
#endif
//@}

/** @name - SPI1.1 Interrupt Flags - */
//@{
#define SPI_TXBUF_EMPTY (SPI_IFG & SPI_TXIFG) && !(SPI_BUSY)
#define SPI_RXBUF_FULL  (SPI_IFG & SPI_RXIFG) && !(SPI_BUSY)
//@}

#endif /* SPI_1P1_PORT_H_ */
