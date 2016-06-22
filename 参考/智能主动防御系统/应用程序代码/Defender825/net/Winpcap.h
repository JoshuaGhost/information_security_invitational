#ifndef _WINPCAP_H_
#define _WINPCAP_H_
#define WPCAP
#define HAVE_REMOTE
#define WM_PRM_NOTIFY WM_USER+1 //自定义托盘消息
#define UPDATE_LIVEHOST_INTERVAL 10000
#define DRIVERNAME "PassThru"
#include<winsock2.h>
#include<Iphlpapi.h>
#include "pcap.h"
#include "collection.h"
#include "IOCommon.h"
//网络适配器信息
#pragma pack(1)  
typedef struct{
	char name[256];          //网卡名称
	UINT ipaddress;          //ip,网络序
	UINT netmask;            //netmask,网络序
	UINT netgate;            //netgate,网络序
	BYTE netgatemac[6];      //netgate mac
	BYTE mac[6];             //mac,网络序
	pcap_t * fp;             //pcap_open返回值
	UINT ipnow;              //当前扫描ip,主机序
	BOOL scanliveover;       //扫描存活主机是否完成
	BOOL hostlistok;         //存活主机列表是否准备好
	Collection livehostlist;  //存活主机列表,网络序
	UINT ipstart;            //网段起始ip,主机序
	UINT ipend;              //网段结束ip,主机序
}Adapter;
typedef  struct
{
	unsigned int ip;
	unsigned char mac[6];
}HostInfor;
typedef struct
{
	unsigned char code[32];
}CodeNode;
#pragma pack()
#endif
//计算校验和
USHORT checksum(USHORT *buffer,int size);
