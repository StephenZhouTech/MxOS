#ifndef __GSENSOR_MPU6050_DRIVER_H__
#define __GSENSOR_MPU6050_DRIVER_H__

#include <stdint.h>

#define AD0_CONNECT_WITH_GND        0x68 // AD0 -> GND
#define AD0_CONNECT_WITH_VCC        0x69 // AD0 -> VCC

#define MPU_ADDR     AD0_CONNECT_WITH_GND

void MPU6050_DRV_Init(void);
uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);
uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);
uint8_t MPU_Write_Byte(uint8_t reg,uint8_t data);
uint8_t MPU_Read_Byte(uint8_t reg);

#endif // __GSENSOR_MPU6050_DRIVER_H__
