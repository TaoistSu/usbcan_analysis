#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "controlcan.h"

#include <ctime>
#include <cstdlib>
#include "unistd.h"

VCI_BOARD_INFO pInfo;//用来获取设备信息。
int count=0;//数据列表中，用来存储列表序号。
float speed=0.0;
int v_flag = 01;
int v_aim_aim = 00;
int stop_aim = 02;
int chang = 127;

void *receive_func(void* param)  //接收线程。
{
	int reclen=0;
	VCI_CAN_OBJ rec[3000];//接收缓存，设为3000为佳。
	int i,j;
	
	int *run=(int*)param;//线程启动，退出控制。
    int ind=0;

	while((*run)&0x0f)
	{
		if((reclen=VCI_Receive(VCI_USBCAN2,0,ind,rec,3000,100))>0)//调用接收函数，如果有数据，进行数据处理显示。
		{
            j=0;
			{
			    if(rec[j].ID==193)
			    {
                    printf("Index:%04d  ",count);count++;//序号递增
                    printf("CAN%d RX ID:0x%08X", ind+1, rec[j].ID);//ID
                    if(rec[j].ExternFlag==0) printf(" Standard ");//帧格式：标准帧
                    if(rec[j].ExternFlag==1) printf(" Extend   ");//帧格式：扩展帧
                    if(rec[j].RemoteFlag==0) printf(" Data   ");//帧类型：数据帧
                    if(rec[j].RemoteFlag==1) printf(" Remote ");//帧类型：远程帧
                    printf("DLC:0x%02X",rec[j].DataLen);//帧长度
                    printf(" data:0x");	//数据
                    for(i = 0; i < rec[j].DataLen; i++)
                    {
                        printf(" %02X", rec[j].Data[i]);
                    }
                    printf(" TimeStamp:0x%08X",rec[j].TimeStamp);//时间标识。
                    printf("\n");
                    break;
				}
			}
		}
	}
	printf("run thread exit\n");//退出接收线程	
	pthread_exit(0);
}

main()
{
    int i=0;
	printf(">>this is hello !\r\n");//指示程序已运行
	if(VCI_OpenDevice(VCI_USBCAN2,0,0)==1)//打开设备
	{
		printf(">>open deivce success!\n");//打开设备成功
	}else
	{
		printf(">>open deivce error!\n");
		exit(1);
	}
	if(VCI_ReadBoardInfo(VCI_USBCAN2,0,&pInfo)==1)//读取设备序列号、版本等信息。
	{
        printf(">>Get VCI_ReadBoardInfo success!\n");
	}else
	{
		printf(">>Get VCI_ReadBoardInfo error!\n");
		exit(1);
	}

	//初始化参数，严格参数二次开发函数库说明书。
	VCI_INIT_CONFIG config;
	config.AccCode=0;
	config.AccMask=0xFFFFFFFF;//FFFFFFFF全部接收
	config.Filter=2;//接收所有帧  2-只接受标准帧  3-只接受扩展帧
	config.Timing0=0x00;/*波特率500 Kbps  0x00  0x1C*/
	config.Timing1=0x1C;
	config.Mode=0;//正常模式		
	
	if(VCI_InitCAN(VCI_USBCAN2,0,0,&config)!=1)
	{
		printf(">>Init CAN1 error\n");
		VCI_CloseDevice(VCI_USBCAN2,0);
	}

	if(VCI_StartCAN(VCI_USBCAN2,0,0)!=1)
	{
		printf(">>Start CAN1 error\n");
		VCI_CloseDevice(VCI_USBCAN2,0);

	}

	//需要发送的帧，结构体设置
	VCI_CAN_OBJ send[1];
	send[0].ID=529;
	send[0].SendType=0;
	send[0].RemoteFlag=0;
	send[0].ExternFlag=0;
	send[0].DataLen=8;
    //需要读取的帧，结构体设置
	VCI_CAN_OBJ rev[1];
	rev[0].SendType=0;
	rev[0].RemoteFlag=0;
	rev[0].ExternFlag=0;
	rev[0].DataLen=8;



	int m_run0=1;
	pthread_t threadid;
	int ret;
	//ret=pthread_create(&threadid,NULL,receive_func,&m_run0);

	int times = 5;
	while(1)
	{
	    //要写入的数据
	    send[0].Data[0]=v_flag;
        send[0].Data[1]=0x00;
        send[0].Data[2]=0x00;
        send[0].Data[3]=v_aim_aim;
        send[0].Data[4]=stop_aim;
        send[0].Data[5]=0x00;
        send[0].Data[6]=chang;
        send[0].Data[7]=0x00;
	    //写入数据
		if(VCI_Transmit(VCI_USBCAN2, 0, 0, send, 1) == 1)
		{
           // printf("TX data successful!\n");
		}

        //读取数据
        if(VCI_Receive(VCI_USBCAN2, 0, 0,rev, 2500, 0)>0)
        {
            if(rev[0].ID==193)
            {
                printf("-----fuck u------\n");
                printf("CAN2 RX ID:0x%08X", rev[0].ID);
                printf("\n");
                printf("RX data:0x");
                for(i = 0; i < rev[0].DataLen; i++)
                {
                    printf(" %02X", rev[0].Data[i]);
                }
                printf("\n");
                //usleep(100000);//延时100ms。


                if(rev[0].Data[1]>0)//when high bit have data
                {
                    speed = (rev[0].Data[0]+rev[0].Data[1]*256)/3.6/100;
                  //cout << "speed:" << speed <<endl;
                }
                else
                {
                    speed = rev[0].Data[0]/3.6/100;
                }
                printf("speed = %f\n",speed);
            }





        }
    }
	VCI_CloseDevice(VCI_USBCAN2,0);//关闭设备。

	//除收发函数外，其它的函数调用前后，最好加个毫秒级的延时，即不影响程序的运行，又可以让USBCAN设备有充分的时间处理指令。
	//goto ext;
}
