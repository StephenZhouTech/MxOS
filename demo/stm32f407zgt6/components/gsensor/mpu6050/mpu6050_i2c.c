/*******************************************************************************
* File Name      : mpu6050_i2c.c
* Description    : Basic I2C driver of MPU6050.
* Author         : StephenZhou
* Date           : 2020.9.13
*******************************************************************************/
#include <stdint.h>
#include "mpu6050_i2c.h"
#include "delay.h"

/*******************************************************************************
***** GLOBAL VARIABLE
*******************************************************************************/

/*******************************************************************************
* Function Name  : IIC_Init
* Description    : Init I2C.
* Input          : N/A
* Output         : N/A
* Return         : N/A
*******************************************************************************/
void IIC_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* GPIO B Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(MPU6050_I2C_IO_RCC, ENABLE);

    // Configure PB10: I2C2_SCL  PB11: I2C2_SDA
    GPIO_InitStructure.GPIO_Pin   = MPU6050_I2C_SCL_PIN | MPU6050_I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(MPU6050_I2C_IO_BANK, &GPIO_InitStructure);

    IIC_SCL = 1;
    IIC_SDA = 1;
}

/*******************************************************************************
* Function Name  : IIC_GenerateStart
* Description    : I2C Generate Start Signal.
* Input          : N/A
* Output         : N/A
* Return         : N/A
*******************************************************************************/
void IIC_GenerateStart(void)
{
    SDA_OUT();
    IIC_SDA=1;
    IIC_SCL=1;
    delay_us(4);
    IIC_SDA=0;
    delay_us(4);
    IIC_SCL=0;
}

/*******************************************************************************
* Function Name  : IIC_GenerateStop
* Description    : I2C Generate Stop Signal.
* Input          : N/A
* Output         : N/A
* Return         : N/A
*******************************************************************************/
void IIC_GenerateStop(void)
{
    SDA_OUT();
    IIC_SCL=0;
    IIC_SDA=0;
    delay_us(4);
    IIC_SCL=1;
    IIC_SDA=1;
    delay_us(4);
}

/*******************************************************************************
* Function Name  : IIC_WaitAck
* Description    : I2C Wait peer device ACK Signal.
* Input          : N/A
* Output         : N/A
* Return         : If succussful, return 0. else return 1;
*******************************************************************************/
uint8_t IIC_WaitAck(void)
{
    uint8_t ucErrTime=0;
    SDA_IN();
    IIC_SDA=1;delay_us(1);     
    IIC_SCL=1;delay_us(1);   
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime > IIC_WAIT_ACK_TO)
        {
            IIC_GenerateStop();
            return 1;
        }
    }
    IIC_SCL=0;
    return 0;
}

/*******************************************************************************
* Function Name  : IIC_GenerateAck
* Description    : I2C Generate ACK Signal.
* Input          : N/A
* Output         : N/A
* Return         : N/A
*******************************************************************************/
static void IIC_GenerateAck(void)
{
    IIC_SCL=0;
    SDA_OUT();
    IIC_SDA=0;
    delay_us(2);
    IIC_SCL=1;
    delay_us(2);
    IIC_SCL=0;
}

/*******************************************************************************
* Function Name  : IIC_GenerateNack
* Description    : I2C Generate NACK Signal.
* Input          : N/A
* Output         : N/A
* Return         : N/A
*******************************************************************************/
static void IIC_GenerateNack(void)
{
    IIC_SCL=0;
    SDA_OUT();
    IIC_SDA=1;
    delay_us(2);
    IIC_SCL=1;
    delay_us(2);
    IIC_SCL=0;
}

/*******************************************************************************
* Function Name  : IIC_SendByte
* Description    : I2C Send one byte.
* Input          : The data which will be send.
* Output         : N/A
* Return         : N/A
*******************************************************************************/
void IIC_SendByte(uint8_t txd)
{
    uint8_t t;
    SDA_OUT();
    IIC_SCL=0;
    for(t=0;t<8;t++)
    {
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
        delay_us(2);
        IIC_SCL=1;
        delay_us(2);
        IIC_SCL=0;
        delay_us(2);
    }
}

/*******************************************************************************
* Function Name  : IIC_ReadByte
* Description    : I2C Read one byte.
* Input          : When finished read data, send ACK or NACK
* Output         : N/A
* Return         : The data which read from IIC
*******************************************************************************/
uint8_t IIC_ReadByte(unsigned char ack)
{
    unsigned char i,receive=0;
    SDA_IN();
    for(i=0;i<8;i++)
    {
        IIC_SCL=0; 
        delay_us(2);
        IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;
        delay_us(1);
    }
    if (!ack)
        IIC_GenerateNack();
    else
        IIC_GenerateAck();
    return receive;
}
