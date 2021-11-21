#ifndef __MPU6050_I2C_H__
#define __MPU6050_I2C_H__

#include "sys.h" 
#include "stm32f4xx_conf.h"

#define MPU6050_I2C_IO_RCC          RCC_AHB1Periph_GPIOB
#define MPU6050_I2C_IO_BANK         GPIOB
#define MPU6050_I2C_SCL_PIN         GPIO_Pin_8
#define MPU6050_I2C_SDA_PIN         GPIO_Pin_9

#define SDA_IN()  {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=0<<9*2;}
#define SDA_OUT() {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=1<<9*2;}

#define IIC_SCL    PBout(8)
#define IIC_SDA    PBout(9)
#define READ_SDA   PBin(9)

#define IIC_WAIT_ACK_TO             250

void IIC_Init(void);
void IIC_GenerateStart(void);
void IIC_GenerateStop(void);
uint8_t IIC_WaitAck(void);
void IIC_SendByte(uint8_t txd);
uint8_t IIC_ReadByte(unsigned char ack);

#endif // __MPU6050_I2C_H__
