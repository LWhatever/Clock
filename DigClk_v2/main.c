
//Anthor:LWhatever
#include <msp430f6638.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dr_lcdseg.h"   //调用段式液晶驱动头文件

//-----对状态机进行宏定义-----
#define IDLE 0
#define SHORT 1
#define LONG 2
#define COUNTER_THRESHOLD 30 //长键判别门限

unsigned char WDT_Counter=0; //用于对按键按下时间进行计数
void WDT_Init();
void P40_Check();
unsigned char LongClick_Dect();
void P40_OnLongClick();

unsigned int second=3,minute=14,hour=10,year=2012,month=2,day=31,temp=405,alarm_temp=890;//电子钟显示变量
int month_mode=1,temp_mode=-1,check_mode=-1;//月份天数判断，温度设置
unsigned int display_mode=0,set_mode=0;//显示标志位，设置标志位

void GPIO_Init();
void Timer_Init();
void Adjust_clock();
void P4_IOCheck();
void P40_Onclick();
void P42_Onclick();
void ADC_Init();
void P40_Check();
void Set();
void Show();

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// 关闭看门狗
    WDT_Init();                 //看门狗初始化为定时模式
    Timer_Init();               //定时器初始化
    ADC_Init();                 //ADC初始化
    GPIO_Init();
    initLcdSeg();           //初始化段式液晶
	_enable_interrupts();   //开中断
	while(1)
	{
		if(check_mode)           //每分钟进行一次温度采样
		{                        
			ADC12CTL0 |= ADC12SC;//开始采样转换
			temp = ADC12MEM0;//把结果赋给变量
			check_mode=-check_mode;
		}
	}
	return 0;
}

void WDT_Init()
{
	WDTCTL=WDT_ADLY_16;
	SFRIE1 |= WDTIE;
}

void P40_Check()
{
	static unsigned char State=0; //状态机的状态变量
	static unsigned char Key_Now=0; //记录按键的当前电平
	unsigned char Key_Past=0; //记录按键的前一次电平
	Key_Past=Key_Now;
	//-----查询IO的输入寄存器-----
	if(P4IN&BIT0) 
		Key_Now=1; 
	else 
		Key_Now=0;
	//-----电平前高后低，表明按键按下事件-----
	if((Key_Past==1)&&(Key_Now==0))
	{
		switch(State)
		{
		case IDLE:WDT_Counter=0;State=SHORT;break; //按键被按下，切换到短按状态，并开始对按下时间计时
		default:break;
		}
	}
	if((Key_Past==0)&&(Key_Now==1))
	{
		switch(State)
		{
		case SHORT:
			State=IDLE;
			P40_Onclick();break; //按键弹起，仍然为短按状态，进入短按处理函数
		case LONG:
			WDT_Counter=0;
			State=IDLE;break; //计时完成后变成长按状态，计时状态清零
		default:break;
		}
	}
	if(LongClick_Dect())//进行按下后计时，计时通过后切换为长按状态
	{
		switch(State)
		{
		case SHORT:State=LONG;P40_OnLongClick();break; //进入长按处理函数
		default:break;
		}
	}
}

unsigned char LongClick_Dect()
{
	WDT_Counter++;
	if(WDT_Counter==COUNTER_THRESHOLD)
	{
		WDT_Counter=0;
		return(1);
	}
	else
		return(0);
}

void P40_OnLongClick() //P4.0长按的事件处理函数
{
	if(set_mode==0)
	{
		display_mode++;
		if(display_mode>4)
			display_mode=0;
		if(temp_mode==1){alarm_temp+=10;display_mode=3;}
	}
	while(!(P4IN & BIT0))
	{
		__delay_cycles(100000);
		if(set_mode>0)
		{
			set_mode==1?(year+=1000):(year+=0);
			set_mode==2?(year+=100):(year+=0);
			set_mode==3?(year+=10):(year+=0);
			set_mode==4?(year+=1):(year+=0);
			set_mode==5?(month+=1):(month+=0);
			set_mode==6?(day+=1):(day+=0);
			set_mode==7?(hour+=1):(hour+=0);
			set_mode==8?(minute+=1):(minute+=0);
			Adjust_clock();
		}
		if(display_mode==0)                       //实现长按闪烁
		{
			Set();
		}
	}
}

void GPIO_Init()
{
	P4REN |=BIT2;
	P4OUT |=BIT2;//S5//B键
	P4IES |= BIT2;                       // 中断沿设置（下降沿触发）
	P4IFG &= ~BIT2;                      // 清 P4.2 中断标志
	P4IE |= BIT2;                        // 使能 P4.2 口中断
	
	P4REN |=BIT0;
	P4OUT |=BIT0;//S7//A键
	P4IES |= BIT0;                       // 中断沿设置（下降沿触发）
	P4IFG &= ~BIT0;                      // 清 P4.0中断标志
	P4IE |= BIT0;

	P1DIR |= BIT5;                                 //控制蜂鸣器输出
	P1OUT &=~BIT5;
}

void ADC_Init()
{
	ADC12CTL0 |= ADC12MSC;//自动循环采样转换
	ADC12CTL0 |= ADC12ON;//启动ADC12模块
	ADC12CTL1 |= ADC12CONSEQ1 ;//选择单通道循环采样转换
	ADC12CTL1 |= ADC12SHP;//采样保持模式
	ADC12MCTL0 |= ADC12INCH_15; //选择通道15，连接拨码电位器
	ADC12CTL0 |= ADC12ENC;
}

void Timer_Init()
{
	TA0CTL |= MC_1 + TASSEL_1 + TACLR+ID_3;
	TA0CCTL0 = CCIE; //比较器中断使能
	TA0CCR0 = 4096; //比较值设为4096，相当于1 的时间间隔
}

void Adjust_clock()
{
	if(month==1||month==3||month==5||month==7||month==8||month==10||month==12)
	{
		month_mode=1;                         //31天月份
	}
	else if(month==2)
		{
			if((year%4==0&&year%100!=0)||(year%400==0))
				month_mode=2;                 //闰年2月
			else
				month_mode=-2;                //非闰年2月
		}
	else
		month_mode=3;                      //30天月份
	//时间调整
	if(second>59)
	{
		second = 0;
		check_mode=-check_mode;
		minute++;
	}
	if(minute>59)
	{
		minute = 0;
		hour++;
	}
	if(hour>23)
	{
		hour=0;
		day++;
	}
	//年月日调整
	switch(month_mode)
	{
	case 1:
		if(day>31)
		{
			day=1;
			month++;
		}break;
	case 2:
		if(day>29)
		{
			day=1;
			month++;
		}break;
	case -2:
		if(day>28)
		{
			day=1;
			month++;
		}break;
	case 3:
		if(day>30)
		{
			day=1;
			month++;
		}break;
	default:break;
	}
	if(month>12)
	{
		month=1;
		year++;
	}
}

void P4_IOCheck()                    //按键检测函数
{
	unsigned int Push_Key=0;

	Push_Key=P4IFG&(~P4DIR);

	__delay_cycles(10000);

	if((P4IN&Push_Key)==0)
	{
		switch(Push_Key)
		{
		case BIT2: P42_Onclick(); break;
		default: break;
		}
	}
}

void P40_Onclick()                   //短按处理函数
{
	if(set_mode==0)                  //判断是否处于设置状态，不是则进行显示模式的切换
	{
		display_mode++;
		if(display_mode>4)
			display_mode=0;
		if(temp_mode==1){alarm_temp+=10;display_mode=3;}//进入设置报警温度状态
	}
	if(set_mode>0)                   //处于设置状态，进一步对每个设置状态进行处理
	{
		set_mode==1?(year+=1000):(year+=0);//设置年状态对千位加一
		set_mode==2?(year+=100):(year+=0);//设置年状态对百位加一
		set_mode==3?(year+=10):(year+=0);//设置年状态对十位加一
		set_mode==4?(year+=1):(year+=0);//设置年状态对个位加一
		set_mode==5?(month+=1):(month+=0);//设置月状态
		set_mode==6?(day+=1):(day+=0);//设置日状态
		set_mode==7?(hour+=1):(hour+=0);//设置时状态
		set_mode==8?(minute+=1):(minute+=0);//设置分状态
		Adjust_clock();//调整时间的进位规则
	}
}

void P42_Onclick()                  //B键处理函数
{
	set_mode++;
	if(display_mode>0&&display_mode<3)//在显示状态中按下B键进入设置年状态1
	{
		display_mode=0;
		set_mode=1;
	}
	if(display_mode==3)               //在显示温度状态下，按下B键切换到报警温度设置状态
	{
		temp_mode=-temp_mode;
		set_mode=0;
		if(temp_mode==-1)
			display_mode=0;
	}
	if(display_mode==4)               //在显示秒钟状态下，按下B键秒钟归零
	{
		second=0;
		set_mode=0;
	}
	if(set_mode>8)
		set_mode=0;
}

void Show()
{
	switch(display_mode)
	{
	case 0:                                      //显示时间
		LCDSEG_ResetSpecSymbol(5);
		LCDSEG_DisplayNumber(minute,0);
		LCDSEG_DisplayNumber(hour,3);break;
	case 1:                                      //显示年
		LCDSEG_SetDigit(4,-1);
		LCDSEG_DisplayNumber(year,0);break;
	case 2:                                      //显示月日
		LCDSEG_DisplayNumber(day,0);
		LCDSEG_DisplayNumber(month,2);break;
	case 3:                                      //显示温度
		LCDMEM[1] = 0x01+0x02+0x08+0x04;
		LCDSEG_SetSpecSymbol(5);
		LCDSEG_SetDigit(2,-1);
		LCDSEG_SetDigit(1,-1);
		LCDSEG_SetDigit(0,-1);
		LCDSEG_SetDigit(3,-1);
		LCDSEG_SetDigit(5,-1);
		LCDSEG_DisplayNumber(temp,0);break;
	case 4:                                      //显示秒钟
		LCDSEG_SetDigit(2,-1);
		LCDSEG_SetDigit(3,-1);
		LCDSEG_SetDigit(4,-1);
		LCDSEG_ResetSpecSymbol(5);
		LCDSEG_DisplayNumber(second,0);break;
	default:break;
	}
}

void Set()                                      //设置函数，实质在设置状态形成闪烁效果
{
	switch(set_mode)
	{
	case 0:                                              //显示时间状态
		LCDSEG_DisplayNumber(minute,0);
		LCDSEG_DisplayNumber(hour,3);break;
	case 1:                                              //年份的千位闪烁
		LCDSEG_SetDigit(4,-1);
		LCDSEG_SetDigit(3,-1);__delay_cycles(100000);
		LCDSEG_DisplayNumber(year,0);break;
	case 2:                                              //年份的百位闪烁
		LCDSEG_SetDigit(2,-1);__delay_cycles(100000);
		LCDSEG_DisplayNumber(year,0);break;
	case 3:                                              //年份的十位闪烁
		LCDSEG_SetDigit(1,-1);__delay_cycles(100000);
		LCDSEG_DisplayNumber(year,0);break;
	case 4:                                              //年份的个位闪烁
		LCDSEG_SetDigit(0,-1);__delay_cycles(100000);
		LCDSEG_DisplayNumber(year,0);break;
	case 5:                                              //月闪烁
		LCDSEG_SetDigit(2,-1);
		LCDSEG_SetDigit(3,-1);__delay_cycles(100000);
		LCDSEG_DisplayNumber(day,0);
		LCDSEG_DisplayNumber(month,2);break;
	case 6:                                              //日闪烁
		LCDSEG_SetDigit(0,-1);
		LCDSEG_SetDigit(1,-1);__delay_cycles(100000);
		LCDSEG_DisplayNumber(day,0);
		LCDSEG_DisplayNumber(month,2);break;
	case 7:                                              //时闪烁
		LCDSEG_SetDigit(3,-1);
		LCDSEG_SetDigit(4,-1);__delay_cycles(100000);
		LCDMEM[3]=0x04;
		LCDSEG_DisplayNumber(minute,0);
		LCDSEG_DisplayNumber(hour,3);break;
	case 8:                                              //分闪烁
		LCDSEG_SetDigit(0,-1);
		LCDSEG_SetDigit(1,-1);__delay_cycles(100000);
		LCDMEM[3]=0x04;
		LCDSEG_DisplayNumber(minute,0);
		LCDSEG_DisplayNumber(hour,3);break;
	}
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	second++;
	if((set_mode==0)&&(display_mode==0))                //每秒闪烁
	{
		if(second%2==0)
			LCDMEM[3]=0;
		else
			LCDMEM[3]=0x04;
	}
	Adjust_clock();
	if(set_mode==0)
	{
		Show();
	}

	if(display_mode==0)
	{
		Set();
	}

	if(temp_mode==1)                                    //设置报警温度时闪烁
	{
		LCDMEM[1] = 0x01+0x02+0x08+0x04;
		LCDSEG_SetSpecSymbol(5);
		LCDSEG_SetDigit(5,-1);
		LCDSEG_SetDigit(3,-1);
		LCDSEG_SetDigit(2,-1);
		LCDSEG_SetDigit(1,-1);
		LCDSEG_SetDigit(0,-1);__delay_cycles(100000);
		LCDSEG_DisplayNumber(alarm_temp,0);
	}

	if(temp>alarm_temp)                                 //报警状态，LCD显示闪烁，蜂鸣器报警
	{
		display_mode=3;
		__delay_cycles(100000);
		LCDSEG_ResetSpecSymbol(5);
		LCDSEG_SetDigit(3,-1);
		LCDSEG_SetDigit(2,-1);
		LCDSEG_SetDigit(1,-1);
		LCDSEG_SetDigit(0,-1);
		P1OUT ^= BIT5;
	}
	if(temp<alarm_temp)                                 //报警结束关闭蜂鸣器
	{
		P1OUT &=~ BIT5;
	}
}

#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR()
{
	P40_Check();                                        //P4.0按键检测
}
#pragma vector = PORT4_VECTOR
__interrupt void PORT4_ISR()
{
	P4_IOCheck();                                       //P4.2按键检测
	P4IFG = 0;
}
