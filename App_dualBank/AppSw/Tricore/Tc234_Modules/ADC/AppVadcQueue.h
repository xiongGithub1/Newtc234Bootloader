/******************************************************************************

                  ��Ȩ���� (C), 2020-2030, ���������ӿƼ����޹�˾

 ******************************************************************************
  �� �� ��  : AppVadcQueue.h
  �� �� ��  : ����
  ��    ��    :
  �������� : 2021��10��15��
  ����޸� :
  �������� : ADC������������ṹ�嶨��
  �����б� :

  �޸���ʷ :
  1.��    ��  :
  2.��    ��  :
    �޸�����   :

******************************************************************************/

#ifndef APPVADC_H_
#define APPVADC_H_


/*****************************************************************************/
/*----------------------------------����ͷ�ļ�--------------------------------*/
/*****************************************************************************/
#include "Cpu0_Main.h"
#include "IfxVadc_Adc.h"



#define IFXVADC_QUEUE_EXT_TRIGGER	(1 << IFX_VADC_G_QINR0_EXTR_OFF)

/*----------------------------------------------*
 * �ṹ�嶨��                                   *
 *----------------------------------------------*/
typedef enum
{
	G0_Ch0_9183_VO1,
	G0_Ch3_9183_VO3,
	G1_Ch0_9183_VO2,
	G1_Ch3_9183_VRO,
	G1_Ch4_V_INV,
	G1_Ch5_KL15,
	G0_ch8_pressure,
	AppVadc_ChannelCount
}AppVadc_Channels;

typedef enum
{
	AppVadc_Group_0,
	AppVadc_Group_1,
	AppVadc_GroupCount
}AppVadc_Groups;


/*----------------------------------------------*
 * ȫ�ֱ���                                   *
 *----------------------------------------------*/
extern IfxVadc_Adc       	vadc;
extern IfxVadc_Adc_Config   adcConfig;
extern IfxVadc_Adc_Group 	adcGroup[AppVadc_GroupCount];

extern IfxVadc_Adc_GroupConfig     adcGroupConfig[AppVadc_GroupCount];
extern IfxVadc_Adc_ChannelConfig   adcChannelConfig[AppVadc_ChannelCount];
extern IfxVadc_Adc_Channel  adcChannel[AppVadc_ChannelCount];
extern uint32   		 	rawValue[AppVadc_ChannelCount];


/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                    *
 *----------------------------------------------*/
extern void AppVadcQueue_init(void);
extern void AppVadc_GetScanResult(void);
//extern void AppVadc_GetQueueResult(void);

#endif

