/******************************************************************************

                  版权所有 (C), 2020-2030, 重庆和天电子科技有限公司

 ******************************************************************************
  文 件 名  : AppVadcScan.h
  版 本 号  : 初稿
  作    者    :
  生成日期 : 2021年10月15日
  最近修改 :
  功能描述 : ADC采样结构体定义，函数声明
  函数列表 :
  修改历史 :
  1.日    期  :
  2.作    者  :
    修改内容   :

******************************************************************************/

#ifndef APPVADCSCAN_H
#define APPVADCSCAN_H 1


/*****************************************************************************/
/*----------------------------------包含头文件--------------------------------*/
/*****************************************************************************/
#include "Cpu0_Main.h"
#include "IfxVadc_Adc.h"
#include "AppVadcQueue.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
//#define ADScanChannelNum	6
#define ADScanChannelNum	3
#define ADScanChannelNum_atSelfTest 	4


/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef struct
{
    IfxVadc_Adc 	  vadc; 		/* VADC handle */
    IfxVadc_Adc_Group adcGroup0;
    IfxVadc_Adc_Group adcGroup1;
} App_VadcAutoScan;

typedef struct
{
	IfxVadc_Adc_Channel  g_VadcCh0;
	IfxVadc_Adc_Channel  g_VadcCh1;
	IfxVadc_Adc_Channel  g_VadcCh2;
	IfxVadc_Adc_Channel  g_VadcCh3;
	IfxVadc_Adc_Channel  g_VadcCh4;
	IfxVadc_Adc_Channel  g_VadcCh5;
	IfxVadc_Adc_Channel  g_VadcCh6;
	IfxVadc_Adc_Channel  g_VadcCh7;
	IfxVadc_Adc_Channel  g_VadcCh8;
	IfxVadc_Adc_Channel  g_VadcCh9;
	IfxVadc_Adc_Channel  g_VadcCh10;
	IfxVadc_Adc_Channel  g_VadcCh11;
} App_VadcChannel;

typedef struct
{
	uint16	AN000;			// G0通道0
	uint16	AN001;			// G0通道1
	uint16	AN002;			// G0通道2
	uint16	AN003;			// G0通道3
	uint16	AN004;			// G0通道4
	uint16	AN005;			// G0通道5
	uint16	AN006;			// G0通道6
	uint16	AN007;			// G0通道7
	uint16	AN008;			// G0通道8
	uint16	AN009;			// G0通道9
	uint16	AN010;			// G0通道10
	uint16	AN011;			// G0通道11
	uint16	AN100;			// G1通道0
	uint16	AN101;			// G1通道1
	uint16	AN102;			// G1通道2
	uint16	AN103;			// G1通道3
	uint16	AN104;			// G1通道4
	uint16	AN105;			// G1通道5
	uint16	AN106;			// G1通道6
	uint16	AN107;			// G1通道7
	uint16	AN108;			// G1通道8
	uint16	AN109;			// G1通道9
	uint16	AN110;			// G1通道10
	uint16	AN111;			// G1通道11
}Analog_Sample;


/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
extern Analog_Sample AnalogSample;

extern uint16 TLE9183VO1;
extern uint16 TLE9183VO3;
extern uint16 TLE9183VO2;
extern uint16 TLE9183VRO;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
extern void ADC_Scan_GetResult(void);
extern void VadcAutoScanChannel_init(void);

extern void VadcAutoScanChannel_atSelfTest_init(void);
extern void ADC_Scan_GetResult_atSelfTest(void);
extern void VadcManualScanSetNextChannelToConvert_atSelfTest(void);
extern void VadcManualScanReadScanChannelResult_atSelfTest(void);


#endif
