#ifndef __GSENSOR_MPU6050_API_H__
#define __GSENSOR_MPU6050_API_H__

#include <stdint.h>

void GSensor_MPU6050_Init(void);
uint16_t MPU_Get_Temperature(void);
uint8_t MPU_Get_Gyroscope(uint16_t *gx,uint16_t *gy,uint16_t *gz);
uint8_t MPU_Get_Accelerometer(uint16_t *ax,uint16_t *ay,uint16_t *az);
#endif // __GSENSOR_MPU6050_API_H__
