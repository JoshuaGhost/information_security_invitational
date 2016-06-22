/*++
 
模块名称:
 
    hostlist.c

    智能主动防御系统-NDIS中间层驱动程序-本机IP/MAC表

--*/
#include "precomp.h"
#include "hostlist.h"
extern NDIS_PHYSICAL_ADDRESS	HighestAcceptableMax;
//创建带有头节点的链表
int CreateHostList(HostListLink *host)
{
	if(NdisAllocateMemory(host, sizeof(HostListNode), 0,HighestAcceptableMax)!=NDIS_STATUS_SUCCESS)
	   return 1;
	(*host)->next=NULL;
	return 0;
}
//头插法，插入链表
int InsertHostList(HostListLink host,unsigned int ip,unsigned char mac[6])
{
	HostListLink newhostnode;
	if(!host) return 1;
	if(NdisAllocateMemory(&newhostnode, sizeof(HostListNode), 0,HighestAcceptableMax)!=NDIS_STATUS_SUCCESS)
	   return 1;
	if(!newhostnode) return 1;
	newhostnode->next=host->next;
	host->next=newhostnode;
	newhostnode->ip=ip;
	newhostnode->mac[0]=mac[0];
	newhostnode->mac[1]=mac[1];
	newhostnode->mac[2]=mac[2];
	newhostnode->mac[3]=mac[3];
	newhostnode->mac[4]=mac[4];
	newhostnode->mac[5]=mac[5];
	return 0;
}
//查询指ip是否存在
//返回值:0-不存在,1-存在，并且mac地址匹配,2-存在,并且mac地址不匹配
int FindHostList(HostListLink host,unsigned int ip,unsigned char mac[6])
{
	HostListLink p;
	p=host->next;
	while(p)
	{
		if(p->ip==ip)
		{
			if(p->mac[0]==mac[0]&&
				p->mac[1]==mac[1]&&
				p->mac[2]==mac[2]&&
				p->mac[3]==mac[3]&&
				p->mac[4]==mac[4]&&
				p->mac[5]==mac[5])
		    	return 1;
			else
				return 2;
		}
		p=p->next;
	}
    return 0;
}
