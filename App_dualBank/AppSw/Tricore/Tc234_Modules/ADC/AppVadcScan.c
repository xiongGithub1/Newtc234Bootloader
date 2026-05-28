/******************************************************************************

                  版权所有 (C), 2020-2030, 重庆和天电子科技有限公司

 ******************************************************************************
  文 件 名  : AppVadcScan.c
  版 本 号  : 初稿
  作    者    :
  生成日期 : 2021年10月15日
  最近修改 :
  功能描述 : ADC自动扫描初始化，通道切换，及获取采样结果
  函数列表 :

  修改历史 :
  1.日    期  :
  2.作    者  :
    修改内容   :

******************************************************************************/

/*****************************************************************************/
/*----------------------------------包含头文件--------------------------------*/
/*****************************************************************************/
#include "AppVadcScan.h"
#include "AppVadcQueue.h"



/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
App_VadcChannel g_VadcGroup0Channel;
App_VadcChannel g_VadcGroup1Channel;
Analog_Sample 	AnalogSample;


/***************************************************************
 函数名称： void VadcAutoScanChannel_init(void)
 输入变量：
 输出变量：
 函数注释： ADC自动扫描采集通道初始化
 **************************************************************/
void VadcAutoScanChannel_init(void)
{
#if 0
	//Group0 Channel 0
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig0;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig0, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig0.channelId      = IfxVadc_ChannelId_0;//(IfxVadc_ChannelId)(chnIx);
		adcChannelConfig0.resultRegister = IfxVadc_ChannelResult_8;//(IfxVadc_ChannelResult)(chnIx);  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh0, &adcChannelConfig0);
	}

	//Group0 Channel 1
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig1;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig1, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig1.channelId      = IfxVadc_ChannelId_1;
		adcChannelConfig1.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh1, &adcChannelConfig1);
	}

	//Group0 Channel 2
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig2;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig2, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig2.channelId      = IfxVadc_ChannelId_2;
		adcChannelConfig2.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh2, &adcChannelConfig2);
	}

	//Group0 Channel 3
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig3;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig3, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig3.channelId      = IfxVadc_ChannelId_3;
		adcChannelConfig3.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh3, &adcChannelConfig3);
	}

	//Group0 Channel 4
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig4;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig4, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig4.channelId      = IfxVadc_ChannelId_4;
		adcChannelConfig4.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh4, &adcChannelConfig4);
	}

	//Group0 Channel 5
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig5;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig5, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig5.channelId      = IfxVadc_ChannelId_5;
		adcChannelConfig5.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh5, &adcChannelConfig5);
	}

	//Group0 Channel 6
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig6;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig6, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig6.channelId      = IfxVadc_ChannelId_6;
		adcChannelConfig6.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh6, &adcChannelConfig6);
	}

	//Group0 Channel 7
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig7;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig7, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig7.channelId      = IfxVadc_ChannelId_7;
		adcChannelConfig7.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh7, &adcChannelConfig7);
	}

	//Group0 Channel 8
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig8;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig8, &g_VadcAutoScan.adcGroup0);
		adcChannelConfig8.channelId      = IfxVadc_ChannelId_8;
		adcChannelConfig8.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh8, &adcChannelConfig8);
	}
#endif


	//Group0 Channel 8	3.3V，感觉只是配置通道，没有像普通IO口那样配置，通道数与AD口一一对应，曾军20210908
	// 3.3V采集20240611取消，采集此电压的MCU端口改成了采集主缸压力
//	{
//		IfxVadc_Adc_ChannelConfig adcChannelConfig8;
//
//		IfxVadc_Adc_initChannelConfig(&adcChannelConfig8, &adcGroup[AppVadc_Group_0]);
//		adcChannelConfig8.channelId      = IfxVadc_ChannelId_8;
//		adcChannelConfig8.resultRegister = IfxVadc_ChannelResult_8;   /*use dedicated result register*/
//		/* initialize the channel */
//		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh8, &adcChannelConfig8);
//	}


	//Group0 Channel 9		QT1
//	{
//		IfxVadc_Adc_ChannelConfig adcChannelConfig9;
//
//		IfxVadc_Adc_initChannelConfig(&adcChannelConfig9, &adcGroup[AppVadc_Group_0]);
//		adcChannelConfig9.channelId      = IfxVadc_ChannelId_9;
//		adcChannelConfig9.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
//		/* initialize the channel */
//		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh9, &adcChannelConfig9);
//	}

	//Group0 Channel 10			TEMP1 uc temperature
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig10;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig10, &adcGroup[AppVadc_Group_0]);
		adcChannelConfig10.channelId      = IfxVadc_ChannelId_10;
		adcChannelConfig10.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh10, &adcChannelConfig10);
	}

	//Group0 Channel 11			QT2
//	{
//		IfxVadc_Adc_ChannelConfig adcChannelConfig11;
//
//		IfxVadc_Adc_initChannelConfig(&adcChannelConfig11, &adcGroup[AppVadc_Group_0]);
//		adcChannelConfig11.channelId      = IfxVadc_ChannelId_11;
//		adcChannelConfig11.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
//		/* initialize the channel */
//		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh11, &adcChannelConfig11);
//	}

#if 0
	//Group1 Channel 0
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig12;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig12, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig12.channelId      = IfxVadc_ChannelId_0;
		adcChannelConfig12.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh0, &adcChannelConfig12);
	}

	//Group1 Channel 1
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig13;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig13, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig13.channelId      = IfxVadc_ChannelId_1;
		adcChannelConfig13.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh1, &adcChannelConfig13);
	}

	//Group1 Channel 2		
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig14;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig14, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig14.channelId      = IfxVadc_ChannelId_2;
		adcChannelConfig14.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh2, &adcChannelConfig14);
	}

	//Group1 Channel 3
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig15;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig15, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig15.channelId	  = IfxVadc_ChannelId_3;
		adcChannelConfig15.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh3, &adcChannelConfig15);
	}
	
	//Group1 Channel 4
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig16;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig16, &g_VadcAutoScan.adcGroup1);
		adcChannelConfig16.channelId	  = IfxVadc_ChannelId_4;
		adcChannelConfig16.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh4, &adcChannelConfig16);
	}
#endif

	//Group1 Channel 5			//SS_Battery
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig17;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig17, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig17.channelId      = IfxVadc_ChannelId_5;
		adcChannelConfig17.resultRegister = IfxVadc_ChannelResult_8;   /*use dedicated result register*/
		/* initialize the channel*/
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh5, &adcChannelConfig17);
	}

#if 0
	//Group1 Channel 6
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig18;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig18, &g_VadcAutoScan.adcGroup1);
		adcChannelConfig18.channelId      = IfxVadc_ChannelId_6;
		adcChannelConfig18.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh6, &adcChannelConfig18);
	}

	//Group1 Channel 7
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig19;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig19, &g_VadcAutoScan.adcGroup1);
		adcChannelConfig19.channelId      = IfxVadc_ChannelId_7;
		adcChannelConfig19.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh7, &adcChannelConfig19);
	}

	//Group1 Channel 8
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig20;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig20, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig20.channelId	  = IfxVadc_ChannelId_8;
		adcChannelConfig20.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh8, &adcChannelConfig20);
	}
	
	//Group1 Channel 9
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig21;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig21, &g_VadcAutoScan.adcGroup1);
		adcChannelConfig21.channelId	  = IfxVadc_ChannelId_9;
		adcChannelConfig21.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh9, &adcChannelConfig21);
	}
#endif

	//Group1 Channel 10		////TEMP2 mosfet temperature
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig22;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig22, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig22.channelId      = IfxVadc_ChannelId_10;
		adcChannelConfig22.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh10, &adcChannelConfig22);
	}

#if 0
	//Group1 Channel 11
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig23;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig23, &g_VadcAutoScan.adcGroup1);
		adcChannelConfig23.channelId      = IfxVadc_ChannelId_11;
		adcChannelConfig23.resultRegister = IfxVadc_ChannelResult_8; /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh11, &adcChannelConfig23);
	}
#endif
}

/***************************************************************
 函数名称： void VadcManualScanSetOneChannelToConvert(IfxVadc_Adc_Group *group, uint32 channelNumber)
 输入变量：ADC组，通道号
 输出变量：
 函数注释： 将单一个通道添加到扫描队列里面，并开启扫描
 **************************************************************/
void VadcManualScanSetOneChannelToConvert(IfxVadc_Adc_Group *group, uint32 channelNumber)
{
    unsigned channels = (1 << channelNumber);
    unsigned mask     = 0x00000FFF;

	// set single channel conversion
	//IfxVadc_Adc_setScan(&g_VadcAutoScan.adcGroup0, channels, mask);
	IfxVadc_Adc_setScan(group, channels, mask);


	/* start autoscan */
	//IfxVadc_Adc_startScan(&g_VadcAutoScan.adcGroup0);
	IfxVadc_Adc_startScan(group);

}

/***************************************************************
 函数名称： static void VadcManualScanSetNextChannelToConvert(void)
 输入变量：
 输出变量：
 函数注释： 切换通道
 **************************************************************/
static void VadcManualScanSetNextChannelToConvert(void)
{
	static uint8 g_VadcGroup1ChannelScanIndex = 0;

	switch(g_VadcGroup1ChannelScanIndex)
	{
//	    case 0:
//	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_0],IfxVadc_ChannelId_8);
//	    	break;
//	    case 1:
//	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_0],IfxVadc_ChannelId_9);
//	    	break;
	    case 0:
	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_0],IfxVadc_ChannelId_10);
	    	break;
//	    case 3:
//	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_0],IfxVadc_ChannelId_11);
//	    	break;
	    case 1:
	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_1],IfxVadc_ChannelId_5);
	    	break;
	    case 2:
	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_1],IfxVadc_ChannelId_10);	//TEMP1 uc temperature
	    	break;
	    default:
			break;
	}

	g_VadcGroup1ChannelScanIndex ++;
	if (g_VadcGroup1ChannelScanIndex > (ADScanChannelNum-1))  // Group 6个通道
	{
		g_VadcGroup1ChannelScanIndex = 0;
	}
}

/***************************************************************
 函数名称： void VadcManualScanReadScanChannelResult(void)
 输入变量：
 输出变量：
 函数注释： 获取通道采样结果
 **************************************************************/
void VadcManualScanReadScanChannelResult(void)
{
    Ifx_VADC_RES conversionResult, conversionResult1;

	conversionResult  = VADC_G0RES8;
	conversionResult1 = VADC_G1RES8;

	if(conversionResult.B.VF)
	{
		uint8 CH;
		CH = conversionResult.B.CHNR;

//		if(CH == 8)         // TLF35884输出的MCU等芯片供电3.3V电源
//		{
//			AnalogSample.AN008 = conversionResult.B.RESULT;
//		}
//		else if(CH == 9)    // TLF35884输出的传感器供电5V电源
//		{
//			AnalogSample.AN009 = conversionResult.B.RESULT;
//		}
		if(CH == 10)   // TLF35584周围板上环境温度检测
		{
			AnalogSample.AN010 = conversionResult.B.RESULT;
		}
//		else if(CH == 11)   // TLF35884输出的传感器供电5V电源
//		{
//			AnalogSample.AN011 = conversionResult.B.RESULT;
//		}
	}
	if(conversionResult1.B.VF)
	{
		uint8 CH;
		CH = conversionResult1.B.CHNR;
		if(CH == 5)         // KL15电源电压检测
		{
			AnalogSample.AN105 = conversionResult1.B.RESULT;
		}
		else if(CH == 10)   // MOS管周边板上环境温度检测
		{
			AnalogSample.AN110 = conversionResult1.B.RESULT;
		}
	}
}

/***************************************************************
 函数名称： void ADC_Scan_GetResult(void)
 输入变量：
 输出变量：
 函数注释： 切换AD采样通道，获取通道采样结果
 **************************************************************/
void ADC_Scan_GetResult(void)
{
	VadcManualScanSetNextChannelToConvert();
	VadcManualScanReadScanChannelResult();
}


/***************************************************************
 函数名称： void VadcAutoScanChannel_atSelfTest_init(void)
 输入变量：
 输出变量：
 函数注释： ADC自动扫描采集通道初始化
 **************************************************************/
void VadcAutoScanChannel_atSelfTest_init(void)
{
	uint8 i;

	IfxVadc_Adc_initModuleConfig(&adcConfig, &MODULE_VADC);		                        //初始化ADC模块配置
	// initialize VADC Module
	IfxVadc_Adc_initModule(&vadc, &adcConfig);		                                    //初始化ADC模块，曾军
	/* create group config*/
	IfxVadc_Adc_initGroupConfig(&adcGroupConfig[AppVadc_Group_0], &vadc);		        //初始化组0配置，曾军
	adcGroupConfig[AppVadc_Group_0].groupId = IfxVadc_GroupId_0;			            //指定ADC内核ID（组）
	adcGroupConfig[AppVadc_Group_0].master  = adcGroupConfig[AppVadc_Group_0].groupId;	//设置组ID

	adcGroupConfig[AppVadc_Group_0].inputClass[0].sampleTime = 5e-7;//1e-6;		         //指定采样时间为 0.5US  曾军
	adcGroupConfig[AppVadc_Group_0].inputClass[1].sampleTime = 5e-7;//1e-6;

	adcGroupConfig[AppVadc_Group_0].arbiter.requestSlotScanEnabled  = TRUE;              // 请求扫描

	/* enable auto scan */
	adcGroupConfig[AppVadc_Group_0].scanRequest.autoscanEnabled = TRUE;  //single auto scan conversion
	/* enable all gates in "always" mode (no edge detection) */
	adcGroupConfig[AppVadc_Group_0].scanRequest.triggerConfig.gatingMode = IfxVadc_GatingMode_always;

	IfxVadc_Adc_initGroup(&adcGroup[AppVadc_Group_0], &adcGroupConfig[AppVadc_Group_0]); //初始化ADC内核(组)

	IfxVadc_Adc_initGroupConfig(&adcGroupConfig[AppVadc_Group_1], &vadc);
	adcGroupConfig[AppVadc_Group_1].groupId = IfxVadc_GroupId_1;			             //另一个ADC内核（组）
	adcGroupConfig[AppVadc_Group_1].master  = adcGroupConfig[AppVadc_Group_1].groupId;	 //设置组ID

	adcGroupConfig[AppVadc_Group_1].inputClass[0].sampleTime = 5e-7;//1e-6;
	adcGroupConfig[AppVadc_Group_1].inputClass[1].sampleTime = 5e-7;//1e-6;

	adcGroupConfig[AppVadc_Group_1].arbiter.requestSlotScanEnabled  = TRUE;

	/* enable auto scan */
	adcGroupConfig[AppVadc_Group_1].scanRequest.autoscanEnabled = TRUE;                //single auto scan conversion
	/* enable all gates in "always" mode (no edge detection) */
	adcGroupConfig[AppVadc_Group_1].scanRequest.triggerConfig.gatingMode = IfxVadc_GatingMode_always;

	IfxVadc_Adc_initGroup(&adcGroup[AppVadc_Group_1], &adcGroupConfig[AppVadc_Group_1]);//初始化ADC内核(另一个组)

	unsigned int savedGate[AppVadc_GroupCount];
	for(i=0;i<AppVadc_GroupCount;i++)
	{
		savedGate[i] = adcGroup[i].module.vadc->G[adcGroup[i].groupId].QMR0.B.ENGT;
		adcGroup[i].module.vadc->G[adcGroup[i].groupId].QMR0.B.ENGT = 0;
	}

	//Group0 Channel 0 9183 VO1
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig0;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig0, &adcGroup[AppVadc_Group_0]);
		adcChannelConfig0.channelId      = IfxVadc_ChannelId_0;//(IfxVadc_ChannelId)(chnIx);
		adcChannelConfig0.resultRegister = IfxVadc_ChannelResult_8;//(IfxVadc_ChannelResult)(chnIx);  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh0, &adcChannelConfig0);
	}

	//Group0 Channel 3 9183 VO3
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig3;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig3, &adcGroup[AppVadc_Group_0]);
		adcChannelConfig3.channelId      = IfxVadc_ChannelId_3;
		adcChannelConfig3.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup0Channel.g_VadcCh3, &adcChannelConfig3);
	}


	//Group1 Channel 0 9183 VO2
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig12;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig12, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig12.channelId      = IfxVadc_ChannelId_0;
		adcChannelConfig12.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh0, &adcChannelConfig12);
	}

	//Group1 Channel 3 9183 VR0
	{
		IfxVadc_Adc_ChannelConfig adcChannelConfig15;

		IfxVadc_Adc_initChannelConfig(&adcChannelConfig15, &adcGroup[AppVadc_Group_1]);
		adcChannelConfig15.channelId	  = IfxVadc_ChannelId_3;
		adcChannelConfig15.resultRegister = IfxVadc_ChannelResult_8;  /* use dedicated result register */
		/* initialize the channel */
		IfxVadc_Adc_initChannel(&g_VadcGroup1Channel.g_VadcCh3, &adcChannelConfig15);
	}

	for(i=0;i<AppVadc_GroupCount;i++)
	{
		adcGroup[i].module.vadc->G[adcGroup[i].groupId].QMR0.B.ENGT = savedGate[i];
	}
}


/***************************************************************
 函数名称： void ADC_Scan_GetResult(void)
 输入变量：
 输出变量：
 函数注释： 切换AD采样通道，获取通道采样结果
 **************************************************************/
uint16 TLE9183VO1 = 0;
uint16 TLE9183VO3 = 0;
uint16 TLE9183VO2 = 0;
uint16 TLE9183VRO = 0;
void ADC_Scan_GetResult_atSelfTest(void)
{
	uint8 i;
	for(i=0; i<(ADScanChannelNum_atSelfTest + 1); i++)	// 因为底层是手动转换结果，一次只能读取一个通道的值
	{
		delay_ms(1);
		VadcManualScanSetNextChannelToConvert_atSelfTest();
		VadcManualScanReadScanChannelResult_atSelfTest();
	}

	TLE9183VO1 = AnalogSample.AN000;
	TLE9183VO3 = AnalogSample.AN003;
	TLE9183VO2 = AnalogSample.AN100;
	TLE9183VRO = AnalogSample.AN103;
}


/***************************************************************
 函数名称： void VadcManualScanSetNextChannelToConvert_atSelfTest(void)
 输入变量：
 输出变量：
 函数注释： 切换通道
 **************************************************************/
void VadcManualScanSetNextChannelToConvert_atSelfTest(void)
{
	static uint8 VadcGroupChannelScanIndex = 0;

	switch(VadcGroupChannelScanIndex)
	{
	    case 0:
	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_0],IfxVadc_ChannelId_0);
	    	break;
	    case 1:
			VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_0],IfxVadc_ChannelId_3);
			break;
	    case 2:
	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_1],IfxVadc_ChannelId_0);
	    	break;
	    case 3:
	    	VadcManualScanSetOneChannelToConvert(&adcGroup[AppVadc_Group_1],IfxVadc_ChannelId_3);	//TEMP1 uc temperature
	    	break;
	    default:
			break;
	}

	VadcGroupChannelScanIndex ++;
	if (VadcGroupChannelScanIndex > (ADScanChannelNum_atSelfTest - 1))  // Group 6个通道
	{
		VadcGroupChannelScanIndex = 0;
	}
}



/***************************************************************
 函数名称： void VadcManualScanReadScanChannelResult_atSelfTest(void)
 输入变量：
 输出变量：
 函数注释： 获取通道采样结果
 **************************************************************/
void VadcManualScanReadScanChannelResult_atSelfTest(void)
{
    Ifx_VADC_RES conversionResult, conversionResult1;

	conversionResult  = VADC_G0RES8;
	conversionResult1 = VADC_G1RES8;

	if(conversionResult.B.VF)
	{
		uint8 CH;
		CH = conversionResult.B.CHNR;
		conversionResult.B.VF = 0;

		if(CH == 0)         // 9183 VO1
		{
			AnalogSample.AN000 = conversionResult.B.RESULT;
		}
		else if(CH == 3)    // 9183 VO3
		{
			AnalogSample.AN003 = conversionResult.B.RESULT;
		}
	}
	if(conversionResult1.B.VF)
	{
		uint8 CH;
		CH = conversionResult1.B.CHNR;
		conversionResult1.B.VF = 0;

		if(CH == 0)         // 9183 VO2
		{
			AnalogSample.AN100 = conversionResult1.B.RESULT;
		}
		else if(CH == 3)   // 9183 VRO
		{
			AnalogSample.AN103 = conversionResult1.B.RESULT;
		}
	}
}
