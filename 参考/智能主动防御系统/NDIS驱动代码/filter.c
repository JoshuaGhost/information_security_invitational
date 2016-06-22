/*++
 
模块名称:
 
    filter.c

    智能主动防御系统-NDIS中间层驱动程序-数据包过滤

--*/
#include "precomp.h"
#include "Packet.h"
#include "md5.h"
#include "hashlink.h"
#include "hostlist.h"

extern NDIS_PHYSICAL_ADDRESS	HighestAcceptableMax;
extern HostListLink netgatelist;    //网关信息列表
extern Attack_Infor attack_infor;   //攻击日志信息
extern PVOID  gpEventObject;        //通信事件

//基本过滤规则
extern UCHAR ICMP_FILTER;          //是否允许Ping入
extern UCHAR PACKET_FILTER;        //是否进行数据包过滤
extern UCHAR ANTIARP;              //是否启用ARP防火墙
extern NDIS_SPIN_LOCK  IcmpLock;
extern NDIS_SPIN_LOCK  PacketLock;
extern NDIS_SPIN_LOCK  ArpLock;
extern NDIS_SPIN_LOCK  NetGateListLock;
extern NDIS_SPIN_LOCK  HashLinkLock;
extern NDIS_SPIN_LOCK  AttackInforLock;



//数据包过滤函数
unsigned short ntohs(unsigned short num)
{
	unsigned short numRet;
	numRet = ((num << 8) | (num >> 8));
	return numRet;
}
NDIS_STATUS FilterPacket(IN	PNDIS_PACKET Packet)
{
	//------------------------数据包过滤---------------------------
			 int        PacketSize;
			 PUCHAR     pPacketContent;
			 PUCHAR     pBuf;
			 UINT       BufLength;
			 MDL    *   pNext;
			 UINT       i;
			 NDIS_STATUS Status;

			 DBGPRINT(("==>调用数据包过滤函数\n"));
			 
			 //把数据包内容从Packet拷贝到pPacketContent
			 NdisQueryPacket(Packet,NULL,NULL,NULL,&PacketSize);
			 Status= NdisAllocateMemory( &pPacketContent, 2000, 0,HighestAcceptableMax);
			 if (Status!=NDIS_STATUS_SUCCESS ) return Status;
			 NdisZeroMemory (pPacketContent, 2000);
			 
			 NdisQueryBufferSafe(Packet->Private.Head, &pBuf, &BufLength,32);
			 NdisMoveMemory(pPacketContent, pBuf, BufLength);
			 
			 i = BufLength;
			 pNext = Packet->Private.Head;
			 
			 for(;;)
			 {
				 if(pNext == Packet->Private.Tail)
					 break;
				 pNext = pNext->Next;   //指针后移
				 if(pNext == NULL) 
					 break;
				 
				 NdisQueryBufferSafe(pNext,&pBuf,&BufLength,32);
				 NdisMoveMemory(pPacketContent+i,pBuf,BufLength);
				 i+=BufLength;
			 }
			 			 
			 //数据拷贝完毕
			 //现在，我们已经在PtReceive函数中得到了数据报的内容，存放在pPacketContent中
       
			 //IP包
			 if(((char *)pPacketContent)[12] == 8 && 
				 ((char *)pPacketContent)[13] == 0)
			 {
				 //根据IP头中proto值判断传输层数据包类型
				 switch (((char *)pPacketContent)[23]) 
				 {
				 case 1: //ICMP包
					 {
						 Icmp_Packet *picmp_packet=NULL; //Icmp数据包结构体指针
						 picmp_packet=(Icmp_Packet *)pPacketContent;
             DbgPrint("==>ICMP数据包!\n"); 
						
						 NdisAcquireSpinLock(&IcmpLock);
						 //ICMP过滤开关-如果过滤开关没开启，则不予处理
						 if(!ICMP_FILTER) 
						 {
						 		NdisReleaseSpinLock(&IcmpLock);
						 		return 0;
						 }
						 NdisReleaseSpinLock(&IcmpLock);

						 //禁止目的IP是本机的Ping入
						 NdisAcquireSpinLock(&NetGateListLock);
						 if(picmp_packet->icmp_header.i_type==8 &&
								 FindHostList(netgatelist,picmp_packet->ip_header.destIP,picmp_packet->dlc_header.desmac))
						 {
							 NdisFreeMemory(pPacketContent, 2000, 0);
							 NdisReleaseSpinLock(&NetGateListLock);
							 DbgPrint("==>ICMP数据包被阻止!\n"); 
						   return NDIS_STATUS_NOT_ACCEPTED;
						 }
						 NdisReleaseSpinLock(&NetGateListLock);						 
						 return 0;
					 }
				 case 6: //TCP包
					 {
						 Tcp_Packet * ptcp_packet=NULL; //Tcp数据包结构体指针
						 UCHAR *pscode;//预处理数据起始指针
						 int codelen;  //预处理数据长度
						 char hash[HASH_HEX_LEN];
						 DbgPrint("==>TCP数据包!\n"); 
						 ptcp_packet=(Tcp_Packet *)pPacketContent;
						 
						  NdisAcquireSpinLock(&NetGateListLock);
						 //如果目的主机不在本机列表中则该数据包不予处理
						 if(!FindHostList(netgatelist,ptcp_packet->ip_header.destIP,ptcp_packet->dlc_header.desmac))
						 	{
						 		 NdisReleaseSpinLock(&NetGateListLock);
						 		 return 0;
						 	}
						  NdisReleaseSpinLock(&NetGateListLock);	
						  
						 //取得附加数据的长度
						 codelen=ntohs(ptcp_packet->ip_header.total_len)-((ptcp_packet->ip_header.ver_len) & 15)*4-((ptcp_packet->tcp_header.dataoff)>>4)*4;
						 DbgPrint("code len=%d\n",codelen);
						 if(codelen<10)//如果附加数据太短则不予处理
						 {
								 NdisFreeMemory(pPacketContent, 2000, 0);
								 return 0;		
						 }
						 //取得TCP包附加数据的起始位置
						 pscode=(UCHAR *)pPacketContent+14+((ptcp_packet->ip_header.ver_len) & 15)*4+((ptcp_packet->tcp_header.dataoff)>>4)*4;
						 //计算附加数据的md5摘要值
						 MD5_Hash(hash,pscode,codelen); 
						 DbgPrint(hash);//MD5串保存在hash中

						 //计算负载部分md5摘要值与特征库进行匹配
						 NdisAcquireSpinLock(&HashLinkLock);
						 if(md5_search(hash))//如果特征库中含有该特征，则丢弃该数据包
						 {
							 NdisReleaseSpinLock(&HashLinkLock);
							 //写日志
							 NdisAcquireSpinLock(&AttackInforLock);
							 NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
							 attack_infor.flag=2;
							 attack_infor.srcip=ptcp_packet->ip_header.sourceIP;
							 NdisMoveMemory((char *)attack_infor.code,hash,32);
							 //使通信事件有信号
							 if(gpEventObject)
								 KeSetEvent(gpEventObject,0,FALSE);
							 NdisReleaseSpinLock(&AttackInforLock);
							
							 NdisFreeMemory(pPacketContent, 2000, 0);
							 DbgPrint("==>TCP数据包被阻止!\n"); 
							 return NDIS_STATUS_NOT_ACCEPTED;
						 }
						 NdisReleaseSpinLock(&HashLinkLock);
						 
						 NdisFreeMemory(pPacketContent, 2000, 0);
						 return 0;
					 }
				 case 17: //UDP包
					 {
						 Udp_Packet *pudp_packet=NULL; //Udp数据包结构体
						 UCHAR *pscode;//预处理数据起始指针
						 int codelen;  //预处理数据长度
						 char hash[HASH_HEX_LEN];
						 DbgPrint("==>UDP数据包!\n");
						 pudp_packet=(Udp_Packet *)pPacketContent;
						 
						 NdisAcquireSpinLock(&NetGateListLock);
						 //如果目的主机不在本机列表中则该数据包不予处理
						 if(!FindHostList(netgatelist,pudp_packet->ip_header.destIP,pudp_packet->dlc_header.desmac))
						 	{
						 		 NdisReleaseSpinLock(&NetGateListLock);
						 		 return 0;
						 	}
						  NdisReleaseSpinLock(&NetGateListLock);
						 
						 //取得附加数据的长度
						 codelen=ntohs(pudp_packet->ip_header.total_len)-((pudp_packet->ip_header.ver_len) & 15)*4 -8;
						 DbgPrint("code len=%d\n",codelen);
						 if (codelen<10) //如果附加数据太短则不予处理
						 {
						 		NdisFreeMemory(pPacketContent, 2000, 0);
								return 0;					 		
						 }
						 //取得UDP包附加数据的起始位置
						 pscode=(UCHAR *)pPacketContent+14+((pudp_packet->ip_header.ver_len) & 15)*4+8;
						 //计算附加数据的md5摘要值
						 MD5_Hash(hash,pscode,codelen);
						 DbgPrint(hash);//MD5串保存在hash中
						 
						 //计算负载部分md5摘要值与特征库进行匹配
						 NdisAcquireSpinLock(&HashLinkLock);
						 if(md5_search(hash))//如果特征库中含有该特征，则丢弃该数据包
						 {
							 NdisReleaseSpinLock(&HashLinkLock);
							 //写日志
							 NdisAcquireSpinLock(&AttackInforLock);
							 NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
							 attack_infor.flag=3;
							 attack_infor.srcip=pudp_packet->ip_header.sourceIP;
							 NdisMoveMemory((char *)attack_infor.code,hash,32);
							 //使通信事件有信号
							 if(gpEventObject)
								 KeSetEvent(gpEventObject,0,FALSE);
							 NdisReleaseSpinLock(&AttackInforLock);
							 
							 NdisFreeMemory(pPacketContent, 2000, 0);
							 DbgPrint("==>UDP数据包被阻止!\n");
							 return NDIS_STATUS_NOT_ACCEPTED;
						 }
						 NdisReleaseSpinLock(&HashLinkLock);					
						 
						  NdisFreeMemory(pPacketContent, 2000, 0);
						  return 0;
					 }
				 default:
					 {
						 NdisFreeMemory(pPacketContent, 2000, 0);
					   return 0;
					 }
				 }	
			 }
			 //ARP包
			 if(((char *)pPacketContent)[12] == 8 && 
				 ((char *)pPacketContent)[13] == 6)
			 {
			 	 ARP_Packet *p_arp_packet=NULL;
				 NdisAcquireSpinLock(&ArpLock);
				 if(!ANTIARP) return 0;
				 NdisReleaseSpinLock(&ArpLock);
				 //DbgPrint("==>ARP数据包!\n");
			     
				 p_arp_packet=(ARP_Packet *)pPacketContent;
				 
				 //过滤:目的mac为广播的应答
				 if(ntohs(p_arp_packet->arpframe.flag)==2
					 &&p_arp_packet->dlcheader.desmac[0]==0xff
					 &&p_arp_packet->dlcheader.desmac[1]==0xff
					 &&p_arp_packet->dlcheader.desmac[2]==0xff
					 &&p_arp_packet->dlcheader.desmac[3]==0xff
					 &&p_arp_packet->dlcheader.desmac[4]==0xff
					 &&p_arp_packet->dlcheader.desmac[5]==0xff)
				 {
					
					 //写日志
					 NdisAcquireSpinLock(&AttackInforLock);
					 NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
					 attack_infor.flag=1;
					 attack_infor.srcip=p_arp_packet->arpframe.send_prot_addr;
					 //使通信事件有信号
					 if(gpEventObject)
						 KeSetEvent(gpEventObject,0,FALSE);
					 NdisReleaseSpinLock(&AttackInforLock);
					 DbgPrint("==>ARP数据包被阻止!\n");
					 return NDIS_STATUS_NOT_ACCEPTED;
				 }
			     //过滤:假冒网关,IP冲突
				 NdisAcquireSpinLock(&NetGateListLock);
				 if(FindHostList(netgatelist,
					 p_arp_packet->arpframe.send_prot_addr,
					 p_arp_packet->arpframe.send_hw_addr)==2)
				 {
					 //写日志
					 NdisAcquireSpinLock(&AttackInforLock);
					 NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
					 attack_infor.flag=1;
					 attack_infor.srcip=p_arp_packet->arpframe.send_prot_addr;
					 //使通信事件有信号
					 if(gpEventObject)
						 KeSetEvent(gpEventObject,0,FALSE);
					 NdisReleaseSpinLock(&AttackInforLock);
					 DbgPrint("==>ARP数据包被阻止!\n");
					 return NDIS_STATUS_NOT_ACCEPTED;
				 }
				 NdisReleaseSpinLock(&NetGateListLock);
				 NdisFreeMemory(pPacketContent, 2000, 0);
				 return 0;
			 }		 
			 NdisFreeMemory(pPacketContent, 2000, 0);
			 return 0;
			 //---------------------------------------------------------	
			 
}