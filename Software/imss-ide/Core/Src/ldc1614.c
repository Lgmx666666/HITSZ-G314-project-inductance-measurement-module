#include "ldc1614.h"
#include "math.h"

#define FREQUENCY_REF 40 // 40MHz
#define CAP_VALUE 330 // 330pF
static uint16_t allData[8];
static uint8_t default_addr = LDC1614_DEFAULT_I2CADDR << 1;
extern UART_HandleTypeDef hlpuart1;
extern TIM_HandleTypeDef htim2;
uint8_t info = 0xFF;

void ldc1614_writeWord(uint8_t addr, uint16_t cmd){
  uint8_t transmittedData[2];
  transmittedData[0] = cmd >> 8;
  transmittedData[1] = cmd & 0xFF;

  HAL_I2C_Mem_Write(&hi2c1, default_addr, (uint16_t)addr, I2C_MEMADD_SIZE_8BIT, transmittedData, 2, 0xffff);
}

uint16_t ldc1614_readWord(uint8_t addr){
  uint8_t receivedData[2];

  if(HAL_OK == HAL_I2C_Mem_Read(&hi2c1, default_addr, (uint16_t)addr, I2C_MEMADD_SIZE_8BIT, receivedData, 2, 0xffff)){
    return ((receivedData[0]<<8) | receivedData[1]);
  }
  else Error_Handler();
}

uint8_t ldc1614_init(void){
  HAL_TIM_Base_Start_IT(&htim2);
  SD_LOW;
  // soft reset
  ldc1614_writeWord(LDC13xx16xx_CMD_RESET_DEVICE, 0x8000);
  // clock divider
  ldc1614_writeWord(LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH0, 0x1002);
  ldc1614_writeWord(LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH1, 0x1002);
  ldc1614_writeWord(LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH2, 0x1002);
  ldc1614_writeWord(LDC13xx16xx_CMD_CLOCK_DIVIDERS_CH3, 0x1002);
  // settle time
  ldc1614_writeWord(LDC13xx16xx_CMD_SETTLE_COUNT_CH0, 0x0014);  //before: 0x000D
  ldc1614_writeWord(LDC13xx16xx_CMD_SETTLE_COUNT_CH1, 0x0014);
  ldc1614_writeWord(LDC13xx16xx_CMD_SETTLE_COUNT_CH2, 0x0014);
  ldc1614_writeWord(LDC13xx16xx_CMD_SETTLE_COUNT_CH3, 0x0014);
  // conversion time
  ldc1614_writeWord(LDC13xx16xx_CMD_REF_COUNT_CH0, 0x09AD);
  ldc1614_writeWord(LDC13xx16xx_CMD_REF_COUNT_CH1, 0x09AD);
  ldc1614_writeWord(LDC13xx16xx_CMD_REF_COUNT_CH2, 0x09AD);
  ldc1614_writeWord(LDC13xx16xx_CMD_REF_COUNT_CH3, 0x09AD);
  //error_config
  ldc1614_writeWord(LDC13xx16xx_CMD_ERROR_CONFIG, 0x0000);
  // drive current config for all channels!!!
  ldc1614_writeWord(LDC13xx16xx_CMD_DRIVE_CURRENT_CH0, 0xE900);
  ldc1614_writeWord(LDC13xx16xx_CMD_DRIVE_CURRENT_CH1, 0xE900);
  ldc1614_writeWord(LDC13xx16xx_CMD_DRIVE_CURRENT_CH2, 0xE900);
  ldc1614_writeWord(LDC13xx16xx_CMD_DRIVE_CURRENT_CH3, 0xE900);
  // mux_config
  ldc1614_writeWord(LDC13xx16xx_CMD_MUX_CONFIG, 0xC20D); // for auto-conv bit 15->1, bit 14:13-> to select channels default:0xA20D
  // config
  ldc1614_writeWord(LDC13xx16xx_CMD_CONFIG, 0x1601);
}

void ldc1614_processDRDY(void){
  allData[0] = ldc1614_readWord(LDC13xx16xx_CMD_DATA_MSB_CH0);
  allData[1] = ldc1614_readWord(LDC13xx16xx_CMD_DATA_LSB_CH0);
  allData[2] = ldc1614_readWord(LDC13xx16xx_CMD_DATA_MSB_CH1);
  allData[3] = ldc1614_readWord(LDC13xx16xx_CMD_DATA_LSB_CH1);
  allData[4] = ldc1614_readWord(LDC13xx16xx_CMD_DATA_MSB_CH2);
  allData[5] = ldc1614_readWord(LDC13xx16xx_CMD_DATA_LSB_CH2);
  allData[6] = ldc1614_readWord(LDC13xx16xx_CMD_DATA_MSB_CH3);
  allData[7] = ldc1614_readWord(LDC13xx16xx_CMD_DATA_LSB_CH3);
}

float ldc1614_readFreqRatio(uint8_t ch){
  uint8_t data[4];
  float num;

  data[0] = allData[ch*2] >> 8;
  data[1] = allData[ch*2] & 0xFF;
  data[2] = allData[ch*2+1] >> 8;
  data[3] = allData[ch*2+1] & 0xFF;

  num = (((float)data[0])*powf(256, 3) + ((float)data[1])*powf(256, 2) + ((float)data[2])*powf(256, 1) + (float)data[3])/
        powf(2, 28);

  return num;
}

float ldc1614_readFreq(uint8_t ch){
  uint8_t data[4];
  float num;

  data[0] = allData[ch*2] >> 8;
  data[1] = allData[ch*2] & 0xFF;
  data[2] = allData[ch*2+1] >> 8;
  data[3] = allData[ch*2+1] & 0xFF;

  num = FREQUENCY_REF*(((float)data[0])*powf(256, 3) + ((float)data[1])*powf(256, 2) + ((float)data[2])*powf(256, 1) + (float)data[3])/
        powf(2, 28);

  return num;
}

float ldc1614_readInductance(uint8_t ch){
  float freq, inductance;

  freq = ldc1614_readFreq(ch);
  inductance = 1e6/(4*M_PI*M_PI*CAP_VALUE*freq*freq); // uH in Hertz

  return inductance;
}

