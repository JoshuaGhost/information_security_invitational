/*++
 
ģ������:
 
    filter.c

    ������������ϵͳ-NDIS�м����������-���ݰ�����

--*/
#include "precomp.h"
#include "Packet.h"
#include "md5.h"
#include "hashlink.h"
#include "hostlist.h"

extern NDIS_PHYSICAL_ADDRESS	HighestAcceptableMax;
extern HostListLink netgatelist;    //������Ϣ�б�
extern Attack_Infor attack_infor;   //������־��Ϣ
extern PVOID  gpEventObject;        //ͨ���¼�

//�������˹���
extern UCHAR ICMP_FILTER;          //�Ƿ�����Ping��
extern UCHAR PACKET_FILTER;        //�Ƿ�������ݰ�����
extern UCHAR ANTIARP;              //�Ƿ�����ARP����ǽ
extern NDIS_SPIN_LOCK  IcmpLock;
extern NDIS_SPIN_LOCK  PacketLock;
extern NDIS_SPIN_LOCK  ArpLock;
extern NDIS_SPIN_LOCK  NetGateListLock;
extern NDIS_SPIN_LOCK  HashLinkLock;
extern NDIS_SPIN_LOCK  AttackInforLock;



//���ݰ����˺���
unsigned short ntohs(unsigned short num)
{
	unsigned short numRet;
	numRet = ((num << 8) | (num >> 8));
	return numRet;
}
NDIS_STATUS FilterPacket(IN	PNDIS_PACKET Packet)
{
	//------------------------���ݰ�����---------------------------
			 int        PacketSize;
			 PUCHAR     pPacketContent;
			 PUCHAR     pBuf;
			 UINT       BufLength;
			 MDL    *   pNext;
			 UINT       i;
			 NDIS_STATUS Status;

			 DBGPRINT(("==>�������ݰ����˺���\n"));
			 
			 //�����ݰ����ݴ�Packet������pPacketContent
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
				 pNext = pNext->Next;   //ָ�����
				 if(pNext == NULL) 
					 break;
				 
				 NdisQueryBufferSafe(pNext,&pBuf,&BufLength,32);
				 NdisMoveMemory(pPacketContent+i,pBuf,BufLength);
				 i+=BufLength;
			 }
			 			 
			 //���ݿ������
			 //���ڣ������Ѿ���PtReceive�����еõ������ݱ������ݣ������pPacketContent��
       
			 //IP��
			 if(((char *)pPacketContent)[12] == 8 && 
				 ((char *)pPacketContent)[13] == 0)
			 {
				 //����IPͷ��protoֵ�жϴ�������ݰ�����
				 switch (((char *)pPacketContent)[23]) 
				 {
				 case 1: //ICMP��
					 {
						 Icmp_Packet *picmp_packet=NULL; //Icmp���ݰ��ṹ��ָ��
						 picmp_packet=(Icmp_Packet *)pPacketContent;
             DbgPrint("==>ICMP���ݰ�!\n"); 
						
						 NdisAcquireSpinLock(&IcmpLock);
						 //ICMP���˿���-������˿���û���������账��
						 if(!ICMP_FILTER) 
						 {
						 		NdisReleaseSpinLock(&IcmpLock);
						 		return 0;
						 }
						 NdisReleaseSpinLock(&IcmpLock);

						 //��ֹĿ��IP�Ǳ�����Ping��
						 NdisAcquireSpinLock(&NetGateListLock);
						 if(picmp_packet->icmp_header.i_type==8 &&
								 FindHostList(netgatelist,picmp_packet->ip_header.destIP,picmp_packet->dlc_header.desmac))
						 {
							 NdisFreeMemory(pPacketContent, 2000, 0);
							 NdisReleaseSpinLock(&NetGateListLock);
							 DbgPrint("==>ICMP���ݰ�����ֹ!\n"); 
						   return NDIS_STATUS_NOT_ACCEPTED;
						 }
						 NdisReleaseSpinLock(&NetGateListLock);						 
						 return 0;
					 }
				 case 6: //TCP��
					 {
						 Tcp_Packet * ptcp_packet=NULL; //Tcp���ݰ��ṹ��ָ��
						 UCHAR *pscode;//Ԥ����������ʼָ��
						 int codelen;  //Ԥ�������ݳ���
						 char hash[HASH_HEX_LEN];
						 DbgPrint("==>TCP���ݰ�!\n"); 
						 ptcp_packet=(Tcp_Packet *)pPacketContent;
						 
						  NdisAcquireSpinLock(&NetGateListLock);
						 //���Ŀ���������ڱ����б���������ݰ����账��
						 if(!FindHostList(netgatelist,ptcp_packet->ip_header.destIP,ptcp_packet->dlc_header.desmac))
						 	{
						 		 NdisReleaseSpinLock(&NetGateListLock);
						 		 return 0;
						 	}
						  NdisReleaseSpinLock(&NetGateListLock);	
						  
						 //ȡ�ø������ݵĳ���
						 codelen=ntohs(ptcp_packet->ip_header.total_len)-((ptcp_packet->ip_header.ver_len) & 15)*4-((ptcp_packet->tcp_header.dataoff)>>4)*4;
						 DbgPrint("code len=%d\n",codelen);
						 if(codelen<10)//�����������̫�����账��
						 {
								 NdisFreeMemory(pPacketContent, 2000, 0);
								 return 0;		
						 }
						 //ȡ��TCP���������ݵ���ʼλ��
						 pscode=(UCHAR *)pPacketContent+14+((ptcp_packet->ip_header.ver_len) & 15)*4+((ptcp_packet->tcp_header.dataoff)>>4)*4;
						 //���㸽�����ݵ�md5ժҪֵ
						 MD5_Hash(hash,pscode,codelen); 
						 DbgPrint(hash);//MD5��������hash��

						 //���㸺�ز���md5ժҪֵ�����������ƥ��
						 NdisAcquireSpinLock(&HashLinkLock);
						 if(md5_search(hash))//����������к��и����������������ݰ�
						 {
							 NdisReleaseSpinLock(&HashLinkLock);
							 //д��־
							 NdisAcquireSpinLock(&AttackInforLock);
							 NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
							 attack_infor.flag=2;
							 attack_infor.srcip=ptcp_packet->ip_header.sourceIP;
							 NdisMoveMemory((char *)attack_infor.code,hash,32);
							 //ʹͨ���¼����ź�
							 if(gpEventObject)
								 KeSetEvent(gpEventObject,0,FALSE);
							 NdisReleaseSpinLock(&AttackInforLock);
							
							 NdisFreeMemory(pPacketContent, 2000, 0);
							 DbgPrint("==>TCP���ݰ�����ֹ!\n"); 
							 return NDIS_STATUS_NOT_ACCEPTED;
						 }
						 NdisReleaseSpinLock(&HashLinkLock);
						 
						 NdisFreeMemory(pPacketContent, 2000, 0);
						 return 0;
					 }
				 case 17: //UDP��
					 {
						 Udp_Packet *pudp_packet=NULL; //Udp���ݰ��ṹ��
						 UCHAR *pscode;//Ԥ����������ʼָ��
						 int codelen;  //Ԥ�������ݳ���
						 char hash[HASH_HEX_LEN];
						 DbgPrint("==>UDP���ݰ�!\n");
						 pudp_packet=(Udp_Packet *)pPacketContent;
						 
						 NdisAcquireSpinLock(&NetGateListLock);
						 //���Ŀ���������ڱ����б���������ݰ����账��
						 if(!FindHostList(netgatelist,pudp_packet->ip_header.destIP,pudp_packet->dlc_header.desmac))
						 	{
						 		 NdisReleaseSpinLock(&NetGateListLock);
						 		 return 0;
						 	}
						  NdisReleaseSpinLock(&NetGateListLock);
						 
						 //ȡ�ø������ݵĳ���
						 codelen=ntohs(pudp_packet->ip_header.total_len)-((pudp_packet->ip_header.ver_len) & 15)*4 -8;
						 DbgPrint("code len=%d\n",codelen);
						 if (codelen<10) //�����������̫�����账��
						 {
						 		NdisFreeMemory(pPacketContent, 2000, 0);
								return 0;					 		
						 }
						 //ȡ��UDP���������ݵ���ʼλ��
						 pscode=(UCHAR *)pPacketContent+14+((pudp_packet->ip_header.ver_len) & 15)*4+8;
						 //���㸽�����ݵ�md5ժҪֵ
						 MD5_Hash(hash,pscode,codelen);
						 DbgPrint(hash);//MD5��������hash��
						 
						 //���㸺�ز���md5ժҪֵ�����������ƥ��
						 NdisAcquireSpinLock(&HashLinkLock);
						 if(md5_search(hash))//����������к��и����������������ݰ�
						 {
							 NdisReleaseSpinLock(&HashLinkLock);
							 //д��־
							 NdisAcquireSpinLock(&AttackInforLock);
							 NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
							 attack_infor.flag=3;
							 attack_infor.srcip=pudp_packet->ip_header.sourceIP;
							 NdisMoveMemory((char *)attack_infor.code,hash,32);
							 //ʹͨ���¼����ź�
							 if(gpEventObject)
								 KeSetEvent(gpEventObject,0,FALSE);
							 NdisReleaseSpinLock(&AttackInforLock);
							 
							 NdisFreeMemory(pPacketContent, 2000, 0);
							 DbgPrint("==>UDP���ݰ�����ֹ!\n");
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
			 //ARP��
			 if(((char *)pPacketContent)[12] == 8 && 
				 ((char *)pPacketContent)[13] == 6)
			 {
			 	 ARP_Packet *p_arp_packet=NULL;
				 NdisAcquireSpinLock(&ArpLock);
				 if(!ANTIARP) return 0;
				 NdisReleaseSpinLock(&ArpLock);
				 //DbgPrint("==>ARP���ݰ�!\n");
			     
				 p_arp_packet=(ARP_Packet *)pPacketContent;
				 
				 //����:Ŀ��macΪ�㲥��Ӧ��
				 if(ntohs(p_arp_packet->arpframe.flag)==2
					 &&p_arp_packet->dlcheader.desmac[0]==0xff
					 &&p_arp_packet->dlcheader.desmac[1]==0xff
					 &&p_arp_packet->dlcheader.desmac[2]==0xff
					 &&p_arp_packet->dlcheader.desmac[3]==0xff
					 &&p_arp_packet->dlcheader.desmac[4]==0xff
					 &&p_arp_packet->dlcheader.desmac[5]==0xff)
				 {
					
					 //д��־
					 NdisAcquireSpinLock(&AttackInforLock);
					 NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
					 attack_infor.flag=1;
					 attack_infor.srcip=p_arp_packet->arpframe.send_prot_addr;
					 //ʹͨ���¼����ź�
					 if(gpEventObject)
						 KeSetEvent(gpEventObject,0,FALSE);
					 NdisReleaseSpinLock(&AttackInforLock);
					 DbgPrint("==>ARP���ݰ�����ֹ!\n");
					 return NDIS_STATUS_NOT_ACCEPTED;
				 }
			     //����:��ð����,IP��ͻ
				 NdisAcquireSpinLock(&NetGateListLock);
				 if(FindHostList(netgatelist,
					 p_arp_packet->arpframe.send_prot_addr,
					 p_arp_packet->arpframe.send_hw_addr)==2)
				 {
					 //д��־
					 NdisAcquireSpinLock(&AttackInforLock);
					 NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
					 attack_infor.flag=1;
					 attack_infor.srcip=p_arp_packet->arpframe.send_prot_addr;
					 //ʹͨ���¼����ź�
					 if(gpEventObject)
						 KeSetEvent(gpEventObject,0,FALSE);
					 NdisReleaseSpinLock(&AttackInforLock);
					 DbgPrint("==>ARP���ݰ�����ֹ!\n");
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