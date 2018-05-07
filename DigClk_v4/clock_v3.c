//*************************************************************
//不同于main.c中的直接设置时间，此代码为每次按键按下后时间+1或者-1，此代码更加稳定。
//Anthor:LWhatever_WHU
//*************************************************************
#include <msp430f6638.h>
#include "oled.h"
#include "bmp.h"
/*
 * main.c
 */

int second=3,minute=14,hour=10,month=3,year=2018,day=1,Hour = 0,iWeek = 0;
int mon=0, ye=0;
int judge1 = 1,judge2 = 1,judge3 = 1; //闹钟到达标志
int month_mode = 1, set_mode = 0, set_alarm = 0,display = 1;//月份天数判断
int alarm[9] = {0,0,1,0,0,1,0,0,1};//存储闹钟值以及使能标志
unsigned int KeyVal = 16; //键值

void SetClock_MCLK125KHz()
{
	  P7SEL |= BIT2+BIT3;                       // Port select XT2

	  UCSCTL5 |= DIVS_5;
	  UCSCTL6 &= ~XT2OFF + XT2DRIVE_1;                       // Enable XT2
	  UCSCTL3 |= SELREF_2;                      // FLLref = REFO
	                                            // Since LFXT1 is not used,
	                                            // sourcing FLL with LFXT1 can cause
	                                            // XT1OFFG flag to set
	  UCSCTL4 |= SELA_2;                        // ACLK=REFO,SMCLK=DCO,MCLK=DCO

	  // Loop until XT1,XT2 & DCO stabilizes - in this case loop until XT2 settles
	  do
	  {
	    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
	                                            // Clear XT2,XT1,DCO fault flags
	    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

	  UCSCTL6 &= ~XT2DRIVE0;                    // Decrease XT2 Drive according to
	                                            // expected frequency
	  UCSCTL4 |= SELS_5 + SELM_5;               // SMCLK=MCLK=XT2
}
/*
	OLED_ShowNum(0+24, 0, year, 4, 16);
	OLED_ShowChar(32+24,0,'/');
	OLED_ShowNum(40+24, 0, month/10, 1, 16);
	OLED_ShowNum(48+24, 0, month%10, 1, 16);
	OLED_ShowChar(56+24,0,'/');
	OLED_ShowNum(64+24, 0, day/10, 1, 16);
	OLED_ShowNum(72+24, 0, day%10, 1, 16);
	OLED_ShowNum(24+8, 2, hour/10, 1, 16);
	OLED_ShowNum(32+8, 2, hour%10, 1, 16);
	OLED_ShowChar(40+8,2,':');
	OLED_ShowNum(48+8, 2, minute/10, 1, 16);
	OLED_ShowNum(56+8, 2, minute%10, 1, 16);
	OLED_ShowChar(64+8,2,':');
	OLED_ShowNum(72+8, 2, second/10, 1, 16);
	OLED_ShowNum(80+8, 2, second%10, 1, 16);
*/
void flash()
{
	switch(set_mode)
	{
		case 0:break;
		case 1:                                              //年份的千位闪烁
			OLED_ShowChar(0+24, 0, ' ');__delay_cycles(1000000);
			OLED_ShowNum(0+24, 0, year/1000, 1, 16);break;
		case 2:                                              //年份的百位闪烁
			OLED_ShowChar(8+24, 0, ' ');__delay_cycles(1000000);
			OLED_ShowNum(8+24, 0, (year-(year/1000)*1000)/100, 1, 16);break;
		case 3:                                              //年份的十位闪烁
			OLED_ShowChar(16+24, 0, ' ');__delay_cycles(1000000);
			OLED_ShowNum(16+24, 0, (year-(year/100)*100)/10, 1, 16);break;
		case 4:                                              //年份的个位闪烁
			OLED_ShowChar(24+24, 0, ' ');__delay_cycles(1000000);
			OLED_ShowNum(24+24, 0, year%10, 1, 16);break;
		case 5:                                              //月闪烁
			OLED_ShowChar(40+24, 0, ' ');
			OLED_ShowChar(48+24, 0, ' ');__delay_cycles(1000000);
			OLED_ShowNum(40+24, 0, month/10, 1, 16);
			OLED_ShowNum(48+24, 0, month%10, 1, 16);break;
		case 6:                                              //日闪烁
			OLED_ShowChar(64+24, 0, ' ');
			OLED_ShowChar(72+24, 0, ' ');__delay_cycles(1000000);
			OLED_ShowNum(64+24, 0, day/10, 1, 16);
			OLED_ShowNum(72+24, 0, day%10, 1, 16);break;
		case 7:                                              //时闪烁
			OLED_ShowChar(24+8, 2, ' ');
			OLED_ShowChar(32+8, 2, ' ');__delay_cycles(1000000);
			OLED_ShowNum(24+8, 2, hour/10, 1, 16);
			OLED_ShowNum(32+8, 2, hour%10, 1, 16);break;
		case 8:                                              //分闪烁
			OLED_ShowChar(48+8, 2, ' ');
			OLED_ShowChar(56+8, 2, ' ');__delay_cycles(1000000);
			OLED_ShowNum(48+8, 2, minute/10, 1, 16);
			OLED_ShowNum(56+8, 2, minute%10, 1, 16);break;
		default:break;
	}
}

void Adjust_clock_al()
{
	//alarm1时间调整
	if(alarm[0]>59)
	{
		alarm[0] = 0;
		alarm[1]++;
	}
	else if(alarm[0]<0)
	{
		alarm[0] = alarm[0] + 60;
		alarm[1]--;
	}
	if(alarm[1]>23)
	{
		alarm[1]=0;
	}
	else if(alarm[1]<0)
	{
		alarm[1] = alarm[1] + 24;
	}
	//alarm2时间调整
	if(alarm[3]>59)
	{
		alarm[3] = 0;
		alarm[4]++;
	}
	else if(alarm[3]<0)
	{
		alarm[3] = alarm[3] + 60;
		alarm[4]--;
	}
	if(alarm[4]>23)
	{
		alarm[4]=0;
	}
	else if(alarm[4]<0)
	{
		alarm[4] = alarm[4] + 24;
	}

	//alarm3时间调整
	if(alarm[6]>59)
	{
		alarm[6] = 0;
		alarm[7]++;
	}
	else if(alarm[6]<0)
	{
		alarm[6] = alarm[6] + 60;
		alarm[7]--;
	}
	if(alarm[7]>23)
	{
		alarm[7]=0;
	}
	else if(alarm[7]<0)
	{
		alarm[7] = alarm[7] + 24;
	}
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
		minute++;
	}
	if(minute>59)
	{
		minute = 0;
		hour++;
	}
	else if(minute<0)
	{
		minute = minute + 60;
		hour--;
	}
	if(hour>23)
	{
		hour=0;
		day++;
	}
	else if(hour<0)
	{
		hour = hour + 24;
		day--;
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
	//extra thinking
	if(month>12)
	{
		month=1;
		year++;
	}
	else if(month<1)
	{
		month = month + 12;
		year--;
	}
	if(day<1)
	{
		day = 1;
	}
	if(year<1000)
	{
		year = 1000;
	}
	if(month==1||month==2)//判断month是否为1或2
	{
		ye = year - 1;
		mon = month + 12;
	}
	else
	{
		ye = year;
		mon = month;
	}
	int c=ye/100;
	int y=year-c*100;
	iWeek=(c/4)-2*c+(y+y/4)+(13*(mon+1)/5)+day-1;
	while(iWeek<0){iWeek+=7;}
	iWeek%=7;
	if(hour == 0)
		Hour = 12;
	else if(hour>12)
		Hour = hour-12;
	else
		Hour = hour;
}


void show()
{
	// second = KeyVal == 3?50:second;
	// minute = KeyVal == 3?59:minute;
	// hour = KeyVal == 3?23:hour;
	// day = KeyVal == 3?29:day;
	// month = KeyVal == 3?2:month;
	// year = KeyVal == 3?2016:year;
	// KeyVal = 16;
	if(set_alarm==0)
	{
		OLED_ShowNum(0+24, 0, year, 4, 16);
		OLED_ShowChar(32+24,0,'/');
		OLED_ShowNum(40+24, 0, month/10, 1, 16);
		OLED_ShowNum(48+24, 0, month%10, 1, 16);
		OLED_ShowChar(56+24,0,'/');
		OLED_ShowNum(64+24, 0, day/10, 1, 16);
		OLED_ShowNum(72+24, 0, day%10, 1, 16);
		if(display==1)
		{
			OLED_ShowNum(24+8, 2, hour/10, 1, 16);
			OLED_ShowNum(32+8, 2, hour%10, 1, 16);
			OLED_ShowString(88+8,2,"    ");
		}
		else
		{
			OLED_ShowNum(24+8, 2, Hour/10, 1, 16);
			OLED_ShowNum(32+8, 2, Hour%10, 1, 16);
			if(hour<13&&hour>0)
				OLED_ShowString(88+8,2,"AM   ");
			else
				OLED_ShowString(88+8,2,"PM   ");
		}
		OLED_ShowChar(40+8,2,':');
		OLED_ShowNum(48+8, 2, minute/10, 1, 16);
		OLED_ShowNum(56+8, 2, minute%10, 1, 16);
		OLED_ShowChar(64+8,2,':');
		OLED_ShowNum(72+8, 2, second/10, 1, 16);
		OLED_ShowNum(80+8, 2, second%10, 1, 16);
		switch(iWeek)
		{
		case 1:OLED_ShowString(48+8,4,"Mon");break;
		case 2:OLED_ShowString(48+8,4,"Tue");break;
		case 3:OLED_ShowString(48+8,4,"Wen");break;
		case 4:OLED_ShowString(48+8,4,"Tir");break;
		case 5:OLED_ShowString(48+8,4,"Fri");break;
		case 6:OLED_ShowString(48+8,4,"Sat");break;
		case 0:OLED_ShowString(48+8,4,"Sun");break;
		default:break;
		}
	}
	else if(set_alarm>0)
	{
		switch(set_alarm)
		{
		case 1:
		case 2:
			OLED_ShowString(0+24, 0,"               ");
			OLED_ShowString(56, 4,"    ");
			OLED_ShowNum(32, 2, 1, 1, 16);
			OLED_ShowChar(40,2,')');
			OLED_ShowChar(48,2,' ');
			OLED_ShowNum(48+8, 2, alarm[1]/10, 1, 16);
			OLED_ShowNum(56+8, 2, alarm[1]%10, 1, 16);
			OLED_ShowChar(64+8,2,':');
			OLED_ShowNum(72+8, 2, alarm[0]/10, 1, 16);
			OLED_ShowNum(80+8, 2, alarm[0]%10, 1, 16);
			if(alarm[2]==-1)
				OLED_ShowString(88+8,2," ON ");
			else
				OLED_ShowString(88+8,2," OFF");
			break;
		case 3:
			OLED_ShowString(0+24, 0,"               ");
			OLED_ShowString(56, 4,"    ");
			OLED_ShowNum(32, 2, 2, 1, 16);
			OLED_ShowChar(40,2,')');
			OLED_ShowChar(48,2,' ');
			OLED_ShowNum(48+8, 2, alarm[4]/10, 1, 16);
			OLED_ShowNum(56+8, 2, alarm[4]%10, 1, 16);
			OLED_ShowChar(64+8,2,':');
			OLED_ShowNum(72+8, 2, alarm[3]/10, 1, 16);
			OLED_ShowNum(80+8, 2, alarm[3]%10, 1, 16);
			if(alarm[5]==-1)
				OLED_ShowString(88+8,2," ON ");
			else
				OLED_ShowString(88+8,2," OFF");
			break;
		case 5:
		case 6:
			OLED_ShowString(0+24, 0,"               ");
			OLED_ShowString(56, 4,"    ");
			OLED_ShowNum(32, 2, 3, 1, 16);
			OLED_ShowChar(40,2,')');
			OLED_ShowChar(48,2,' ');
			OLED_ShowNum(48+8, 2, alarm[7]/10, 1, 16);
			OLED_ShowNum(56+8, 2, alarm[7]%10, 1, 16);
			OLED_ShowChar(64+8,2,':');
			OLED_ShowNum(72+8, 2, alarm[6]/10, 1, 16);
			OLED_ShowNum(80+8, 2, alarm[6]%10, 1, 16);
			if(alarm[8]==-1)
				OLED_ShowString(88+8,2," ON ");
			else
				OLED_ShowString(88+8,2," OFF");
			break;
		default:break;
		}
	}

}

void set_time()
{
	if(set_mode)                   //处于设置状态，进一步对每个设置状态进行处理
	{
		if(KeyVal == 6)
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
		else if(KeyVal == 5)
		{
			set_mode==1?(year-=1000):(year-=0);//设置年状态对千位加一
			set_mode==2?(year-=100):(year-=0);//设置年状态对百位加一
			set_mode==3?(year-=10):(year-=0);//设置年状态对十位加一
			set_mode==4?(year-=1):(year-=0);//设置年状态对个位加一
			set_mode==5?(month-=1):(month-=0);//设置月状态
			set_mode==6?(day-=1):(day-=0);//设置日状态
			set_mode==7?(hour-=1):(hour-=0);//设置时状态
			set_mode==8?(minute-=1):(minute-=0);//设置分状态
			Adjust_clock();//调整时间的进位规则
		}
	}
}

void Set_Alarm()
{
	if(set_alarm)                   //处于设置状态，进一步对每个设置状态进行处理
	{
		if(KeyVal == 6)
		{
			set_alarm==1?(alarm[0]+=1):(alarm[0]+=0);//设置分状态1
			set_alarm==2?(alarm[1]+=1):(alarm[1]+=0);//设置时状态1
			set_alarm==3?(alarm[3]+=1):(alarm[3]+=0);//设置分状态1
			set_alarm==4?(alarm[4]+=1):(alarm[4]+=0);//设置时状态1
			set_alarm==5?(alarm[6]+=1):(alarm[6]+=0);//设置分状态1
			set_alarm==6?(alarm[7]+=1):(alarm[7]+=0);//设置时状态1
			Adjust_clock_al();//调整时间的进位规则
		}
		else if(KeyVal == 5)
		{
			set_alarm==1?(alarm[0]-=1):(alarm[0]-=0);//设置分状态1
			set_alarm==2?(alarm[1]-=1):(alarm[1]-=0);//设置时状态1
			set_alarm==3?(alarm[3]-=1):(alarm[3]-=0);//设置分状态1
			set_alarm==4?(alarm[4]-=1):(alarm[4]-=0);//设置时状态1
			set_alarm==5?(alarm[6]-=1):(alarm[6]-=0);//设置分状态1
			set_alarm==6?(alarm[7]-=1):(alarm[7]-=0);//设置时状态1
			Adjust_clock_al();//调整时间的进位规则
		}
	}
}

void Alarm()
{
	judge1 = (alarm[0]==minute&alarm[1]==hour)?1:0;
	judge2 = (alarm[3]==minute&alarm[4]==hour)?1:0;
	judge3 = (alarm[6]==minute&alarm[7]==hour)?1:0;
	if(alarm[2]==-1&judge1)
	{
		P6OUT ^= BIT0;
	}
	if(alarm[5]==-1&judge2)
	{
		P6OUT ^= BIT1;
	}
	if(alarm[8]==-1&judge3)
	{
		P6OUT ^= BIT2;
	}
}

const unsigned char KeyOut[4] = { 0xef, 0xdf, 0xbf, 0x7f }; //4X4按输出端控制

void Key_Head() {
	static unsigned int ReadData[4];
	int i;
	for (i = 0; i < 4; i++) {
		P4OUT = KeyOut[i] | 0x0f; //忽略低4位
		ReadData[i] = (P4IN | 0xf0) ^ 0xff;
		// CF[i] = ReadData[i] & (ReadData[i] ^ Cont[i]);
		// Cont[i] = ReadData[i];
//输出键值
		switch (ReadData[i]) //第i列
		{
		case 0x08:
			KeyVal = 4 * i + 3;
			//show();
			break;
		case 0x04:
			KeyVal = 4 * i + 2;
			//show();
			break;
		case 0x02:
			KeyVal = 4 * i + 1;
			//show();
			break;
		case 0x01:
			KeyVal = 4 * i;
			//show();
			break;
		default:
			break;
		}
	}
}

void Timer_Init()
{
	TA0CTL |= MC_1 + TASSEL_2 + TACLR+ID_3;
	TA0CCTL0 = CCIE; //比较器中断使能
	TA0CCR0 = 15625; //比较值设为4096，相当于1s的时间间隔

	TA1CCTL1 = CCIE;                          // CCR0 interrupt enabled
	TA1CCR0 = 10000;
	TA1CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, upmode, clear TAR
}
void GPIO_Init()
{
	P4REN |= BIT0 + BIT1 + BIT2 + BIT3;
	P4OUT |= BIT0 + BIT1 + BIT2 + BIT3;
	OLED_Init();		//初始化OLED
	P4DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3);
	P4DIR |= BIT4 + BIT5 + BIT6 + BIT7;
	P6DIR |= BIT0 + BIT1 + BIT2;
	P6OUT &= ~(BIT0 + BIT1 + BIT2);
	//P3DIR |= BIT2;
}

//void ADC_Init()
//{
//	ADC12CTL0 |= ADC12MSC;//自动循环采样转换
//	ADC12CTL0 |= ADC12ON;//启动ADC12模块
//	ADC12CTL1 |= ADC12CONSEQ1 ;//选择单通道循环采样转换
//	ADC12CTL1 |= ADC12SHP;//采样保持模式
//	ADC12MCTL0 |= ADC12INCH_15; //选择通道15，连接拨码电位器
//	ADC12CTL0 |= ADC12ENC;
//}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    SetClock_MCLK125KHz();
    GPIO_Init();
    Timer_Init();
    OLED_Clear();
    //ADC_Init();
    int i,i1;
    for(i = 0;i<7;i++)
    {
    OLED_ShowCHinese(4*i, i, 0);		//数
    OLED_ShowCHinese(4*i+18, i, 1);		//字
    OLED_ShowCHinese(4*i+36, i, 2);		//钟
    __delay_cycles(1000);
    OLED_Clear();
    }
    for(i1 = 6;i1>=0;i1--)
	{
    i++;
	OLED_ShowCHinese(5*i, i1, 0);		//数
	OLED_ShowCHinese(5*i+18, i1, 1);		//字
	OLED_ShowCHinese(5*i+36, i1, 2);		//钟
	__delay_cycles(1000);
	OLED_Clear();
	}
    __bis_SR_register(GIE+LPM3_bits);
    __no_operation();
	return 0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	second++;
	//P3OUT ^= BIT2;
	if(set_mode>0)
	{
		flash();
	}
	Alarm();
	Adjust_clock();
	if(second%2==0)
		OLED_ShowString(100,6,"^O^");

	else
		OLED_ShowString(100,6,"^o^");
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void)
{
	Key_Head();
	switch(KeyVal)
	{
		case 0:
			set_mode = 0;//0
			set_alarm++;
			set_alarm = set_alarm%7;
			KeyVal = 16;
			__delay_cycles(1000000);break;
		case 1:
			alarm[2] = (set_alarm==1|set_alarm==2)?-alarm[2]:alarm[2];//1
			alarm[5] = (set_alarm==3|set_alarm==4)?-alarm[5]:alarm[5];
			alarm[8] = (set_alarm==5|set_alarm==6)?-alarm[8]:alarm[8];
			KeyVal = 16;
			__delay_cycles(1000000);break;
		case 2:
			alarm[2] = 1;//2
			alarm[5] = 1;
			alarm[8] = 1;
			KeyVal = 16;
			__delay_cycles(1000000);break;
		case 3:break;
		case 4:
			set_mode = 0;//4
			set_alarm = 0;
			KeyVal = 16;
			__delay_cycles(1000000);break;
		case 5:
		case 6:
			set_time();//6
			Set_Alarm();
			KeyVal = 16;
			__delay_cycles(1000000);break;
		case 7:
			set_alarm = 0;//7
			set_mode++;
			set_mode = set_mode%10;
			second = set_mode == 9?0:second;
			KeyVal = 16;
			__delay_cycles(1000000);break;
		case 8: display = -display;//8
				KeyVal = 16;
				__delay_cycles(1000000);break;
		case 9:break;
		case 10:break;
		case 11:break;
		case 12:break;
		case 13:break;
		case 14:break;
		case 15:break;
		default:break;
	}
	show();
}
