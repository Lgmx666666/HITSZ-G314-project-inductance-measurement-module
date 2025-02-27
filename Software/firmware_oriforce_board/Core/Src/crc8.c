#include "crc8.h"

uint8_t crc8_MAXIM(uint8_t *data, uint8_t len)
{
  uint8_t crc, i;
  crc = 0x00;

  while(len--)
  {
    crc ^= *data++;
    for(i = 0;i < 8;i++)
    {
      if(crc & 0x01)
      {
        crc = (crc >> 1) ^ 0x8c;
      }
      else crc >>= 1;
    }
  }
  return crc;
}
