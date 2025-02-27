/** @file
 * @brief Implementation of functions for LDC13xx16xx_evm
 */

#include "LDC13xx16xx_cmd.h"
#include "LDC13xx16xx_evm.h"
#include "host_interface.h"
//#include "spi_1p1.h"
#include "i2c.h"
#include "crc8.h"

#include "pll.h"

#include "HAL_FLASH.h"

/** Defaults */
#define INFOD_START (0x1800)

#pragma DATA_SECTION(FlashConfig, ".infoD")
uint16_t  FlashConfig[EVM_DEFAULTS_SIZE*sizeof(uint16_t)];

/** DRDY */
static volatile uint8_t dataReady;
static uint16_t allData[8];

static uint8_t default_addr;

static void evm_Delay_Ms(uint16_t ms) {
	uint16_t i;
	for (i = 0; i < ms; i ++) {
		__delay_cycles(EVM_TIME_1MS);
	}
}

/** LEDS */
static uint8_t led_status = 0;
uint8_t evm_LED_Set(evm_led_state_t state) {
	switch (state) {
	case ALLTOGGLE:
		led_status ^= 0x03;
		break;
	case CYCLE:
		led_status += 1;
		led_status &= 0x03;
		break;
	case GREEN:
		led_status = 0x01;
		break;
	case RED:
		led_status = 0x02;
		break;
	case ALLON:
		led_status = 0x03;
		break;
	case ALLOFF:
		led_status = 0x00;
		break;
	default:
		break;
	}
	if (led_status & 0x01) {
		EVM_GRN_LED_ON();
	}
	else {
		EVM_GRN_LED_OFF();
	}
	if (led_status & 0x02) {
		EVM_RED_LED_ON();
	}
	else {
		EVM_RED_LED_OFF();
	}
	return led_status;
}

uint8_t evm_LED_State() {
	return led_status;
}

uint8_t evm_readDefaults(uint8_t offset,uint8_t * buffer,uint8_t size) {
	memcpy(&buffer[0],(uint8_t*)&FlashConfig[offset],size*sizeof(uint16_t));
	return size;
}

/** Default Values */
void evm_saveDefaults(uint8_t * buffer) {
//	uint8_t x;
	// write flash
	//Erase INFOD
//	do {
		//Erase INFOD
		Flash_SegmentErase((uint16_t*)INFOD_START);
//		x = Flash_EraseCheck((uint16_t*)INFOD_START,128/2);
//	} while (x == FLASH_STATUS_ERROR);

	//Write calibration data to INFOD
	FlashWrite_8(
				 (uint8_t*)buffer,
				 (uint8_t*)FlashConfig,
				 EVM_DEFAULTS_SIZE*sizeof(uint16_t)
				 );
}

/** Initialization */
uint8_t evm_init() {

	uint8_t retVal = TRUE;
	uint8_t i;

	dataReady = 0;

	init_Clock_prePLL();

	// LEDs
	EVM_GRN_LEDDIR |= EVM_GRN_LEDBIT;
	EVM_RED_LEDDIR |= EVM_RED_LEDBIT;

	_disable_interrupts();

	// initialize I2C
	i2c_setup();

	_enable_interrupts();

	// ADDR low (0x2A)
	EVM_ADDR_OUT &= ~EVM_ADDR_BIT;
	EVM_ADDR_DIR |= EVM_ADDR_BIT;
	default_addr = EVM_DEFAULT_I2CADDR;
	// SHUTDOWN low
	EVM_SHUTDOWN_OUT &= ~EVM_SHUTDOWN_BIT;
	EVM_SHUTDOWN_DIR |= EVM_SHUTDOWN_BIT;

	// init pll
	pll_init();

	InitMCU();
	// software reset
	smbus_writeWord(default_addr,LDC13xx16xx_CMD_RESET_DEVICE,0x8000);

	// delay ~10ms
	evm_Delay_Ms(10);

	// TODO: put addrs in a const array; iterate over array
	// if default data has invalid crc use below and recalibrate
	if (calcCRC8((uint8_t *)&FlashConfig[0], EVM_DEFAULTS_SIZE*sizeof(uint16_t))) {
		evm_LED_Set(ALLON);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_REF_COUNT_CH0,0xFFFF); // 4 clock periods
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_REF_COUNT_CH1,0xFFFF);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_REF_COUNT_CH2,0xFFFF);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_REF_COUNT_CH3,0xFFFF);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_OFFSET_CH0,0x0000);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_OFFSET_CH1,0x0000);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_OFFSET_CH2,0x0000);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_OFFSET_CH3,0x0000);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_SETTLE_COUNT_CH0,0x0400); // 1 clock period
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_SETTLE_COUNT_CH1,0x0400);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_SETTLE_COUNT_CH2,0x0400);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_SETTLE_COUNT_CH3,0x0400);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH0,0x0000); // bypass dividers
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH1,0x0000);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH2,0x0000);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH3,0x0000);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_STATUS,0x0001); // report only DRDYs to INT
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_CONFIG,0x0200); // CLKIN pin
		
		#if defined(HOST_DEVICE_LDC1612) || defined (HOST_DEVICE_LDC1312)
			   retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_MUX_CONFIG,0x820F);//MS, changed to seq ch0 n ch1 // ch0, ch1
		#elif defined(HOST_DEVICE_LDC1614) || defined (HOST_DEVICE_LDC1314)
			   retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_MUX_CONFIG,0xC20F); // ch0, ch1,ch2,ch3-> Wipro for 4 ch
		#endif

		//retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_MUX_CONFIG,0x820F); // ch0, ch1
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_SYSTEM_CLOCK_CONFIG,0x0200); // default, divide by 2
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_DRIVE_CURRENT_CH0,0x0000); //
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_DRIVE_CURRENT_CH1,0x0000); //
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_DRIVE_CURRENT_CH2,0x0000); //
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_DRIVE_CURRENT_CH3,0x0000); //
//		evm_tuneRp();
	}
	else { // valid crc write defaults
		for (i = 0x08; i < 0x19; i++) {
			retVal &= smbus_writeWord(default_addr,i,FlashConfig[i-8]);
		}
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_CONFIG,FlashConfig[i-8]);
		retVal &= smbus_writeWord(default_addr,LDC13xx16xx_CMD_MUX_CONFIG,FlashConfig[i-7]);
		for (i = 0x1D; i < 0x22; i++) {
			retVal &= smbus_writeWord(default_addr,i,FlashConfig[i-10]);
		}
	}

	// setup DRDY pin interrupt
	EVM_INT_DIR &= ~EVM_INT_BIT;							// INPUT
	EVM_INT_IE  |=  EVM_INT_BIT;							// interrupt enabled
	EVM_INT_IES |=  EVM_INT_BIT;							// Hi->Lo Edge
	EVM_INT_IFG &= ~EVM_INT_BIT;							// Clear IFG

	return retVal;
}

// TODO: use DMA to transfer data
// interrupt service routine, DRDY
#pragma vector=EVM_INT_VECTOR
__interrupt void DRDY()
{
  if (!(EVM_INT_IN & EVM_INT_BIT)) {
	  dataReady = 1;
  }
  EVM_INT_IFG &= ~EVM_INT_BIT;       // IFG cleared
}

void evm_processDRDY() {
	if (dataReady) {
	  smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_MSB_CH0,&allData[0]);
	  smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_LSB_CH0,&allData[1]);
	  smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_MSB_CH1,&allData[2]);
	  smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_LSB_CH1,&allData[3]);
	  smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_MSB_CH2,&allData[4]);
	  smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_LSB_CH2,&allData[5]);
	  smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_MSB_CH3,&allData[6]);
	  smbus_readWord(default_addr,LDC13xx16xx_CMD_DATA_LSB_CH3,&allData[7]);
	  dataReady = 0;
	}
}

// TODO: use DMA to transfer data
// ch 0-3
uint8_t evm_readFreq(uint8_t ch, uint8_t * buffer) {
	buffer[0] = allData[ch*2] >> 8;
	buffer[1] = allData[ch*2] & 0xFF;
	buffer[2] = allData[ch*2+1] >> 8;
	buffer[3] = allData[ch*2+1] & 0xFF;
	return 4;
}

uint8_t evm_changeAddr(uint8_t addr) {
	if (default_addr == addr) return default_addr;
	if (addr == EVM_MAX_I2CADDR) {
		EVM_ADDR_OUT |= EVM_ADDR_BIT;
		default_addr = EVM_MAX_I2CADDR;
	}
	else if (addr == EVM_MIN_I2CADDR) {
		EVM_ADDR_OUT &= ~EVM_ADDR_BIT;
		default_addr = EVM_MIN_I2CADDR;
	}
	return default_addr;
}

//void evm_setLDCLK(evm_ldclk_state_t state) {
//	uint16_t reg;
//	switch (state) {
//	case OFF:
//		EVM_LDCLK_DIR &= ~EVM_LDCLK_BIT;   // Set to input (hiz)
//		EVM_LDCLK_SEL &= ~EVM_LDCLK_BIT;   // disable function
//		clockState = state;
//		break;
//	case DIV4: case DIV8: case DIV16: case DIV32:
//		EVM_LDCLK_DIR |= EVM_LDCLK_BIT;   // LDC CLK for Freq counter (set to output ACLK)
//		EVM_LDCLK_SEL |= EVM_LDCLK_BIT;   // LDC CLK for freq counter (set to output ACLK)
//		reg = EVM_LDCLK_REG;
//		reg &= ~EVM_LDCLK_MASK;
//		reg |= state;
//		EVM_LDCLK_REG = reg;
//		clockState = state;
//		break;
//	default:
//		break;
//	}
//}

//evm_ldclk_state_t evm_readLDCLK() {
//	return clockState;
//}

void init_Clock_prePLL() {

	UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
	  UCSCTL4 |= SELA_3;                        // Set ACLK = REFO

	  __bis_SR_register(SCG0);                  // Disable the FLL control loop
	  UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
	  UCSCTL1 = DCORSEL_5;                      // Select DCO range 24MHz operation
	  UCSCTL2 = FLLD_1 + 374;                   // Set DCO Multiplier for 12MHz
	                                            // (N + 1) * FLLRef = Fdco
	                                            // ( 374+1) * 32k = 12MHz
	                                            // Set FLL Div = fDCOCLK/2
	  __bic_SR_register(SCG0);                  // Enable the FLL control loop

	  // Worst-case settling time for the DCO when the DCO range bits have been
	  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	  // UG for optimization.
	  // 32 x 32 x 12 MHz / 32,768 Hz = 375000 = MCLK cycles for DCO to settle
	  __delay_cycles(375000);

	  // Loop until XT1,XT2 & DCO fault flag is cleared
	  do
	  {
	    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
	                                            // Clear XT2,XT1,DCO fault flags
	    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	  } while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
}

