#ifndef _WINPCAP_H_
#define _WINPCAP_H_
#define WPCAP
#define HAVE_REMOTE
#define WM_PRM_NOTIFY WM_USER+1 //�Զ���������Ϣ
#define UPDATE_LIVEHOST_INTERVAL 10000
#define DRIVERNAME "PassThru"
#include<winsock2.h>
#include<Iphlpapi.h>
#include "pcap.h"
#include "collection.h"
#include "IOCommon.h"
//������������Ϣ
#pragma pack(1)  
typedef struct{
	char name[256];          //��������
	UINT ipaddress;          //ip,������
	UINT netmask;            //netmask,������
	UINT netgate;            //netgate,������
	BYTE netgatemac[6];      //netgate mac
	BYTE mac[6];             //mac,������
	pcap_t * fp;             //pcap_open����ֵ
	UINT ipnow;              //��ǰɨ��ip,������
	BOOL scanliveover;       //ɨ���������Ƿ����
	BOOL hostlistok;         //��������б��Ƿ�׼����
	Collection livehostlist;  //��������б�,������
	UINT ipstart;            //������ʼip,������
	UINT ipend;              //���ν���ip,������
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
//����У���
USHORT checksum(USHORT *buffer,int size);
