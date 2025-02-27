
#ifndef PLL_H_
#define PLL_H_

#define PLL_ADDR 0x65

/** @name - S0 Pin Definition - */
//@{
#define PLL_S0_OUT            P6OUT	/**< Define S0 pin output register */
#define PLL_S0_DIR            P6DIR	/**< Define S0 pin direction register */
#define PLL_S0_BIT            BIT6	/**< Define S0 pin bit register */
//@}

/** @name - S0 Function Macros - */
//@{
#define PLL_ON()	   (PLL_S0_DIR &= ~PLL_S0_BIT)
#define PLL_OFF()      do { \
						(PLL_S0_OUT &= ~PLL_S0_BIT);\
						(PLL_S0_DIR |= PLL_S0_BIT);\
					   } while(0)
//@}

void pll_init();

#endif
