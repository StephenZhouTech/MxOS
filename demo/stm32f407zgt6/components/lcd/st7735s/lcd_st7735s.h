#ifndef __LCD_ST7735S_H__
#define __LCD_ST7735S_H__

#include <stdint.h>
#include "stm32f4xx.h"

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 128
#define LCD_H 128

#else
#define LCD_W 128
#define LCD_H 128
#endif

// CS => PA1
#define LCD_CS_LOW()   GPIO_ResetBits(GPIOA,GPIO_Pin_1)
#define LCD_CS_HIGH()  GPIO_SetBits(GPIOA,GPIO_Pin_1)

// BL => PA2
#define LCD_BLK_LOW()  GPIO_ResetBits(GPIOA,GPIO_Pin_2)
#define LCD_BLK_HIGH() GPIO_SetBits(GPIOA,GPIO_Pin_2)

// RST => PA3
#define LCD_RST_LOW()  GPIO_ResetBits(GPIOA,GPIO_Pin_3)
#define LCD_RST_HIGH() GPIO_SetBits(GPIOA,GPIO_Pin_3)

// RS => PA4
#define LCD_CTR_LOW()   GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define LCD_CTR_HIGH()  GPIO_SetBits(GPIOA,GPIO_Pin_4)

void LCD_ST7735S_Init(void);
uint8_t SPI1_ReadWriteByte(uint8_t data);

void LCD_WriteData(uint8_t data);
void LCD_WriteData_16Bit(uint16_t data);
void LCD_WriteCmd(uint8_t cmd);
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);

#endif //__LCD_ST7735S_H__
