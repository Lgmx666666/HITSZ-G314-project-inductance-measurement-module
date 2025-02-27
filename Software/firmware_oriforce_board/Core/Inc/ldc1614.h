//
// Created by LiangGuang on 2023/6/2.
//

#ifndef ISENSOR_LDC1614_H
#define ISENSOR_LDC1614_H

#include "main.h"

// LDC1614 COMMAND
#define LDC13xx16xx_CMD_DATA_MSB_CH0	0x00
#define LDC13xx16xx_CMD_DATA_LSB_CH0	0x01
#define LDC13xx16xx_CMD_DATA_MSB_CH1	0x02
#define LDC13xx16xx_CMD_DATA_LSB_CH1	0x03
#define LDC13xx16xx_CMD_DATA_MSB_CH2	0x04
#define LDC13xx16xx_CMD_DATA_LSB_CH2	0x05
#define LDC13xx16xx_CMD_DATA_MSB_CH3	0x06
#define LDC13xx16xx_CMD_DATA_LSB_CH3	0x07
#define LDC13xx16xx_CMD_REF_COUNT_CH0	0x08
#define LDC13xx16xx_CMD_REF_COUNT_CH1	0x09
#define LDC13xx16xx_CMD_REF_COUNT_CH2	0x0A
#define LDC13xx16xx_CMD_REF_COUNT_CH3	0x0B
#define LDC13xx16xx_CMD_OFFSET_CH0	0x0C
#define LDC13xx16xx_CMD_OFFSET_CH1	0x0D
#define LDC13xx16xx_CMD_OFFSET_CH2	0x0E
#define LDC13xx16xx_CMD_OFFSET_CH3	0x0F
#define LDC13xx16xx_CMD_SETTLE_COUNT_CH0	0x10
#define LDC13xx16xx_CMD_SETTLE_COUNT_CH1	0x11
#define LDC13xx16xx_CMD_SETTLE_COUNT_CH2	0x12
#define LDC13xx16xx_CMD_SETTLE_COUNT_CH3	0x13
#define LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH0 	0x14
#define LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH1 	0x15
#define LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH2 	0x16
#define LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH3 	0x17
#define LDC13xx16xx_CMD_STATUS 	0x18
#define LDC13xx16xx_CMD_ERROR_CONFIG 	0x19
#define LDC13xx16xx_CMD_CONFIG 	0x1A
#define LDC13xx16xx_CMD_MUX_CONFIG 	0x1B
#define LDC13xx16xx_CMD_RESET_DEVICE 	0x1C
#define LDC13xx16xx_CMD_SYSTEM_CLOCK_CONFIG	0x1D
#define LDC13xx16xx_CMD_DRIVE_CURRENT_CH0	0x1E
#define LDC13xx16xx_CMD_DRIVE_CURRENT_CH1 	0x1F
#define LDC13xx16xx_CMD_DRIVE_CURRENT_CH2 	0x20
#define LDC13xx16xx_CMD_DRIVE_CURRENT_CH3	0x21
#define LDC13xx16xx_CMD_MANUFACTID	0x7E
#define LDC13xx16xx_CMD_DEVID	0x7F

#define SD_LOW HAL_GPIO_WritePin(LDC_SD_GPIO_Port, LDC_SD_Pin, GPIO_PIN_RESET);
#define SD_HIGH HAL_GPIO_WritePin(LDC_SD_GPIO_Port, LDC_SD_Pin, GPIO_PIN_SET);

extern I2C_HandleTypeDef hi2c1;

#define LDC1614_DEFAULT_I2CADDR     0x2A
#define LDC1614_DEFAULT_SIZE 24     // 13 registers, 0x08 - 0x14
/** Write Word
 *  @param cmd value of register
 *  @param addr register address
 * **/
void ldc1614_writeWord(uint8_t addr, uint16_t cmd);

/** Read Word
 *  @param addr register address
 *  @return value of register
 * **/
uint16_t ldc1614_readWord(uint8_t addr);

/** Initialize ldc1614
 *  @return 1 if initialized, 0 if error occurred
 * **/
uint8_t ldc1614_init(void);

/** Data Ready interrupt Handler
 * @remark put into a callback function of INTB assert
 * **/
void ldc1614_processDRDY(void);

/** Read Ratio of Frequency_sensor/Frequency_ref
 *  Read the last updated data ready values
 *  @param ch channel
 *  @return float ratio value
 *  @remarks Data is raw and includes error codes
 * **/
float ldc1614_readFreqRatio(uint8_t ch);

/** Read Frequency of sensor
 *  Read the last updated data ready values
 *  @param ch channel
 *  @return float ratio value
 *  @remarks Data is raw and includes error codes
 * **/
float ldc1614_readFreq(uint8_t ch);

/** Read Inductance of sensor
 *  Read the last updated data ready values
 *  @param ch channel
 *  @return float ratio value
 *  @remarks Data is raw and includes error codes
 * **/
float ldc1614_readInductance(uint8_t ch);
#endif //ISENSOR_LDC1614_H
