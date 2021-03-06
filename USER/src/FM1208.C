 
#include "FM1208.H"
#include "des.h"
#include <string.h>
#include "bsp_serial.h"
//extern unsigned char Pcb;					//CPU卡APDU指令分组号
  unsigned char  DataBuf[64];
  unsigned char  g_cReceBuf[64]; 



/////自定义持卡人基本信息内容////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char static CardTpye=0x00;	//卡类型标识：00  （这只是一个代码，比如00表示中国银行，01表示工商银行等等）
unsigned char static StaffType=0x00;  //本行职工标识	这只是一个代码，例如00表示贵宾用户，01表示普通用户等等

char name[32] = {"苏胖子\0"};	//持卡人姓名
unsigned char static DocumentNum[]={0x31 ,0x31 ,0x30 ,0x31 ,0x30 ,0x32 ,0x39 ,0x38 ,0x31 ,0x32 ,0x31 ,0x38 ,0x30 ,0x30 ,0x31 ,0x30};//持卡人的证件号码	  
unsigned char static DocumentTpye=0x05;//持卡人的证件类型	比如05表示身份证，06表示护照等等  ,05是不是代表身份证，不知道,要去问问银行的人,身份证件的代号是什么

#define	 NameLen     strlen (name)	 //计算姓名的长度

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//-------------------以下为所要用到指令的DATA数据，建在MF目录下的所有文件及密钥-----------------------------------------------------//
//
//为统一方便管理阅读代码，我们在所有的文件名后面加个_XXXX,表示就是文件的标识号，//														
//例如： FM1208_MF_Key_0000  :表示FM1208 CPU卡 的MF 下 KEY文件（即密钥文件）标识号为0000
//


//建密钥文件
unsigned char static FM1208_MF_Key_0000[] ={0x3F, 0x00, 0x50, 0x01, 0xF0, 0xFF, 0xFF};	

//1、39装载16字节的外部认证密钥//	
unsigned char static  FM1208_MF_ExKey[] ={0x39,0xF0,0xF0,0xAA,0x88,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	
									
//2、36装载8字节的线路保护密钥	   
unsigned char static  FM1208_MF_LineCKey[]={0x36,0xF0,0xF0,0xFF,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//建定长记录文件：定长记录文件内容为“文件控制信息（file control information） 
unsigned char static  FM1208_MF_RECORD[]={0x2A,0x02,0x13,0xF0,0x00,0xFF,0xFF};

//装载 记录文件的目录名 及PBOC 的 ASC 码的内容
unsigned char static  FM1208_MF_RECORD_DATA[]={0x61,0x11,0x4F,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x50,0x04,0x50,0x42,0x4F,0x43};





//-------------------以下为所要用到指令的DATA数据，建在DF目录下的所有文件及密钥-----------------------------------------------------//
//
//为统一方便管理阅读代码，我们在所有的文件名后面加个_XXXX,表示就是文件的标识号，//
//例如： FM1208_DF_3F01  :表示FM1208 CPU卡 的DF 下	标识号为3F01
//


//创建DF文件,标识名为3F01//
unsigned char static  FM1208_DF_3F01[] ={0x38,0x03,0x6F,0xF0,0xF0,0x95,0xFF,0xFF,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01};

//建立DF下的密钥文件
unsigned char static FM1208_DF_Key_0000[] ={0x3F,0x01,0x8F,0x95,0xF0,0xFF,0xFF};	//建密钥文件

//1、装载TAC密钥
unsigned char static  FM1208_DF_TACKey[] ={0x34,0xF0,0x02,0x00,0x01,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34};

//2、线路保护密钥，MAC计算用到的   （由SAM卡的MAC密钥一级分散得出）
unsigned char static  FM1208_DF_LineCKey[] ={0x36,0xF0,0x02,0xFF,0x33,0x7C,0x51,0xF0,0x79,0x10,0xD8,0x03,0x8D,0x20,0x97,0xE5,0x8F,0xDA,0x79,0x3F,0x1E};

//3、外部认证密钥  （由SAM卡分散所得）
unsigned char static  FM1208_DF_ExKey[] ={0x39,0xF0,0x02,0x44,0xFF,0x5F,0xD6,0x76,0x0C,0xDF,0x6D,0xD3,0x93,0xFF,0xDD,0x6C,0x5C,0xCB,0x65,0x85,0xBF};


//4、消费密钥  用PSAM卡的消费密钥种子分散所取得
unsigned char static  FM1208_DF_DPKey[] ={0x3E,0xF0,0x02,0x00,0x01,0x3E,0x5E,0x67,0xF2,0xFF,0x23,0x09,0x29,0x9F,0x84,0x20,0xF4,0x3D,0xE1,0x39,0x2B};

//5、圈存密钥    (用PSAM卡的MAC、加密密钥，然后用户卡序列号作为因子分散出来，作为圈存密钥，这样就每张用户卡的密钥不同)
unsigned char static  FM1208_DF_CZKey[] ={0x3F,0xF0,0x02,0x00,0x01,0x4A,0x17,0xCA,0x82,0x98,0xD5,0x28,0x65,0x14,0x0A,0xBE,0xAA,0x10,0x8A,0x4E,0x00 }; 
	  
//6、口令PIN密钥	  
unsigned char static  FM1208_DF_PIN[]={0x3A,0xF0,0xEF,0x01,0x33,0x12,0x34,0x56,0xFF,0xFF,0xFF,0xFF,0xFF}; 

//7、内部密钥	  （由SAM卡分散所得）
unsigned char static  FM1208_DF_INKey[]={0x30,0xF0,0xF0,0xFF,0x33,0x1E,0xE2,0xFC,0x1F,0x6C,0x47,0xD2,0x55,0xBF,0x4F,0xE9,0x41,0xF6,0x02,0xFB,0x29};




//建立0015 公共应用基本数据文件	 是带线路保护的二进制文件  
unsigned char static  FM1208_DF_0015[] ={0xA8,0x00,0x1E,0xF0,0xF0,0xFF,0xFF};

//公共应用基本数据文件内容  注意: 这里有一个用户卡序列号：00062016102000000001，取后8个字节，我们给每张用户卡发卡时设置为不同
unsigned char static  FM1208_DF_0015_DATA[] ={0x11,0x11,0x22,0x22,0x33,0x33,0x00,0x06,0x03,0x01,0x00,0x06,0x20,0x16,0x10,0x20,0x00,0x00,0x00,0x01,0x20,0x16,0x10,0x20,0x20,0x19,0x12,0x31,0x55,0x66};//


//建立0016 ED/EP持卡人基本信息数据的文件，如，姓名、证件等信息 是带线路保护的二进制文件   
unsigned char static  FM1208_DF_0016[] ={0xA8,0x00,0x27,0xF0,0xF0,0xFF,0xFF};

											
//建0018 ED/EP应用的交易明细循环记录文件： 我们只要把这个文件建好即可，当我们交易成功后COS系统会自动写入，无需我们自己写入的。
unsigned char static  FM1208_DF_0018[] ={0x2E,0x0A,0x17,0xF1,0xEF,0xFF,0xFF};

//建0001 ED电子存折文件
unsigned char static  FM1208_DF_0001[] ={0x2F,0x02,0x08,0xF1,0x00,0xFF,0x18};

//建0002 EP电子钱包文件		
unsigned char static  FM1208_DF_0002[] ={0x2F,0x02,0x08,0xF0,0x00,0xFF,0x18};



//////////////////文件标识的定义/////////////////////////////////////////////////////////////////////////////////
//
//为统一方便管理阅读代码，我们在所有的文件名后面加个_Flag,表示就是该对应文件的标识//

//unsigned char static MF_File_3F00[2]  = {0x3f,0x00};      //MF文件标识3F00
unsigned char static KEY_File_0000[2] = {0x00,0x00};      //MF下的密钥文件标识0000
unsigned char static ExKey_Flag       = {0x00};           //MF下的外部密钥标识00
unsigned char static LineKey_Flag     = {0x01}; 		    //MF下的线路保护密钥标识01
unsigned char static RECORD_File[2]   = {0x00,0x01}; 		//MF下的定长记录文件标识0001
unsigned char static RECORD_Flag      = {0x08}; 		    //MF下的定长记录标识08

unsigned char static FM1208_DF_3F01_Flag[] ={0x3f,0x01};	//DF文件标识3F01
unsigned char static KEY_File_0000_Flag[2] ={0x00,0x00};  //DF下的密钥文件标识0000				   
unsigned char static FM1208_DF_TACKey_Flag={0x00};		//DF下的TAC密钥标识00
unsigned char static FM1208_DF_LineCKey_Flag={0x00};		//DF下的线路保护密钥标识00
unsigned char static FM1208_DF_ExKey_Flag  = {0x00};      //DF下的外部密钥标识00
unsigned char static FM1208_DF_Inital_Key_Flag  = {0x00}; //DF下的内部密钥标识00



unsigned char static FM1208_DF_DPKey_Flag={0x00};			//DF下的消费密钥标识00
unsigned char static FM1208_DF_CZKey_Flag={0x00};			//DF下的圈存密钥标识00
unsigned char static FM1208_DF_PIN_Flag={0x00};			//DF下的口令PIN密钥标识00
unsigned char static FM1208_DF_0015_Flag[]={0x00,0x15};	//公共应用基本数据文件标识0015
unsigned char static FM1208_DF_0016_Flag[]={0x00,0x16};	//持卡人基本信息数据的文件标识0016
unsigned char static FM1208_DF_0018_Flag[]={0x00,0x18};   //交易明细文件标识0018
unsigned char static FM1208_DF_0001_Flag[]={0x00,0x01};	//电子存折文件标识0001
unsigned char static FM1208_DF_0002_Flag[]={0x00,0x02};	//电子钱包文件标识0002


unsigned char  Set_CpuCard(void)
{
	 char st;
	 unsigned int  sta;
	 unsigned char tt[2];
	 unsigned char  Challenge[8]={0,0,0,0,0,0,0,0};
	 unsigned char  stdata[3];



	    st=SendUartStatus(FM1208_CreateFile(KEY_File_0000, sizeof(FM1208_MF_Key_0000), FM1208_MF_Key_0000 )); //建密钥文件
		if(st)	
		return 1;		//错误返回	

		UART_Send_Str("标识为0000的密钥文件建立成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);

		st=SendUartStatus(FM1208_Load_Key(ExKey_Flag, sizeof(FM1208_MF_ExKey), FM1208_MF_ExKey));	//装载外部认证密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("外部认证密钥装载成功,标识为:");
		UART_Put_Num(&ExKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;

							

 		st=SendUartStatus(FM1208_Load_Key(LineKey_Flag, sizeof(FM1208_MF_LineCKey), FM1208_MF_LineCKey));	//装载线路保护密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("线路保护密钥装载成功,标识为:");
		UART_Put_Num(&LineKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;



 		st=SendUartStatus(FM1208_CreateFile(RECORD_File, sizeof(FM1208_MF_RECORD), FM1208_MF_RECORD));	//建立定长记录文件
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("建立定长记录文件成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(AppendRecord(RECORD_Flag, sizeof(FM1208_MF_RECORD_DATA), FM1208_MF_RECORD_DATA));	 //写定长记录文件内容
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("写定长记录文件内容成功!");
		UART_Send_Enter();
	  	UART_Send_Enter();
		//delay(5);;

	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_3F01_Flag, sizeof(FM1208_DF_3F01), FM1208_DF_3F01 )); //建DF文件
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("建DF文件3F01成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(FM1208_SelectFile(FM1208_DF_3F01_Flag,DataBuf,tt)); //打开CPU卡3F01文件//
		if(st)	
		return 1;		//错误返回
		
		UART_Send_Str("3F01文件打开成功:"); 
		UART_Put_Num(&DataBuf[2],tt[0]);	 //串口助手显示返回的数�
	    UART_Send_Enter();
		//delay(5);;
		

	    st=SendUartStatus(FM1208_CreateFile(KEY_File_0000_Flag, sizeof(FM1208_DF_Key_0000), FM1208_DF_Key_0000 )); //建DF下的密钥文件
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下标识为0000的密钥文件建立成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_TACKey_Flag, sizeof(FM1208_DF_TACKey), FM1208_DF_TACKey));	//装载TAC密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("TAC密钥装载成功,标识为:");
		UART_Put_Num(&FM1208_DF_TACKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;
	

		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_LineCKey_Flag, sizeof(FM1208_DF_LineCKey), FM1208_DF_LineCKey));	//装载线路保护密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下的线路保护密钥装载成功,标识为");
		UART_Put_Num(&FM1208_DF_LineCKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_ExKey_Flag, sizeof(FM1208_DF_ExKey), FM1208_DF_ExKey));	//装载外部认证密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下的外部认证密钥装载成功,标识为:");
		UART_Put_Num(&FM1208_DF_ExKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_DPKey_Flag, sizeof(FM1208_DF_DPKey), FM1208_DF_DPKey));	//装载消费密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下的消费密钥装载成功,标识为:");
		UART_Put_Num(&FM1208_DF_DPKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_CZKey_Flag, sizeof(FM1208_DF_CZKey), FM1208_DF_CZKey));	//装载圈存密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下的圈存密钥装载成功,标识为:");
		UART_Put_Num(&FM1208_DF_CZKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_PIN_Flag, sizeof(FM1208_DF_PIN), FM1208_DF_PIN));	//装载口令PIN密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下的口令PIN钥装载成功,标识为:");
		UART_Put_Num(&FM1208_DF_PIN_Flag,1);
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_Inital_Key_Flag, sizeof(FM1208_DF_INKey), FM1208_DF_INKey));	//装载内部密钥
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下的内部钥装载成功,标识为:");
		UART_Put_Num(&FM1208_DF_Inital_Key_Flag,1);
		UART_Send_Enter();
		//delay(5);;





		sta=FM1208_CreateFile(FM1208_DF_0015_Flag, sizeof(FM1208_DF_0015), FM1208_DF_0015 );  //建DF下的0015文件
		if(sta!=0x9000)	//当返回为01时，表示超时
		{
			 tt[0]=sta>>8;
		     if(tt[0]==0x01)	  //返回01时，需要等待时，进入
			 {
				while(1)
				{
					WTX_Cmd(stdata,tt);
					if(stdata[2]!=0x01)
					{
						sta=0x9000;
						break;
					}
					else
					{
						__nop();
						//delay_ms(5);
					}
				}
			 }
		}

		st=SendUartStatus(sta);
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下标识为0015文件建立成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		FM1208_GetChallenge(Challenge);	 //取随机数	
		MAC1(FM1208_DF_0015_Flag[1], sizeof(FM1208_DF_0015_DATA), FM1208_DF_0015_DATA, &FM1208_DF_LineCKey[5],Challenge,DataBuf,tt);
		st=SendUartStatus(FM1208_Update_Binary(FM1208_DF_0015_Flag[1],DataBuf, tt[0] ));    //执行把加线路保护的数据写入0015中。
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("0015公共应用基本内容写入成功!");
		UART_Send_Enter();
	  	UART_Send_Enter();
		//delay(5);;


	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0016_Flag, sizeof(FM1208_DF_0016), FM1208_DF_0016 )); //建DF下的0016文件
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("DF下标识为0016文件建立成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		FM1208_GetChallenge(Challenge);	 				 //取随机数		
		DataBuf[0]=CardTpye;							 //卡类型标识
		DataBuf[1]=StaffType;							 //本行职工标识
		memcpy(&DataBuf[2], name, NameLen+1);			 //持卡人姓名
		memset(&DataBuf[NameLen+2], 0x00, 20-NameLen);	 //姓名后面补0
		memcpy(&DataBuf[22], DocumentNum, 16);			 //持卡人的证件号码
		DataBuf[38]=DocumentTpye;						 //持卡人的证件类型


		MAC1(FM1208_DF_0016_Flag[1], 0x27, DataBuf, &FM1208_DF_LineCKey[5],Challenge,g_cReceBuf,tt);
		st=SendUartStatus(FM1208_Update_Binary(FM1208_DF_0016_Flag[1],g_cReceBuf, tt[0] ));    //执行把加线路保护的数据写入0016中。
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("0016用户卡基本信息写入成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;	



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0018_Flag, sizeof(FM1208_DF_0018), FM1208_DF_0018 )); //建DF下的0018文件
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("建DF下的0018循环交易记录文件成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0001_Flag, sizeof(FM1208_DF_0001), FM1208_DF_0001 )); //建DF下的电子存折文件ED
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("建DF下的电子存折ED文件成功!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0002_Flag, sizeof(FM1208_DF_0002), FM1208_DF_0002 )); //建DF下的电子钱包文件EP
		if(st)	
		return 1;		//错误返回
		UART_Send_Str("建DF下的电子钱包EP文件成功!");
		UART_Send_Enter();
		UART_Send_Enter();


		return 0;


}


//*************************************************************************
// 函数名	：PcdSwitchPCB(void)
// 描述		：切换分组号
// 入口		：无
// 出口		：无
// 返回		：无
//*************************************************************************

//void PcdSwitchPCB(void)
//{
//	switch(Pcb)
//	{
//		case 0x00:
//			Pcb=0x0A;
//			break;
//		case 0x0A:
//			Pcb=0x0B;
//			break;
//		case 0x0B:
//			Pcb=0x0A;
//			break;
//	}
//}

//*************************************************************************
// 函数名	：pro_reset
// 描述		：转入APDU命令格式CPU卡复位操作
// 入口		： 无
// 出口		： DataOut 输出的数据，	rlen 输出数据的长度
// 返回		：成功返回 _OK = 0
//*************************************************************************

char pro_reset(unsigned char *DataOut, unsigned char *rlen)
{

    char status;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

	memset(MfComData.MfData, 0x00, 64);  //清0

	 
	//PcdSetTmo(4);


  //  MfComData.MfCommand = PCD_TRANSCEIVE;		 //发送并接收数据的命令
    MfComData.MfLength  = 2;
    MfComData.MfData[0] = 0xE0;
    MfComData.MfData[1] = 0x60;
      

//    status = PcdComTransceive(pi);
    if (status == 0)
    {

	   memcpy(DataOut, &MfComData.MfData[0], MfComData.MfLength);
	   *rlen=MfComData.MfLength/8;
		
       return   0;
    }

	 return  status;
	 
}

//*************************************************************************
// 广州恒天淘宝店技术总工编写
// 函数名	：FM1208_SelectFile
// 描述		：选择文件
// 入口		：pDataIn : 文件标识符或者DF 名称
// 出口		：pDataOut 输出数据   Out_Len输出的数据长度
// 返回		：执行成功sst=9000
//*************************************************************************
 
unsigned int FM1208_SelectFile(unsigned char* pDataIn, unsigned char* pDataOut,unsigned char * Out_Len)
{
  
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

//    MfComData.MfCommand = PCD_TRANSCEIVE;			  //发送并接收数据的命令
	memset(MfComData.MfData, 0x00, 64);  //清0
	 
	//PcdSetTmo(13);


//	PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x00;				// CLA
	MfComData.MfData[3] = 0xA4;				// INS-Select File 			
	MfComData.MfData[4] = 0x00;				// P1						 
	MfComData.MfData[5] = 0x00;				// P2					
	MfComData.MfData[6] = 0x02;				// LC	
 	MfComData.MfData[7] = pDataIn[0];		// File					
	MfComData.MfData[8] = pDataIn[1];		// File	
   
	MfComData.MfLength  = 9;	   //为MfComData.MfData的所有数据长度为9个字节


//    status = PcdComTransceive(pi);					   //写指令
    if (status == 0)
    {

		 memcpy(pDataOut, MfComData.MfData, MfComData.MfLength);	//DATA	


	    *Out_Len=MfComData.MfLength/8;	

		st[0] = pDataOut[*Out_Len-2];   //执行的返回状态
		st[1] = pDataOut[*Out_Len-1];

		sst=st[0];
		sst=(sst<<8)|st[1];

		*Out_Len=MfComData.MfLength/8-2;//减去前面的两个字节	


		return sst;

    }
	else  
	{	 
		return 0xFF;
	}
	  
}

//*************************************************************************
// 广州恒天淘宝店技术总工编写
// 函数名	：WTX_Cmd
// 描述		：此函数主要作用是，当对CPU卡建文件或删除文件时，需要提供等待时间，
//            当执行CPU卡指令返回01时，我们则把01发送回给CPU卡，直到等到不再返回01则执行成功
// 入口		：pDataIn : 要执行的指令					
// 出口		：pDataOut：输出执行后的返回数据   Out_Len输出的数据长度
// 返回		：0
//*************************************************************************

char  WTX_Cmd(unsigned char* pDataOut,unsigned char * Out_Len)
{
  
    char status;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

 
	memset(MfComData.MfData, 0x00, 64);  //清0

	 
//	PcdSetTmo(13);

//    MfComData.MfCommand = PCD_TRANSCEIVE;			//发送并接收数据的命令

    MfComData.MfData[0] = 0xFA;
    MfComData.MfData[1] = 0x00;
	MfComData.MfData[2] = 0X01;				
		
	MfComData.MfLength  = 3;	               //为MfComData.MfData的所有数据长度

									

//    status = PcdComTransceive(pi);					   //写指令

    if (status == 0)
    {
 	   
		memcpy(pDataOut, MfComData.MfData, MfComData.MfLength);	//DATA	

	    *Out_Len=MfComData.MfLength/8-2;		//减去前面的两个字节
        return   0;
    }
	else
	{
		return 0xFF;
	}
	
 }

 //void FM_GET_CHALLENGE(uint8_t togetsize,uint8_t*challenge)
//{
//	uint16_t i = 0;
//	    gAPDU_Send.Header.CLA = 0;
//		gAPDU_Send.Header.INS = 0x84;
//		gAPDU_Send.Header.P1 = 0;
//		gAPDU_Send.Header.P2 = 0;
//		gAPDU_Send.Body.LC = togetsize;
//		if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
//		{
//			//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
//			if(gAPDU_rxLen >= 2)
//			{
//				gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
//				gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
//			}
//			if( gAPDU_repbuf.SW1 == 0x67)
//			{
//				//长度错误
//			}
//			if((gAPDU_repbuf.SW1 == 0x6a)&&(gAPDU_repbuf.SW2 == 0x81))
//			{
//				//不支持此功能（无MF 或卡片已锁定）
//			}
//			printf("<<----\r\n");
//			for(i= 0; i < gAPDU_rxLen;i++)
//				printf("%02x ", gAPDU_repbuf.Data[i]);
//			printf("\r\n");
//		}
// 

//}
//*************************************************************************
// 广州恒天淘宝店技术总工编写
// 函数名	：FM1208_GetChallenge
// 描述		：读取随机数
// 入口		：无			  		
// 出口		： pRan: 4个字节的随机数
// 返回		：执行成功sst=0X9000
//*************************************************************************
unsigned int FM1208_GetChallenge(unsigned char* pRan)
{

    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
 
	uint16_t i = 0;
	gAPDU_Send.Header.CLA 	= 0;
	gAPDU_Send.Header.INS 	= 0x84;
	gAPDU_Send.Header.P1	= 0;
	gAPDU_Send.Header.P2 	= 0;
	gAPDU_Send.Body.LC 		= 	0x04;
	if(status = CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
		if( gAPDU_repbuf.SW1 == 0x67)
		{
			//长度错误
		}
		if((gAPDU_repbuf.SW1 == 0x6a)&&(gAPDU_repbuf.SW2 == 0x81))
		{
			//不支持此功能（无MF 或卡片已锁定）
		}
//		printf("<<----\r\n");
//		for(i= 0; i < gAPDU_rxLen;i++)
//			printf("%02x ", gAPDU_repbuf.Data[i]);
//		printf("\r\n");
	}
	if (status == 0)
	{
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW1; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			memcpy(pRan, gAPDU_repbuf.Data, 4);
		}
				
		return sst;
	}
	else
		return 0xFF;
	}	
	 

}

//*************************************************************************
// 广州恒天淘宝店技术总工编写
// 函数名	：FM1208_CreateFile
// 描述		：创建文件
// 入口		：fileid :创建的文件标识
//            Len ：报文数据的长度       
//			  pDataIn：创建文件报文的数据
//
// 出口		：无
// 返回		：执行成功sst=0X9000
//*************************************************************************
 
unsigned int  FM1208_CreateFile(unsigned char *fileid, unsigned char Len, unsigned char *pDataIn )
{
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

//    MfComData.MfCommand = PCD_TRANSCEIVE;			  //发送并接收数据的命令
	memset(MfComData.MfData, 0x00, 64);  //清0
	    //CRC校验
//	PcdSetTmo(13);


	//PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x80;				// CLA
	MfComData.MfData[3] = 0xE0;			    //INS-Create File 			
	MfComData.MfData[4] = fileid[0];		//P1						 
	MfComData.MfData[5] = fileid[1];		//P2					
	MfComData.MfData[6] = Len;				//Lc
			
	MfComData.MfLength  = MfComData.MfData[6]+7;	   //为MfComData.MfData的所有数据长度										

	memcpy(&MfComData.MfData[7], pDataIn, Len);			//DATA
	   

//    status = PcdComTransceive(pi);					   //写指令
 	if (status == 0)							   //指令执行成功
	{
		st[0] = MfComData.MfData[2];
		st[1] = MfComData.MfData[3]; 

		sst=st[0];
		sst=(sst<<8)|st[1];

		return sst;									   //指令执行成功后，返回CPU卡的执行结果是否成功返回9000
	}
	else						//指令执行失败

		return 0xFF;
}

//*************************************************************************
// 广州恒天淘宝店技术总工编写
// 函数名	：FM1208_Load_Key
// 描述		：装载密钥
// 入口		：Keysign:密钥标识
//			  Len ：报文数据的长度 		
//			  pDataIn：装载密钥文件报文的数据
//					
//					
// 出口		：无
// 返回		：执行成功sst=0X9000
//*************************************************************************

unsigned int FM1208_Load_Key(unsigned char Keysign, unsigned char Len, unsigned char* pDataIn)
{
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

   // MfComData.MfCommand = PCD_TRANSCEIVE;			  //发送并接收数据的命令
	memset(MfComData.MfData, 0x00, 64);  //清0
	 	   //CRC校验
	//PcdSetTmo(13);




	//PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x80;				// CLA
	MfComData.MfData[3] = 0xD4;			    //INS-Write Key 			
	MfComData.MfData[4] = 0x01;				//P1						 
	MfComData.MfData[5] = Keysign;			//p2-key flag									
	MfComData.MfData[6] = Len;				//Lc		
	MfComData.MfLength  = MfComData.MfData[6]+7;	   //为MfComData.MfData的所有数据长度										


	memcpy(&MfComData.MfData[7], pDataIn, Len);			//DATA	 
    

	  

//    status = PcdComTransceive(pi);					   //写指令
	if (status == 0)
	{
		st[0] = MfComData.MfData[2];	 //0X90
		st[1] = MfComData.MfData[3]; 	 //0X00

		sst=st[0];
		sst=(sst<<8)|st[1];

		return sst;
	}
	else
		return 0xFF;

}


//*************************************************************************
// 广州恒天淘宝店技术总工编写
// 函数名	：AppendRecord
// 描述		：写记录文件
// 入口		：fileid 记录文件的短文标识
//			  Len ：报文数据的长度 	
//			  pDataIn：写入记录文件的报文数据
//				
//				
// 出口		：无
// 返回		：执行成功sst=0X9000
//*************************************************************************

unsigned int AppendRecord(unsigned char fileid, unsigned char Len, unsigned char* pDataIn)
{
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

  //  MfComData.MfCommand = PCD_TRANSCEIVE;			  //发送并接收数据的命令
	memset(MfComData.MfData, 0x00, 64);  //清0
	 
//	PcdSetTmo(13);




//	PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x00;				// CLA
	MfComData.MfData[3] = 0xE2;			    //INS 			
	MfComData.MfData[4] = 0x00;				//P1						 
	MfComData.MfData[5] = fileid;			//p2 									
	MfComData.MfData[6] = Len;				//Lc		
	MfComData.MfLength  = MfComData.MfData[6]+7;	   //为MfComData.MfData的所有数据长度										


	memcpy(&MfComData.MfData[7], pDataIn, Len);			//DATA	 
    

	  

//    status = PcdComTransceive(pi);					   //写指令
	if (status == 0)
	{
		st[0] = MfComData.MfData[2];	 //0X90
		st[1] = MfComData.MfData[3]; 	 //0X00

		sst=st[0];
		sst=(sst<<8)|st[1];

		return sst;
	}
	else
		return 0xFF;

}

//*************************************************************************
// 广州恒天淘宝店技术总工编写
// 函数名	：FM1208_Update_Binary
// 描述		：写二进制文件，是带线路保护
// 入口		：File_Flag: 二进制文件的短文标识
//            pDataIn :需要写入的二进制数据
//            Len:二进制数据的长度
//		  		
// 出口		：无
// 返回		：执行成功sst=0X9000
//*************************************************************************

unsigned int FM1208_Update_Binary(unsigned char File_Flag,unsigned char *pDataIn, unsigned char Len )
{
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

   // MfComData.MfCommand = PCD_TRANSCEIVE;			  //发送并接收数据的命令
	memset(MfComData.MfData, 0x00, 64);  //清0
	 
//	PcdSetTmo(13);




	//PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x04;				//CLA
	MfComData.MfData[3] = 0xD6;			    //INS 			
	MfComData.MfData[4] = 0x80|File_Flag;	//P1						 
	MfComData.MfData[5] = 0x00;			    //p2 									
	MfComData.MfData[6] = Len;				//Lc		
	MfComData.MfLength  = MfComData.MfData[6]+7;	   //为MfComData.MfData的所有数据长度										

	memcpy(&MfComData.MfData[7], pDataIn, Len);			//DATA	 



//    status = PcdComTransceive(pi);					   //写指令
	if (status == 0)
	{
		st[0] = MfComData.MfData[2];	 //0X90		   
		st[1] = MfComData.MfData[3]; 	 //0X00

		sst=st[0];
		sst=(sst<<8)|st[1];

		return sst;
	}
	else
		return 0xFF;




}
 
//*************************************************************************
// 广州恒天淘宝店技术总工编写
// 函数名	： MAC1
// 描述		： 把4个字节的MAC计算结果加在二进制文件指令数据的尾部
// 入口		： File_Flag 短文件标识  
//		       Len 要写入的数据长度
//		       pDataIn 要写入的数据
//		       LineCKey 线路保护密钥
//             Challenge 随机数
//             
// 出口     ： pDataOut	 已加4个字节的MAC值的指令数据
// 返回		： 无
//*************************************************************************
void MAC1(unsigned char File_Flag, unsigned char Len, unsigned char *pDataIn, unsigned char *LineCKey, unsigned char *Challenge,unsigned char *pDataOut,unsigned char *Le)
{

	unsigned char    MData[64];   
  	unsigned char    fout[4]={0,0,0,0};
 
	MData[0] = 0x04;			
	MData[1] = 0xD6;			    			
	MData[2] = 0x80|File_Flag;							 
	MData[3] = 0x00;			    									
	MData[4] = Len+4;
	 
	memcpy(&MData[5], pDataIn, Len);					  
    PBOC_MAC(&LineCKey[0], Challenge, MData, Len+5,fout);	
	
	memcpy(&MData[Len+5], fout, 4);	     //数据尾部加上4位的MAC
	
								
	Le[0]=MData[4];						 //输出数据的长度
	memcpy(pDataOut, &MData[5],Le[0]);	 //输出的数据

}

