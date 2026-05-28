/******************************************************************************

                  ��Ȩ���� (C), 2020-2030, ���������ӿƼ����޹�˾

 ******************************************************************************
  �� �� ��  : AppVadcQueue.c
  �� �� ��  : ����
  ��    ��    :
  �������� : 2021��10��15��
  ����޸� :
  �������� : ADC���������ʼ������ȡ�������
  �����б� :

  �޸���ʷ :
  1.��    ��  :
  2.��    ��  :
    �޸�����   :

******************************************************************************/


/*****************************************************************************/
/*----------------------------------����ͷ�ļ�--------------------------------*/
/*****************************************************************************/
#include "AppVadcQueue.h"
#include "ConfigurationIsr.h"


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
// VADC handle
IfxVadc_Adc         vadc;
IfxVadc_Adc_Config  adcConfig;
IfxVadc_Adc_Group   adcGroup[AppVadc_GroupCount];

// create group config
IfxVadc_Adc_GroupConfig     adcGroupConfig[AppVadc_GroupCount];

// create channel config
IfxVadc_Adc_ChannelConfig   adcChannelConfig[AppVadc_ChannelCount];
IfxVadc_Adc_Channel         adcChannel[AppVadc_ChannelCount];

//rawValue ԭʼ��ֵ
uint32 rawValue[AppVadc_ChannelCount];		// ����Ӧ�����з��Ż����޷����д���һ�����ǣ�����20210617



/***************************************************************
 �������ƣ�void AppVadcQueue_init(void)
 ���������
 ���������
 ����ע�ͣ�ADC���г�ʼ�����̣�����202100206
 1������ģ������
 2����ʼ��ģ��
 3��������0����
 4���趨���������
 5����ʼ����
 6��������1����
 7���趨���������
 8����ʼ����
 9������ͨ������
 10���趨����ͨ������
 11�������жϣ�������ͨ��������ʹ�ܺͽ�ֹ�жϿ���
 12����ʼ��ͨ��
**************************************************************/
void AppVadcQueue_init(void)
{
	uint8 i;
    IfxVadc_Adc_initModuleConfig(&adcConfig, &MODULE_VADC);		                        //��ʼ��ADCģ������
    // initialize VADC Module
    IfxVadc_Adc_initModule(&vadc, &adcConfig);		                                    //��ʼ��ADCģ�飬����
    /* create group config*/
	IfxVadc_Adc_initGroupConfig(&adcGroupConfig[AppVadc_Group_0], &vadc);		        //��ʼ����0���ã�����
	adcGroupConfig[AppVadc_Group_0].groupId = IfxVadc_GroupId_0;			            //ָ��ADC�ں�ID���飩
	adcGroupConfig[AppVadc_Group_0].master  = adcGroupConfig[AppVadc_Group_0].groupId;	//������ID

	adcGroupConfig[AppVadc_Group_0].queueRequest.triggerConfig.gatingMode    = IfxVadc_GatingMode_always; //�����ſش���
	adcGroupConfig[AppVadc_Group_0].queueRequest.triggerConfig.gatingSource  = IfxVadc_GatingSource_1;	  //�����ſ�Դ
	adcGroupConfig[AppVadc_Group_0].queueRequest.triggerConfig.triggerMode   = IfxVadc_TriggerMode_uponFallingEdge; //�ⲿ�¼��������½��ش���
	adcGroupConfig[AppVadc_Group_0].queueRequest.triggerConfig.triggerSource = IfxVadc_TriggerSource_15;  //����Դ�����ſ�Դ���ʹ��
	adcGroupConfig[AppVadc_Group_0].queueRequest.requestSlotStartMode = IfxVadc_RequestSlotStartMode_cancelInjectRepeat;//����ע��ģʽ

	adcGroupConfig[AppVadc_Group_0].inputClass[0].sampleTime = 5e-7;//1e-6;		         //ָ������ʱ��Ϊ 0.5US  ����
	adcGroupConfig[AppVadc_Group_0].inputClass[1].sampleTime = 5e-7;//1e-6;

	adcGroupConfig[AppVadc_Group_0].arbiter.requestSlotQueueEnabled = TRUE;              //����ģʽʹ��
	adcGroupConfig[AppVadc_Group_0].arbiter.requestSlotScanEnabled  = TRUE;              // ����ɨ��

	/* enable auto scan */
	adcGroupConfig[AppVadc_Group_0].scanRequest.autoscanEnabled = FALSE;  //single auto scan conversion
	/* enable all gates in "always" mode (no edge detection) */
	adcGroupConfig[AppVadc_Group_0].scanRequest.triggerConfig.gatingMode = IfxVadc_GatingMode_always;

	IfxVadc_Adc_initGroup(&adcGroup[AppVadc_Group_0], &adcGroupConfig[AppVadc_Group_0]); //��ʼ��ADC�ں�(��)

	IfxVadc_Adc_initGroupConfig(&adcGroupConfig[AppVadc_Group_1], &vadc);
	adcGroupConfig[AppVadc_Group_1].groupId = IfxVadc_GroupId_1;			             //��һ��ADC�ںˣ��飩
	adcGroupConfig[AppVadc_Group_1].master  = adcGroupConfig[AppVadc_Group_1].groupId;	 //������ID

	adcGroupConfig[AppVadc_Group_1].queueRequest.triggerConfig.gatingMode    = IfxVadc_GatingMode_always; //�����ſش���
	adcGroupConfig[AppVadc_Group_1].queueRequest.triggerConfig.gatingSource  = IfxVadc_GatingSource_1;    //�����ſ�Դ
	adcGroupConfig[AppVadc_Group_1].queueRequest.triggerConfig.triggerMode   = IfxVadc_TriggerMode_uponFallingEdge; //�½��ش���
	adcGroupConfig[AppVadc_Group_1].queueRequest.triggerConfig.triggerSource = IfxVadc_TriggerSource_15;  //����Դ�����ſ�Դ���ʹ��
	adcGroupConfig[AppVadc_Group_1].queueRequest.requestSlotStartMode = IfxVadc_RequestSlotStartMode_cancelInjectRepeat;//����ע��ģʽ

	adcGroupConfig[AppVadc_Group_1].inputClass[0].sampleTime = 5e-7;//1e-6;
	adcGroupConfig[AppVadc_Group_1].inputClass[1].sampleTime = 5e-7;//1e-6;

	adcGroupConfig[AppVadc_Group_1].arbiter.requestSlotQueueEnabled = TRUE;
	adcGroupConfig[AppVadc_Group_1].arbiter.requestSlotScanEnabled  = TRUE;


	/* enable auto scan */
	adcGroupConfig[AppVadc_Group_1].scanRequest.autoscanEnabled = FALSE;                //single auto scan conversion
	/* enable all gates in "always" mode (no edge detection) */
	adcGroupConfig[AppVadc_Group_1].scanRequest.triggerConfig.gatingMode = IfxVadc_GatingMode_always;

	IfxVadc_Adc_initGroup(&adcGroup[AppVadc_Group_1], &adcGroupConfig[AppVadc_Group_1]);//��ʼ��ADC�ں�(��һ����)

	unsigned int savedGate[AppVadc_GroupCount];
	for(i=0;i<AppVadc_GroupCount;i++)
	{
		savedGate[i] = adcGroup[i].module.vadc->G[adcGroup[i].groupId].QMR0.B.ENGT;
		adcGroup[i].module.vadc->G[adcGroup[i].groupId].QMR0.B.ENGT = 0;
	}

	/* create channel config*/

	//channel 0  group 0
	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[G0_Ch0_9183_VO1], &adcGroup[AppVadc_Group_0]); //��ADCͨ����ӵ���Ӧ������
	adcChannelConfig[G0_Ch0_9183_VO1].channelId = IfxVadc_ChannelId_0;			//p40.0   group0 ch0   9183_VO1
	adcChannelConfig[G0_Ch0_9183_VO1].resultRegister = IfxVadc_ChannelResult_0;

	IfxVadc_Adc_initChannel(&adcChannel[G0_Ch0_9183_VO1], &adcChannelConfig[G0_Ch0_9183_VO1]);
	IfxVadc_Adc_addToQueue(&adcChannel[G0_Ch0_9183_VO1], IFXVADC_QUEUE_REFILL|IFXVADC_QUEUE_EXT_TRIGGER);//��ӽ������У������ô�ͨ��Ϊ�Ĵ�����ʽΪ�ⲿ������PWM��
//	IfxVadc_Adc_addToQueue(&adcChannel[G0_Ch0_9183_VO1], IFXVADC_QUEUE_REFILL);

	//channel 3 group 0
	/*  AppVadc_Group_bridge���������һ��ͨ��   */
	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[G0_Ch3_9183_VO3], &adcGroup[AppVadc_Group_0]);
	adcChannelConfig[G0_Ch3_9183_VO3].channelId = IfxVadc_ChannelId_3;		    //p40.3   group0 ch3	9183_VO3
	adcChannelConfig[G0_Ch3_9183_VO3].resultRegister = IfxVadc_ChannelResult_3;

	IfxVadc_Adc_initChannel(&adcChannel[G0_Ch3_9183_VO3], &adcChannelConfig[G0_Ch3_9183_VO3]);	//���ͨ��
	IfxVadc_Adc_addToQueue(&adcChannel[G0_Ch3_9183_VO3], IFXVADC_QUEUE_REFILL);

	//channel 8 group 0
	/*  AppVadc_Group_bridge���������һ��ͨ��   */
	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[G0_ch8_pressure], &adcGroup[AppVadc_Group_0]);
	adcChannelConfig[G0_ch8_pressure].channelId = IfxVadc_ChannelId_8;		    //p40.8   group0 ch8	pressure
	adcChannelConfig[G0_ch8_pressure].resultRegister = IfxVadc_ChannelResult_8;

	IfxVadc_Adc_initChannel(&adcChannel[G0_ch8_pressure], &adcChannelConfig[G0_ch8_pressure]);	//���ͨ��
	IfxVadc_Adc_addToQueue(&adcChannel[G0_ch8_pressure], IFXVADC_QUEUE_REFILL);

	/*******************************************/

	//channel 0 group 1
	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[G1_Ch0_9183_VO2], &adcGroup[AppVadc_Group_1]);//��ADCͨ����ӵ���Ӧ������(��һ����)
	adcChannelConfig[G1_Ch0_9183_VO2].channelId = IfxVadc_ChannelId_0;		    //p41.0   group1 ch0	9183_VO2
	adcChannelConfig[G1_Ch0_9183_VO2].resultRegister = IfxVadc_ChannelResult_0;

	adcChannelConfig[G1_Ch0_9183_VO2].resultPriority = ISR_PRIORITY_VADC_QUEUE;	//Interrupt EN  �����ж�

	IfxVadc_Adc_initChannel(&adcChannel[G1_Ch0_9183_VO2], &adcChannelConfig[G1_Ch0_9183_VO2]);
	IfxVadc_Adc_addToQueue(&adcChannel[G1_Ch0_9183_VO2], IFXVADC_QUEUE_REFILL|IFXVADC_QUEUE_EXT_TRIGGER);
//	IfxVadc_Adc_addToQueue(&adcChannel[G1_Ch0_9183_VO2], IFXVADC_QUEUE_REFILL);

	//channel 3 group 1
	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[G1_Ch3_9183_VRO], &adcGroup[AppVadc_Group_1]); //��ADCͨ����ӵ���Ӧ������(��һ����)
	adcChannelConfig[G1_Ch3_9183_VRO].channelId      = IfxVadc_ChannelId_3;		//p41.3  group1 ch3	9183_VRO
	adcChannelConfig[G1_Ch3_9183_VRO].resultRegister = IfxVadc_ChannelResult_3;

	IfxVadc_Adc_initChannel(&adcChannel[G1_Ch3_9183_VRO], &adcChannelConfig[G1_Ch3_9183_VRO]);
	IfxVadc_Adc_addToQueue(&adcChannel[G1_Ch3_9183_VRO], IFXVADC_QUEUE_REFILL);
	
	//channel 4 group 1
	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[G1_Ch4_V_INV], &adcGroup[AppVadc_Group_1]); //��ADCͨ����ӵ���Ӧ������(��һ����)
	adcChannelConfig[G1_Ch4_V_INV].channelId      = IfxVadc_ChannelId_4;	//p41.4   group1 ch4	V_INV
	adcChannelConfig[G1_Ch4_V_INV].resultRegister = IfxVadc_ChannelResult_4;

	IfxVadc_Adc_initChannel(&adcChannel[G1_Ch4_V_INV], &adcChannelConfig[G1_Ch4_V_INV]);
	IfxVadc_Adc_addToQueue(&adcChannel[G1_Ch4_V_INV], IFXVADC_QUEUE_REFILL);


	//channel 5 group 1 (KL15)
	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[G1_Ch5_KL15], &adcGroup[AppVadc_Group_1]);
	adcChannelConfig[G1_Ch5_KL15].channelId      = IfxVadc_ChannelId_5;	//p41.5   group1 ch5 KL15
	adcChannelConfig[G1_Ch5_KL15].resultRegister = IfxVadc_ChannelResult_5;

	IfxVadc_Adc_initChannel(&adcChannel[G1_Ch5_KL15], &adcChannelConfig[G1_Ch5_KL15]);
	IfxVadc_Adc_addToQueue(&adcChannel[G1_Ch5_KL15], IFXVADC_QUEUE_REFILL);

	for(i=0;i<AppVadc_GroupCount;i++)
	{
		adcGroup[i].module.vadc->G[adcGroup[i].groupId].QMR0.B.ENGT = savedGate[i];
	}
}














