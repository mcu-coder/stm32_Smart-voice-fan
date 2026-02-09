#ifndef __GPIO_H
#define __GPIO_H


/**********************************
包含头文件
**********************************/
#include "sys.h"


/**********************************
重定义关键词
**********************************/
#define LEDS_GPIO_CLK                 RCC_APB2Periph_GPIOC        //最小系统LED
#define LEDS_GPIO_PORT                GPIOC
#define LEDS_GPIO_PIN                 GPIO_Pin_13
#define LED_SYS                       PCout(13)

#define HUMAN_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define HUMAN_GPIO_PORT                GPIOA
#define HUMAN_GPIO_PIN                 GPIO_Pin_4
#define HUMAN                          PAin(4)


/**********************************
函数声明
**********************************/
void Gpio_Init(void);													//GPIO初始化


#endif
