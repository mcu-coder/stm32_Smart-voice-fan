#include "sys.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "delay.h"
#include "gpio.h"
#include "key.h"
#include "oled.h"
#include "usart.h"
#include "ds18b20.h"
#include "motor_bujin.h"
#include "timer.h"
#include "motor_dianji.h"



/**********************************
变量定义
**********************************/
uint8_t key_num = 0;									//按键扫描标志位	
uint8_t flag_display = 0;							//显示界面标志位
uint32_t time_num = 0;								//10ms计时
char display_buf[32];									//显示数组

_Bool flag_mode = 0;									//模式标志位（自动|手动）
_Bool flag_direction = 0;							//摇头标志位
int flag_dir	= 0;

short temp_value = 0;                 //温度值
u16 temp_max = 30;                    //温度最大值
u16 temp_min = 20;										//温度最低值

s16 motor_pwm = 0;										//电机速度

int countdown_hour = 0;								//倒计时时
int countdown_minute = 0;							//倒计时分
int countdown_second = 0;							//倒计时秒
_Bool flag_countdown_begin = 0;				//倒计时开始标志位
_Bool flag_countdown_end = 0;					//倒计时结束标志位
extern _Bool flag_1s;									//1秒到达标志位

extern _Bool flag_bujin_foreward;			//步进电机打开标志位
extern _Bool flag_bujin_reversal;			//步进电机关闭标志位
extern _Bool flag_bujin_state;				//步进电机状态标志位

extern uint8_t usart1_buf[256];				//串口1接收数组
extern uint8_t usart2_buf[256];				//串口2接收数组


/**********************************
函数声明
**********************************/
void Key_function(void);							//按键函数
void Monitor_function(void);					//监测函数
void Display_function(void);					//显示函数
void Manage_function(void);						//处理函数


/****
*******	主函数 
*****/
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//配置中断分组为2号
	Delay_Init();	    	 								//延时初始化	  
	Key_Init();		  										//按键初始化
	while(DS18B20_Init());              //DS18B20初始化
	Oled_Init();		  									//OLED初始化
	Oled_Clear_All();										//清屏
	Usart1_Init(9600);  								//串口1初始化
	Usart2_Init(9600);  								//串口2初始化 
	Motor_Dianji_Init();								//直流电机初始化 
  Delay_ms(1000);
	Delay_ms(1000);

	while(1)
	{
		Key_function();										//按键函数
		Monitor_function();								//监测函数
		Display_function();								//显示函数
		Manage_function();								//处理函数

		time_num++;												//计时变量+1
		Delay_ms(10);
		if(time_num %5 == 0)							//最小系统LED闪烁
			LED_SYS = ~LED_SYS;
		if(time_num >= 5000)
		{
			time_num = 0;
		}
	}
}

/****
*******按键函数
*****/
void Key_function(void)
{
	key_num = Chiclet_Keyboard_Scan(0);					//按键扫描
	if(key_num != 0)														//有按键按下
	{
		switch(key_num)
		{
			case 1:																	//按键1：切换界面
				flag_display++;
				if(flag_display >= 6)
					flag_display = 0;
				
				Oled_Clear_All();
			break;
 
			case 2:																//按键2:切换模式
				if(flag_display == 0)
				{
					if(flag_mode == 0)
					{
						flag_mode = 1;
					}
					else
					{
						flag_mode = 0;
					}
				}
			break;

			default:
				break;
		}
	}
}

/****
*******监测函数
*****/
void Monitor_function(void)
{
	if(flag_display == 0)																			//测量界面
	{
		if(time_num % 10 == 0)																	//约2s检测一次
		{
      temp_value = DS18B20_Get_Temp();                      //获取温度值

			if(time_num % 30 == 0)																//发送数据
			{
				UsartPrintf(USART1,"\r\n温度：%d.%dC",temp_value/10,temp_value%10);
				if(HUMAN == 1)
					UsartPrintf(USART1,"  人体：有人");
				else
					UsartPrintf(USART1,"  人体：没有");
				if(motor_pwm == 0)
					UsartPrintf(USART1,"\r\n关闭");
				else if(motor_pwm == 300)
					UsartPrintf(USART1,"\r\n一档");
				else if(motor_pwm == 600)
					UsartPrintf(USART1,"\r\n二档");
				else if(motor_pwm == 900)
					UsartPrintf(USART1,"\r\n三档");
			 
			}
		}
		 
 
	}
}

/****
*******显示函数
*****/
void Display_function(void)
{
	switch(flag_display)																		//根据不同的显示模式标志位，显示不同的界面
	{
		case 0:																								//界面0：测量界面，显示温度、人体、档位、摇头、倒计时时间
			Oled_ShowCHinese(1, 0, "温度：");
			sprintf(display_buf,"%d.%dC ",temp_value/10,temp_value%10);
			Oled_ShowString(1, 6, display_buf);

			Oled_ShowCHinese(2, 0, "人体：");
			if(HUMAN == 1)
				Oled_ShowCHinese(2, 3, "有人");
			else
				Oled_ShowCHinese(2, 3, "没有");

			if(motor_pwm == 0)
				Oled_ShowCHinese(3, 0, "关闭");
			else if(motor_pwm == 300)
				Oled_ShowCHinese(3, 0, "一档");
			else if(motor_pwm == 600)
				Oled_ShowCHinese(3, 0, "二档");
			else if(motor_pwm == 900)
				Oled_ShowCHinese(3, 0, "三档");

			if(flag_direction == 1)
				Oled_ShowCHinese(3, 5, "摇头开");
			else
				Oled_ShowCHinese(3, 5, "摇头关");

			sprintf(display_buf,"%02d:%02d:%02d",countdown_hour,countdown_minute,countdown_second);
			Oled_ShowString(4,0,display_buf);

		 
		break;
 
		case 1:																								//界面5：显示设置倒计时秒
			Oled_ShowCHinese(1,1,"设置倒计时秒");
			if(time_num % 5 == 0)
			{
				sprintf(display_buf,"%02d:%02d:%02d",countdown_hour,countdown_minute,countdown_second);
				Oled_ShowString(2,4,display_buf);
			}
			if(time_num % 10 == 0)
			{
				Oled_ShowString(2,10,"  ");
			}
		break;

		default:
		break;
	}
}

/****
*******处理函数
*****/
void Manage_function(void)
{
	if(flag_display == 0)																			//测量界面
	{
		if(flag_mode == 0)											//自动模式下，（有人时，温度大于最大值三档、大于中值二档、大于最小值一档，否则关闭）
		{
			if((temp_value > temp_max*10) && HUMAN == 1)
			{
				motor_pwm = 900;
				TIM_SetCompare1(MOTOR_DIANJI_TIM,855+motor_pwm/20);
			}
			 
			else
			{
				motor_pwm = 0;
				TIM_SetCompare1(MOTOR_DIANJI_TIM,motor_pwm);
			}
		}
    if(countdown_hour != 0 || countdown_minute != 0 || countdown_second != 0)			//通风倒计时不为0
      flag_countdown_begin = 1;																										//开始倒计时，进入定时器中断
    if(flag_1s == 1)																															//1s到达
    {
      flag_1s = 0;
      if(countdown_second > 0)																										//倒计时秒>0
        countdown_second--;																												//倒计时秒-1
      else																																				//倒计时秒=0
      {
        if(countdown_minute > 0)																									//如果倒计时分>0
        {
          countdown_minute--;																											//倒计时分-1
          countdown_second = 59;																									//倒计时秒=59
        }
        else																																			//如果倒计时分=0
        {
          if(countdown_hour > 0)																									//如果倒计时时>0
          {
            countdown_hour--;																											//倒计时时-1
            countdown_minute = 59;																								//倒计时分=59
            countdown_second = 59;																								//倒计时秒=59
          }
          else																																		//如果倒计时时=0，倒计时结束
          {
            flag_countdown_begin = 0;
            flag_countdown_end = 1;
          }
        }
      }
    }
    
	}
	else																											//设置界面，关闭风扇
	{
		flag_countdown_begin = 0;
		flag_direction = 0;
		flag_bujin_foreward = 0;
		flag_bujin_reversal = 0;
		motor_pwm = 0;
		TIM_SetCompare1(MOTOR_DIANJI_TIM,motor_pwm);
	}
}

