
//Anthor:LWhatever_WHU
#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dr_lcdseg.h"   //调用段式液晶驱动头文件

#define XT2_FREQ   4000000//4兆

#define MCLK_FREQ 16000000
#define SMCLK_FREQ 4000000

void initClock() {
	while (BAKCTL & LOCKIO) //解锁XT1引脚操作
		BAKCTL &= ~(LOCKIO);
	UCSCTL6 &= ~XT1OFF; //启动XT1，选择内部时钟源
	P7SEL |= BIT2 + BIT3; //XT2引脚功能选择
	UCSCTL6 &= ~XT2OFF; //启动XT2
	while (SFRIFG1 & OFIFG) //等待XT1、XT2与DCO稳定
	{
		UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT2OFFG);
		SFRIFG1 &= ~OFIFG;
	}
	UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK; //避免DCO调整中跑飞
	UCSCTL1 = DCORSEL_5; //6000kHz~23.7MHz
	UCSCTL2 = MCLK_FREQ / (XT2_FREQ / 16); //XT2频率较高，分频后作为基准可获得更高的精度
	UCSCTL3 = SELREF__XT2CLK + FLLREFDIV__16; //XT2进行16分频后作为基准
	while (SFRIFG1 & OFIFG) //等待XT1、XT2与DCO稳定
	{
		UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT2OFFG);
		SFRIFG1 &= ~OFIFG;
	}
	UCSCTL5 = DIVA__1 + DIVS__1 + DIVM__1; //设定几个CLK的分频
	UCSCTL4 = SELA__XT1CLK + SELS__XT2CLK + SELM__DCOCLK; //设定几个CLK的时钟源
}

int second=0,minute=58,hour=23,hour1;//时钟时间
int asecond=0,aminute=0,ahour=0;//闹钟时间
int change=1,judge=0;
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// 停止看门狗
    initClock();             //配置系统时钟
    initLcdSeg();           //初始化段式液晶
    
    P1DIR |= BIT0+BIT6+BIT7;
    P4DIR |= BIT5;                       // 设置 P4.5 口方向为输出  
    P4DIR &= ~BIT2;                      // P4.2 置为输入  
    P4REN |= BIT2;                       // 使能 P4.2 上拉电阻 
    P4OUT |= BIT2;                       // P4.2 口置高电平 
    P4IES |= BIT2;                       // 中断沿设置（下降沿触发） 
    P4IFG &= ~BIT2;                      // 清 P4.2 中断标志 
    P4IE |= BIT2;                        // 使能 P4.2 口中断  
    __bis_SR_register(GIE);             // 开中断  
    
    P1DIR |= BIT5;                                 //控制蜂鸣器输出 
    P4DIR |= BIT6;                                 //控制 LED6 输出 
    
  //**************************************************************************
    
    P4OUT &= ~BIT5;
    P4DIR |= BIT6;
    P4DIR &= ~BIT3;
    P4REN |= BIT3;    //S4   
    P4OUT |= BIT3;
    P4DIR |= BIT7;
    P4DIR &= ~BIT1;
    P4REN |= BIT1;      //S6 
    P4OUT |= BIT1;
    P5DIR |= BIT7;
    P4DIR &= ~BIT0;
    P4REN |= BIT0;       //S7
    P4OUT |= BIT0;
    P8DIR |= BIT0; 
    P4DIR &= ~BIT4;
    P4REN |= BIT4;     //S3  
    P4OUT |= BIT4;
    
    while(1)
    {
    switch(change)
    {
    case 1:
  while(1)                          
  {
    P4OUT |= BIT5;
if(second==asecond)
    {
      judge=0;
    }
    if(minute==aminute && hour==ahour && second==asecond)
    {
      judge=1;
    }
    if(judge==1)
    {
      P1OUT ^= BIT5;                      //形成鸣叫效果 
      P4OUT ^= BIT6;                      //形成闪灯效果
      __delay_cycles(MCLK_FREQ/7);
      P1OUT ^= BIT5;                      //形成鸣叫效果 
      P4OUT ^= BIT6;
      if(!(P4IN & BIT4))
        judge=0;
     }
    second++;
    __delay_cycles(MCLK_FREQ);
    
    if(second>59)
    {
      second = 0;
      minute++;
      if(minute>59)
      {
        minute = 0;
        hour++;
        if(hour>23)
          hour=0;
      }
    } 
    LCDSEG_DisplayNumber(minute,2);
    LCDSEG_DisplayNumber(hour,4);
    LCDSEG_DisplayNumber(second,0);
    if(!(P4IN & BIT4))
    {
      change=2;
      break;
    }
  }break;
    case 2:
      while(1)                          
  {
    P4OUT |= BIT5;
 if(second==asecond)
    {
      judge=0;
    }
    if(minute==aminute && hour==ahour && second==asecond)
    {
      judge=1;
    }
    if(judge==1)
    {
      P1OUT ^= BIT5;                      //形成鸣叫效果 
      P4OUT ^= BIT6;                      //形成闪灯效果
      __delay_cycles(MCLK_FREQ/7);
      P1OUT ^= BIT5;                      //形成鸣叫效果 
      P4OUT ^= BIT6;
      if(!(P4IN & BIT4))
        judge=0;
     }
    second++;
    __delay_cycles(MCLK_FREQ);
   
    if(second>59)
    {
      second = 0;
      minute++;
      if(minute>59)
      {
        minute = 0;
        hour++;
      }
      if(hour>23)
          hour=0;
    } if(hour>12)
        hour1=hour-12;
    else
      hour1=hour;
    LCDSEG_DisplayNumber(minute,2);
    LCDSEG_DisplayNumber(hour1,4);
    LCDSEG_DisplayNumber(second,0);
    if(!(P4IN & BIT4))
    {
      change=1;
      break;
    }
  }break;
  //**************************************************************
    }
}
}
#pragma vector=PORT4_VECTOR 
__interrupt void Port_4(void)
  {
    int i,n=1;

    //********************************************************************
    while(1)
    {
    switch(n)
    {
    case 1://设置秒钟
    while(1)
    {
      LCDSEG_SetDigit(0,-1);
      LCDSEG_SetDigit(1,-1);
      __delay_cycles(MCLK_FREQ/10);
      LCDSEG_DisplayNumber(second,0);
      LCDSEG_DisplayNumber(minute,2);
      LCDSEG_DisplayNumber(hour,4);
      __delay_cycles(MCLK_FREQ/10);
      if(!(P4IN & BIT3))
      {
        second++;
        second=second>59?0:second;
        LCDSEG_DisplayNumber(second,0);
      }
      if(!(P4IN & BIT1))
      {
        second--;
        second=second<0?59:second;
        LCDSEG_DisplayNumber(second,0);
      }
      if(!(P4IN & BIT0))
      { n++;break;}
      if(!(P4IN & BIT4))
        goto part1;
      if(!(P4IN & BIT2))
        goto part1;
    }break;
    
    case 2://设置分钟
    while(1)
    {
      LCDSEG_SetDigit(2,-1);
      LCDSEG_SetDigit(3,-1);
      __delay_cycles(MCLK_FREQ/10);
      LCDSEG_DisplayNumber(second,0);
      LCDSEG_DisplayNumber(minute,2);
      LCDSEG_DisplayNumber(hour,4);
      __delay_cycles(MCLK_FREQ/10);
      if(!(P4IN & BIT3))
      {
        minute++;
        minute=minute>59?0:minute;
        LCDSEG_DisplayNumber(minute,2);
      }
      if(!(P4IN & BIT1))
      {
        minute--;
        minute=minute<0?59:minute;
        LCDSEG_DisplayNumber(minute,2);
      }
      if(!(P4IN & BIT0))
      { n++;break;}
      if(!(P4IN & BIT4))
      { n--;break;}
      if(!(P4IN & BIT2))
        goto part1;
    }break;
    
    case 3://设置小时
    while(1)
    {
      LCDSEG_SetDigit(4,-1);
      LCDSEG_SetDigit(5,-1);
      __delay_cycles(MCLK_FREQ/10);
      LCDSEG_DisplayNumber(second,0);
      LCDSEG_DisplayNumber(minute,2);
      LCDSEG_DisplayNumber(hour,4);
      __delay_cycles(MCLK_FREQ/10);
      if(!(P4IN & BIT3))
      {
        hour++;
        hour=hour>23?0:hour;
        LCDSEG_DisplayNumber(hour,4);
      }
      if(!(P4IN & BIT1))
      {
        hour--;
        hour=hour<0?23:hour;
        LCDSEG_DisplayNumber(hour,4);
      }
      if(!(P4IN & BIT0))
      { n++;break;}
      if(!(P4IN & BIT4))
      { n--;break;}
      if(!(P4IN & BIT2))
        goto part1;
    }break;
    //**************************************************************************
    //设置闹钟
    
    case 4://设置秒钟
    while(1)
    {
      LCDSEG_DisplayNumber(asecond,0);
      LCDSEG_DisplayNumber(aminute,2);
      LCDSEG_DisplayNumber(ahour,4);
      LCDSEG_SetDigit(0,-1);
      LCDSEG_SetDigit(1,-1);
      __delay_cycles(MCLK_FREQ/10);
      LCDSEG_DisplayNumber(asecond,0);
      __delay_cycles(MCLK_FREQ/10);
      if(!(P4IN & BIT3))
      {
        asecond++;
        asecond=asecond>59?0:asecond;
        LCDSEG_DisplayNumber(asecond,0);
      }
      if(!(P4IN & BIT1))
      {
        asecond--;
        asecond=asecond<0?59:asecond;
        LCDSEG_DisplayNumber(asecond,0);
      }
      if(!(P4IN & BIT0))
      { n++;break;}
      if(!(P4IN & BIT4))
      { n--;break;}
      if(!(P4IN & BIT2))
        goto part1;
    }break;
    
    case 5://设置分钟
    while(1)
    {
      LCDSEG_DisplayNumber(asecond,0);
      LCDSEG_DisplayNumber(aminute,2);
      LCDSEG_DisplayNumber(ahour,4);
      LCDSEG_SetDigit(2,-1);
      LCDSEG_SetDigit(3,-1);
      __delay_cycles(MCLK_FREQ/10);
      LCDSEG_DisplayNumber(aminute,2);
      __delay_cycles(MCLK_FREQ/10);
      if(!(P4IN & BIT3))
      {
        aminute++;
        aminute=aminute>59?0:aminute;
        LCDSEG_DisplayNumber(aminute,2);
      }
      if(!(P4IN & BIT1))
      {
        aminute--;
        aminute=aminute<0?59:aminute;
        LCDSEG_DisplayNumber(aminute,2);
      }
      if(!(P4IN & BIT0))
      { n++;break;}
      if(!(P4IN & BIT4))
      { n--;break;}
      if(!(P4IN & BIT2))
        goto part1;
    }break;
    
    case 6://设置小时
    while(1)
    {
      LCDSEG_DisplayNumber(asecond,0);
      LCDSEG_DisplayNumber(aminute,2);
      LCDSEG_DisplayNumber(ahour,4);
      LCDSEG_SetDigit(4,-1);
      LCDSEG_SetDigit(5,-1);
      __delay_cycles(MCLK_FREQ/10);
      LCDSEG_DisplayNumber(ahour,4);
      __delay_cycles(MCLK_FREQ/10);
      if(!(P4IN & BIT3))
      {
        ahour++;
        ahour=ahour>23?0:ahour;
        LCDSEG_DisplayNumber(ahour,4);
      }
      if(!(P4IN & BIT1))
      {
        ahour--;
        ahour=ahour<0?23:ahour;
        LCDSEG_DisplayNumber(ahour,4);
      }
      if(!(P4IN & BIT0))
        goto part1;
      if(!(P4IN & BIT4))
      { n--;break;}
      if(!(P4IN & BIT2))
        goto part1;
    }break;
    }
    }
  part1:i=0;
    P4IFG &= ~BIT2;                     // 清 P4.0 中断标志位 
  }


