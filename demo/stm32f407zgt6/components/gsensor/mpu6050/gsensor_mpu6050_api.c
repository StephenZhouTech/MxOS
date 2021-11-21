/*******************************************************************************
* File Name      : gsensor_mpu6050_api.c
* Description    : The basic function of MPU6050.
* Author         : StephenZhou
* Date           : 2020.8.26
*******************************************************************************/
#include "gsensor_mpu6050_api.h"
#include "gsensor_mpu6050_reg.h"
#include "gsensor_mpu6050_driver.h"
#include "delay.h"

#include "log.h"

uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr)
{
    return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr<<3);
}

uint8_t MPU_Set_Accel_Fsr(uint8_t fsr)
{
    return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);
}

uint8_t MPU_Set_LPF(uint16_t lpf)
{
    uint8_t data=0;
    if(lpf>=188)data=1;
    else if(lpf>=98)data=2;
    else if(lpf>=42)data=3;
    else if(lpf>=20)data=4;
    else if(lpf>=10)data=5;
    else data=6;
    return MPU_Write_Byte(MPU_CFG_REG,data);
}

uint8_t MPU_Set_Rate(uint16_t rate)
{
    uint8_t data;
    if(rate>1000)rate=1000;
    if(rate<4)rate=4;
    data=1000/rate-1;
    data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);
    return MPU_Set_LPF(rate/2);
}

uint16_t MPU_Get_Temperature(void)
{
    uint8_t buf[2];
    uint16_t raw;
    float temp;
    MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf);
    raw=((uint16_t)buf[0]<<8)|buf[1];
    temp=36.53+((double)raw)/340;
    return temp*100;
}

uint8_t MPU_Get_Gyroscope(uint16_t *gx,uint16_t *gy,uint16_t *gz)
{
    uint8_t buf[6],res;
    res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
    if(res==0)
    {
        *gx=((uint16_t)buf[0]<<8)|buf[1];
        *gy=((uint16_t)buf[2]<<8)|buf[3];
        *gz=((uint16_t)buf[4]<<8)|buf[5];
    }
    return res;;
}

uint8_t MPU_Get_Accelerometer(uint16_t *ax,uint16_t *ay,uint16_t *az)
{
    uint8_t buf[6],res;
    res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
    if(res==0)
    {
        *ax=((uint16_t)buf[0]<<8)|buf[1];
        *ay=((uint16_t)buf[2]<<8)|buf[3];
        *az=((uint16_t)buf[4]<<8)|buf[5];
    }
    return res;
}

uint8_t MPU6050_Configure(void)
{
    uint8_t res;
    MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);
    delay_ms(500);
    MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);
    delay_ms(500);
    MPU_Set_Gyro_Fsr(3);
    MPU_Set_Accel_Fsr(0);
    MPU_Set_Rate(50);
    MPU_Write_Byte(MPU_INT_EN_REG,0X00);
    MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);
    MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);
    MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);
    res=MPU_Read_Byte(MPU_DEVICE_ID_REG);

    LOG_DEBUG("Device ID:0x%02X",res);

    if(res==MPU_ADDR)
    {
        MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);
        MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);
        MPU_Set_Rate(50);
    }
    else
    {
        MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);
        MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);
        MPU_Set_Rate(50);
        return 1;
    }
    return 0;
}

void GSensor_MPU6050_Init(void)
{
    LOG_DEBUG("GSensor_MPU6050_Init");
    MPU6050_DRV_Init();
    MPU6050_Configure();
}
