/*******************************************************************************
* File Name      : gsensor_mpu6050_driver.c
* Description    : Basic driver of MPU6050.
* Author         : StephenZhou
* Date           : 2020.9.1
*******************************************************************************/
#include "gsensor_mpu6050_driver.h"
#include "mpu6050_i2c.h"

void MPU6050_DRV_Init(void)
{
    IIC_Init();
}

uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
    uint8_t i;
    IIC_GenerateStart();
    IIC_SendByte((addr<<1)|0);
    if(IIC_WaitAck())
    {
        IIC_GenerateStop();
        return 1;
    }
    IIC_SendByte(reg);
    IIC_WaitAck();
    for(i=0;i<len;i++)
    {
        IIC_SendByte(buf[i]);
        if(IIC_WaitAck())
        {
            IIC_GenerateStop();
            return 1;
        }
    }
    IIC_GenerateStop();
    return 0;
}

uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
    IIC_GenerateStart();
    IIC_SendByte((addr<<1)|0);
    if(IIC_WaitAck())
    {
        IIC_GenerateStop();
        return 1;
    }
    IIC_SendByte(reg);
    IIC_WaitAck();
    IIC_GenerateStart();
    IIC_SendByte((addr<<1)|1);
    IIC_WaitAck();
    while(len)
    {
        if(len==1)*buf=IIC_ReadByte(0);
        else *buf=IIC_ReadByte(1);
        len--;
        buf++;
    }
    IIC_GenerateStop();
    return 0;
}

uint8_t MPU_Write_Byte(uint8_t reg,uint8_t data)
{
    IIC_GenerateStart();
    IIC_SendByte((MPU_ADDR<<1)|0);
    if(IIC_WaitAck())
    {
        IIC_GenerateStop();
        return 1;
    }
    IIC_SendByte(reg);
    IIC_WaitAck();
    IIC_SendByte(data);
    if(IIC_WaitAck())
    {
        IIC_GenerateStop();
        return 1;
    }
    IIC_GenerateStop();
    return 0;
}

uint8_t MPU_Read_Byte(uint8_t reg)
{
    uint8_t res;
    IIC_GenerateStart();
    IIC_SendByte((MPU_ADDR<<1)|0);
    IIC_WaitAck();
    IIC_SendByte(reg);
    IIC_WaitAck();
    IIC_GenerateStart();
    IIC_SendByte((MPU_ADDR<<1)|1);
    IIC_WaitAck();
    res=IIC_ReadByte(0);
    IIC_GenerateStop();
    return res;
}

