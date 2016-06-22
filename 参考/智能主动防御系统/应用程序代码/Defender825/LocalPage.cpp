// LocalPage.cpp : implementation file
#include "stdafx.h"
#include "defender.h"

#include "LoadcodeDlg.h"
#include "LocalPage.h"

#include "basic/Ado.h"
#include "net/winpcap.h"
#include "net/packet.h"
#include "net/md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Ҫ���ӵĿ�
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"ws2_32.lib")


BOOL checknet=TRUE;
BOOL checkhost=TRUE;

//�豸�������
HANDLE driverHandle=NULL;	
LPSTR driverDosName=NULL;

UINT gnIDEvent=1;//��ʱ����ʶ

//���˿���
UCHAR ICMP_FILTER=1;   //�Ƿ�����Ping��
UCHAR PACKET_FILTER=1; //�Ƿ�������ݰ�����
UCHAR ANTIARP=1;       //�Ƿ�����ARP����ǽ
UCHAR PLAYALLROLE=1;   //�Ƿ�αװ�����в���������

Adapter netcard[8];//����������

CLocalPage *p_this=NULL;


//�б��ָ��
CComboBox *p_listAdapter=NULL;
HANDLE m_hNewAttactEvent;
HANDLE m_hCommEvent;

//���صĹ�������
UINT g_iDefenceTimes=0;

BOOL bUseCodeDb=FALSE;

//״̬����Ϣ
#define UM_STATUS (WM_USER +12)    //�Զ���״̬����Ϣ
#define UM_MESSLIST (WM_USER +13)  //ʵʱ��Ϣ�б���Ϣ
#define UM_PCOUNT (WM_USER +15)    //�����ѹ��˵����ݰ�����Ϣ

//����ǰ·��
extern char szCurrentDirectory[256];

//��ȡ���������Ϣ
int GetAdapterInfor(Adapter *adapter);
//�ж�һ�黺�����Ƿ�Ϊȫ0:ȫ�㷵��TRUE,��ȫ�㷵��FALSE
BOOL IsAllZero(unsigned char dat[],int length);
//���ù��˹���(����)
int SetFilterRules(UCHAR icmp_filter,UCHAR packet_filter,UCHAR antiarp);
//��ʼ����
void startlisten();
//�����̺߳���
DWORD WINAPI ListenThreadFun( LPVOID lpParameter );
//��ʱ���´�������б�
void CALLBACK EXPORT UpdateLiveHost(HWND hWnd,UINT nMsg,UINT nIDEvent,DWORD dwTime );
//�򱾽ӿ����ڵľ����������л�������arp����
DWORD WINAPI ArpRequestToAll(LPVOID lpParameter );
//���ݰ�����
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
//���� ���ݰ�
void sendpacket(Adapter * adapter,BYTE * packet,int size);
//����������Ϣ(iP,mac)����������
int SendNetGateInfor(unsigned int netgateip,unsigned char netgatemac[6]);
//������ȡ�������͵Ĺ�����־��Ϣ�߳�
void StartReadAttackLog();
//��ȡ�������͵Ĺ�����־��Ϣ
DWORD WINAPI ReadAttackLog(LPVOID lpParameter);
//���յ�������־��Ϣд��ע�����
void WriteLogToMdb(char *logcontent);
//������������ݿ⣬�����µ��������͸���������
void InsertDB(char *shellcode);
//�ж������Ƿ���αװ�����б���
//����:   ip-Ҫ�жϵ�����IP(������)
//����ֵ: ����-TRUE,������-FALSE
BOOL IsInCheateHostList(unsigned int ip);
//��ѯĳ�����������ĳ�˿��Ƿ񿪷�
//����:   ip- ip-Ҫ�жϵ�����IP(������) protocol-Э������ port-Ҫ��ѯ�Ķ˿�(������)
//����ֵ: �ҵ�-TRUE,δ�ҵ�-FALSE
BOOL IsPortOpen(unsigned int ip,unsigned char protocol,unsigned short port);
//�ж�αװ�����Ƿ����ICMP
//����:   ip-Ҫ�жϵ�����IP(������)
//����ֵ: ����-����TTLֵ,������-0
unsigned char IsAcceptIcmp(unsigned int ip);
//����TCP���ݰ�У���
void PacketCheckSum(unsigned char packet[]);
//��ȡ�����������
void GetShellCodeCount();
/////////////////////////////////////////////////////////////////////////////
// CLocalPage property page

IMPLEMENT_DYNCREATE(CLocalPage, CPropertyPage)

CLocalPage::CLocalPage() : CPropertyPage(CLocalPage::IDD)
{
	//{{AFX_DATA_INIT(CLocalPage)
	//}}AFX_DATA_INIT
}

CLocalPage::~CLocalPage()
{
}

void CLocalPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocalPage)
	DDX_Control(pDX, IDC_ADAPTER_LIST, m_listAdapter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocalPage, CPropertyPage)
	//{{AFX_MSG_MAP(CLocalPage)
	ON_CBN_SELCHANGE(IDC_ADAPTER_LIST, OnSelchangeAdapterList)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_MESLIST, OnRclickMeslist)
	ON_COMMAND(IDM_DELETE, OnDelete)
	ON_COMMAND(IDM_DELETEALL, OnDeleteall)
	ON_COMMAND(IDM_NETSHOW, OnNetshow)
	ON_COMMAND(IDM_HOSTSHOW, OnHostshow)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UM_MESSLIST,OnMessageListMessage)
	ON_MESSAGE(UM_PCOUNT,OnMessagePcount)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocalPage message handlers

BOOL CLocalPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	p_listAdapter=&m_listAdapter;
	p_this=this;
	

	//��ʼ���б�
	ListView_SetExtendedListViewStyle(
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->m_hWnd, 
		LVS_EX_GRIDLINES  | LVS_EX_FULLROWSELECT
		); 
	
	
	((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "ʱ��",LVCFMT_LEFT,150);

	((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "�����Ϣ",LVCFMT_LEFT,600);


	//���ù��˹���
    SetFilterRules(ICMP_FILTER,PACKET_FILTER,ANTIARP);
	memset(netcard,0,sizeof(Adapter) *8);
	//��ȡ���������Ϣ
	if(GetAdapterInfor(&netcard[0]))
	{
		AfxMessageBox("��ȡ������Ϣʧ��!");
		PostQuitMessage(0);
	}else
	{
		m_listAdapter.SetCurSel(0);
		OnSelchangeAdapterList();
	}
	//���������߳�
	startlisten();	
	gnIDEvent=1; //��ʱ����ʾ��
	//������ʱ���´�������б�ʱ��
	this->SetTimer(gnIDEvent,50,UpdateLiveHost);
	//������ȡ�������͵Ĺ�����־��Ϣ�߳�
	StartReadAttackLog();
	//��ʾͳ����Ϣ-����������
	GetShellCodeCount();
	return TRUE; 
}

//��ʾ������ӿ���Ϣ
void CLocalPage::OnSelchangeAdapterList() 
{
	Adapter * padapter=NULL;
	int i=0;
	char tempbuf[256];
	struct in_addr in;
	//��ȡ��ǰ��������
	memset(tempbuf,0,256);
	this->GetDlgItemText(IDC_ADAPTER_LIST,tempbuf,256);
	while(!IsAllZero((BYTE *)&netcard[i],sizeof(Adapter)))
	{
		if(!strcmp(netcard[i].name,tempbuf))
		{
			padapter=&netcard[i];
			break;
		}
		i++;
	}
	if(!padapter) return;
	//��ʾ�ӿ���Ϣ
	in.S_un.S_addr=padapter->ipaddress;
	//ip
	this->SetDlgItemText(IDC_IPADDR,inet_ntoa(in));
	in.S_un.S_addr=padapter->netmask;
	//netmask
	this->SetDlgItemText(IDC_NETMASK,inet_ntoa(in));
	in.S_un.S_addr=padapter->netgate;
	//net gate
	this->SetDlgItemText(IDC_NETGATE,inet_ntoa(in));
	memset(tempbuf,0,256);
	sprintf(tempbuf,"%02x-%02x-%02x-%02x-%02x-%02x",
		padapter->mac[0],
		padapter->mac[1],
		padapter->mac[2],
		padapter->mac[3],
		padapter->mac[4],
		padapter->mac[5]);
	//mac
	this->SetDlgItemText(IDC_MACADDR,tempbuf);
}

void CLocalPage::OnPaint() 
{
	CPaintDC dc(this);
	if(GetParent()->IsWindowVisible()==false)
	{
		this->ShowWindow(SW_HIDE);
		return ;
	}
	//��ȡͳ����Ϣ
	GetStatistics();
}

//��ȡ���������Ϣ
int GetAdapterInfor(Adapter *adapter)
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen;
	int i=0;
	UINT iplen;

	pAdapterInfo=(PIP_ADAPTER_INFO)malloc(sizeof(IP_ADAPTER_INFO));
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);	
	// ��һ�ε���GetAdapterInfo��ȡulOutBufLen��С
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen); 
	}	
	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter&&i<7) 
		{
			if(strlen(pAdapter->AdapterName)<200)
			{
				//ip
				adapter->ipaddress=inet_addr(pAdapter->IpAddressList.IpAddress.String);
				//netmask
				adapter->netmask=inet_addr(pAdapter->IpAddressList.IpMask.String);
				//netgate
				adapter->netgate=inet_addr(pAdapter->GatewayList.IpAddress.String);

				//������������
				iplen=0xFFFFFFFF-ntohl(adapter->netmask);
				//��ʼIP
				adapter->ipstart=(ntohl(adapter->ipaddress) & ntohl(adapter->netmask))+1;
				//����IP
				adapter->ipend=adapter->ipstart+iplen-2;
				
				//������ǲ��������ӿ�
				if(adapter->ipstart<adapter->ipend)
				{
					//adapter name
					strcpy(adapter->name,"rpcap://\\Device\\NPF_");
					strcat(adapter->name,pAdapter->AdapterName);
					p_listAdapter->AddString(adapter->name);
					//mac
					memcpy(adapter->mac,pAdapter->Address,6);
					//������ip��mac���͸���������
					SendNetGateInfor(adapter->ipaddress,adapter->mac);
					// livehostlist ��ʼ����������б�
					CreateCollection(adapter->livehostlist);			
					adapter++;
					i++;
				}
				else
					//����
					memset(adapter,0,sizeof(Adapter));
			}
			pAdapter = pAdapter->Next;
		}
	}
	else
		return 1;
	return 0;
}



//�ж�һ�黺�����Ƿ�Ϊȫ0
//ȫ�㷵��TRUE,��ȫ�㷵��FALSE
BOOL IsAllZero(unsigned char dat[],int length)
{
	int i=0;
	for(i=0;i<length;i++)
	{
		if(dat[i]) return FALSE;
	}
	return TRUE;
}


//���ù��˹���
int SetFilterRules(UCHAR icmp_filter,UCHAR packet_filter,UCHAR antiarp)
{
	unsigned char inBuffer[2]={0};
	DWORD bytesReturned=0;
	BOOL ret=0;

	if(!driverHandle)//��������豸û�д�
	{
		driverHandle = CreateFile(driverDosName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if(driverHandle == INVALID_HANDLE_VALUE)
		{
			AfxMessageBox("�������豸ʧ��!");
			return 1;
		}
	}
	
	inBuffer[0]=icmp_filter;
	ret=DeviceIoControl(driverHandle,
		IADS_ICMP_FILTER,
		inBuffer,
		2,
		NULL,
		0,
		&bytesReturned,
		NULL);//ͬ������
	if(!ret) return 1;
	inBuffer[0]=packet_filter;
	ret=DeviceIoControl(driverHandle,
		IADS_PACKET_FILTER,
		inBuffer,
		2,
		NULL,
		0,
		&bytesReturned,
		NULL);//ͬ������
	if(!ret) return 1;
	inBuffer[0]=antiarp;
	ret=DeviceIoControl(driverHandle,
		IADS_ANTIARP,
		inBuffer,
		2,
		NULL,
		0,
		&bytesReturned,
		NULL);//ͬ������
	if(!ret) return 1;
	return 0;
}



void startlisten()
{
	// ��ʼ�����������е�����ӿ�
	HANDLE PThreads[8];
	DWORD dwThreadId;
	int i=0;
	while(!IsAllZero((BYTE *)&netcard[i],sizeof(Adapter)))
	{
		//PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:���ڽ��������߳�...");
		PThreads[i]=CreateThread(NULL,
			0,
			ListenThreadFun,
			(LPVOID)&netcard[i],
			0,
			&dwThreadId);
		i++;
	}
	for(;i;i--)
	{
		CloseHandle(PThreads[i-1]);
	}
	//PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:�����߳̽������!");
}

//�����̺߳���
DWORD WINAPI ListenThreadFun(
  LPVOID lpParameter // thread data
  )
{
	Adapter *adapter=(Adapter *)lpParameter;
	char errbuf[PCAP_ERRBUF_SIZE];
    //�������� 
	if(!adapter->fp)//����ýӿ�û�д򿪹�
	{
		if ((adapter->fp= pcap_open(adapter->name,  // �豸��
			65536,                                  // Ҫ��׽�����ݰ��Ĳ��� 
			PCAP_OPENFLAG_PROMISCUOUS,              // ����ģʽ
			1000,                                   // ��ȡ��ʱʱ��
			NULL,                                   // Զ�̻�����֤
			errbuf                                  // ���󻺳��
			)) == NULL)
		{
			PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:������������!");
			return 0;
		}
	}
	pcap_loop(adapter->fp, 0, packet_handler, (unsigned char *)lpParameter);
	return 0;
}

//���ݰ�����
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{

	Adapter * adapter=(Adapter *)param;//���ݰ����Ըýӿ�
	
	//�Ѵ������ݰ���+1
	if(PACKET_FILTER)
		PostMessage(p_this->m_hWnd,UM_PCOUNT,(WPARAM)0,(LPARAM)0);
	
	if(adapter->scanliveover) //ɨ��ýӿڴ����������
	{
		//ɾ������ʧ������
		DeleteFlagZNode(adapter->livehostlist);
	}

	Dlc_Header *dlc_header;
	dlc_header=(Dlc_Header *)pkt_data;
	
	switch(ntohs(dlc_header->ethertype))//�ж���IP������ARP��
	{
	case 0x0806://arp��
		{
			Arp_Frame *arp_frame;
			arp_frame=(Arp_Frame *)(pkt_data + sizeof(Dlc_Header));
		
			//���dest ip�뱾������ͬһ�������账��
			if(ntohl(arp_frame->targ_prot_addr)<adapter->ipstart
			   ||ntohl(arp_frame->targ_prot_addr)>adapter->ipend
			   )  return;

			if(ntohs(arp_frame->flag)==0x01)//arp request
			{
				//����arp�㲥 srcip=destip
				if(arp_frame->send_prot_addr==arp_frame->targ_prot_addr)
				{
					InsertColletion(adapter->livehostlist,arp_frame->send_prot_addr);
					return;
				}
			
				if(adapter->hostlistok)//�����������б���׼����
				{
					//���������ǲ��������(ip),���Ҳ��Ǳ����������ʵ������,���Ҳ��Ƿ������ص�
					//����αװ���������ļ����и���������������Ӧ��
					if(!FindNode(adapter->livehostlist,arp_frame->targ_prot_addr)
						&&(arp_frame->padding)[0]!='x'
						&&(arp_frame->padding)[1]!='d'
						&&arp_frame->targ_prot_addr!=adapter->netgate
						&&IsInCheateHostList(arp_frame->targ_prot_addr))
					{	
						struct in_addr in,in2;
						char szMesBuf[256]={0};

						//״̬����ʾ
						PostMessage(AfxGetMainWnd()->m_hWnd,UM_STATUS,
							(WPARAM)0,(LPARAM)"��ʾ��Ϣ:ϵͳ�����򲻴�����������ARP����,���������Ӧ��!");

						in.S_un.S_addr=arp_frame->send_prot_addr;						
						in2.S_un.S_addr=arp_frame->targ_prot_addr;
						strcpy(szMesBuf,"ϵͳ��������");
						strcat(szMesBuf,inet_ntoa(in));
						strcat(szMesBuf,"�򲻴������");
						strcat(szMesBuf,inet_ntoa(in2));
						strcat(szMesBuf,"����ARP���󣬲����������Ӧ��!");
                 
						//ʵʱ��Ϣ����ʾ
						if(checknet)
						   PostMessage(p_this->m_hWnd,UM_MESSLIST,(WPARAM)0,(LPARAM)szMesBuf);
			

						//����Ӧ�����ݰ�
						ARP_Packet arp_packet;
						//dest mac
						memcpy((BYTE *)arp_packet.dlcheader.desmac,(BYTE *)dlc_header->srcmac,6);
						//src mac
						memcpy((BYTE *)arp_packet.dlcheader.srcmac,(BYTE *)adapter->mac,6);
						//ethertype
						arp_packet.dlcheader.ethertype=htons(0x0806);
						
						arp_packet.arpframe.flag=htons(0x02);
						arp_packet.arpframe.hw_addr_len=0x06;
						arp_packet.arpframe.prot_addr_len=0x04;
						arp_packet.arpframe.hw_type=htons(0x01);
						arp_packet.arpframe.prot_type=htons(0x0800);
						
						//arp src mac
						memcpy((BYTE *)arp_packet.arpframe.send_hw_addr,(BYTE *)adapter->mac,6);
						//arp dest mac
						memcpy((BYTE *)arp_packet.arpframe.targ_hw_addr,(BYTE *)dlc_header->srcmac,6);
						//arp src ip
						arp_packet.arpframe.send_prot_addr=arp_frame->targ_prot_addr;
						//arp dest ip
						arp_packet.arpframe.targ_prot_addr=arp_frame->send_prot_addr;
						
						//�ڸ����ֽ��� ���뱾����ı�־
						arp_packet.arpframe.padding[0]='x';
						arp_packet.arpframe.padding[1]='d';
						
						//����Ӧ��
						sendpacket(adapter,(BYTE *)&arp_packet,sizeof(ARP_Packet));
						return;
				  }
				}
				return;
			}
			else if(ntohs(arp_frame->flag)==0x02)//arp response
			{
				//����Ƿ���������Ӧ��
				if(arp_frame->targ_prot_addr==adapter->ipaddress)
				{
					//��������ص�Ӧ�����ȡ��mac��ַ
					if(arp_frame->send_prot_addr==adapter->netgate
						&&!adapter->netgatemac[0]
						&&!adapter->netgatemac[1]
						&&!adapter->netgatemac[2]
						&&!adapter->netgatemac[3]
						&&!adapter->netgatemac[4]
						&&!adapter->netgatemac[5])
					{
						//�ڴ�������ʾ����MAC��ַ
						char tempbuf[20];
						memset(tempbuf,0,20);
						sprintf(tempbuf,"%02x-%02x-%02x-%02x-%02x-%02x",
								arp_frame->send_hw_addr[0],
								arp_frame->send_hw_addr[1],
								arp_frame->send_hw_addr[2],
								arp_frame->send_hw_addr[3],
								arp_frame->send_hw_addr[4],
								arp_frame->send_hw_addr[4]);
						p_this->SetDlgItemText(IDC_NETGATEMAC,tempbuf);
						memcpy(adapter->netgatemac,arp_frame->send_hw_addr,6);
						//����������Ϣ����������
						SendNetGateInfor(arp_frame->send_prot_addr,arp_frame->send_hw_addr);
					}
					InsertColletion(adapter->livehostlist,arp_frame->send_prot_addr);
					return;
				}
			    return;
			}
			break;
		}
	case 0x0800://ip��
		{
			Ip_Header *ip_header;
			ip_header=(Ip_Header *)(pkt_data + sizeof(Dlc_Header));
		
			//ֻ����dest macΪ���ӿڶ�dest ip�Ǳ��ӿڵ����ݰ�
			if(ntohl(ip_header->destIP)<adapter->ipstart||ntohl(ip_header->destIP)>adapter->ipend)  return;
			if(ip_header->destIP==adapter->ipaddress) return;
			if(dlc_header->desmac[0]!=adapter->mac[0]
				||dlc_header->desmac[1]!=adapter->mac[1]
				||dlc_header->desmac[2]!=adapter->mac[2]
				||dlc_header->desmac[3]!=adapter->mac[3]
				||dlc_header->desmac[4]!=adapter->mac[4]
				||dlc_header->desmac[5]!=adapter->mac[5]
				) return;

			switch(ip_header->proto)
			{
			case 0x01: //icmp
				{
					unsigned char ucTtl=0;
				    Icmp_Packet *picmp_packet;
					picmp_packet=(Icmp_Packet *)pkt_data;	
					//��ѯ�����ļ�
					ucTtl=IsAcceptIcmp(picmp_packet->ip_header.destIP);
					if(!ucTtl) return;

					//����icmp��
					Icmp_Packet icmp_packet;
					//dest mac
					memcpy((BYTE *)icmp_packet.dlc_header.desmac,(BYTE *)dlc_header->srcmac,6);
					//src mac
					memcpy((BYTE *)icmp_packet.dlc_header.srcmac,(BYTE *)adapter->mac,6);
					//ether type
					icmp_packet.dlc_header.ethertype=dlc_header->ethertype;
					icmp_packet.ip_header.ver_len=picmp_packet->ip_header.ver_len;
					icmp_packet.ip_header.ident=picmp_packet->ip_header.ident;
					icmp_packet.ip_header.proto=picmp_packet->ip_header.proto;
					icmp_packet.ip_header.frag_and_flags=picmp_packet->ip_header.frag_and_flags;
					icmp_packet.ip_header.destIP=picmp_packet->ip_header.sourceIP;
					icmp_packet.ip_header.sourceIP=picmp_packet->ip_header.destIP;
					icmp_packet.ip_header.tos=picmp_packet->ip_header.tos;
					icmp_packet.ip_header.ttl=ucTtl;
					icmp_packet.ip_header.total_len=picmp_packet->ip_header.total_len;
					icmp_packet.ip_header.checksum=0;
					icmp_packet.ip_header.checksum=checksum((USHORT *)((BYTE *)&icmp_packet+14),20);
					
					icmp_packet.icmp_header.i_code=(BYTE)0;
					icmp_packet.icmp_header.i_id=picmp_packet->icmp_header.i_id;
					icmp_packet.icmp_header.i_seq=picmp_packet->icmp_header.i_seq;
					icmp_packet.icmp_header.i_type=0;
					//icmp_packet.icmp_header.timestamp=::GetTickCount();
					icmp_packet.icmp_header.i_cksum=0;
					memcpy((BYTE *)icmp_packet.icmp_header.padding,(BYTE *)picmp_packet->icmp_header.padding,32);
					icmp_packet.icmp_header.i_cksum=checksum((USHORT *)((BYTE *)&icmp_packet+14+20),sizeof(Icmp_Header));
					sendpacket(adapter,(BYTE *)&icmp_packet,sizeof(Icmp_Packet));					
				    break;
				}
			case 0x06://tcp
				{
					BYTE syn=0,ack=0;//ͬ��ȷ�ϱ�־λ
					Tcp_Header *ptcp_header;
					ptcp_header=(Tcp_Header *)(pkt_data+14+20);//tcpͷ
					
					//Ŀ�Ķ˿��Ƿ񿪷�
					if(!IsPortOpen(ip_header->destIP,0x06,ptcp_header->dstport)) return;

					//��ȡsyn��־λ
					if(((ptcp_header->flags) & 2)==2) 
						syn=1;
					else 
						syn=0;
					//��ȡack��־λ
					if(((ptcp_header->flags) & 16)==16)
						ack=1;
					else
						ack=0;

					//��Ӧ�ڶ�������
					if(syn==1
					   &&ack==0
					   &&ptcp_header->acknum==0)
					{
						unsigned char tcpbuf[66];
						unsigned char ucTcpReplyOption[13]={0x02,0x04,0x05,0xb4,0x01,0x01,0x04,0x02,0x01,0x03,0x03,0x02,0x00};
						memset(tcpbuf,0,66);
						memcpy(tcpbuf,pkt_data,66);
						//dest mac
						memcpy((BYTE *)(((Dlc_Header *)tcpbuf)->desmac),(BYTE *)(dlc_header->srcmac),6);
						//src mac
						memcpy((BYTE *)(((Dlc_Header *)tcpbuf)->srcmac),(BYTE *)(dlc_header->desmac),6);
						//src ip
						((Ip_Header *)(tcpbuf+14))->sourceIP=ip_header->destIP;
						//dest ip
					    ((Ip_Header *)(tcpbuf+14))->destIP=ip_header->sourceIP;
						//checksum
						((Ip_Header *)(tcpbuf+14))->checksum=0;
						//ip ttl
						((Ip_Header *)(tcpbuf+14))->ttl=80;
						//src port
						((Tcp_Header *)(tcpbuf+14+20))->srcport=ptcp_header->dstport;
						//dest port
						((Tcp_Header *)(tcpbuf+14+20))->dstport=ptcp_header->srcport;
						//an
						((Tcp_Header *)(tcpbuf+14+20))->acknum=htonl(ntohl(ptcp_header->seqnum)+1);
						//sn
						((Tcp_Header *)(tcpbuf+14+20))->seqnum=htonl(ntohl(ptcp_header->seqnum)+10086);
						//flag
						((Tcp_Header *)(tcpbuf+14+20))->flags=0x12;
						//window
						((Tcp_Header *)(tcpbuf+14+20))->window=htons(2920);
						//TCP Option
						memcpy(tcpbuf+14+20+20,ucTcpReplyOption,12);

						//����У���
						PacketCheckSum(tcpbuf);						
						//����
						sendpacket(adapter,(BYTE *)tcpbuf,66);
						return;
					}
					else if(ack==1&&syn==0)
					{
						//�������ֺ�ͻ��˷��͵����ݰ�
						BYTE * pscode;//���ݰ���������
						int codelen;//�������ݳ���
						codelen=(ntohs(ip_header->total_len))-((ip_header->ver_len) & 15)*4-((ptcp_header->dataoff)>>4)*4;
						//�����������̫���򲻴���
						if(codelen<=10) return;
						pscode=(BYTE *)malloc(codelen);
						memcpy(pscode,pkt_data+14+((ip_header->ver_len) & 15)*4+((ptcp_header->dataoff)>>4)*4,codelen);
						
						//����Ƿ���80�˿�
						if(80==ntohs(ptcp_header->dstport))
						{
							//ȡ��http����(��pscode)
							if(!strncmp((char *)pscode,"GET",3)||!strncmp((char *)pscode,"POST",4))
							{
								char szCrlf[5]={'\x0d','\x0a','\x0d','\x0a',0};
								char *pCrlf=NULL;
								pCrlf=strstr((char *)pscode,szCrlf);
								//���HTTP�������и�������
								if(pCrlf)
								{
									if((unsigned char)*(pCrlf+4))
									{
										//�����������ݵ�ժҪֵ
										MD5_CTX  cmd5;
										char result[33];
										cmd5.MD5Update(pscode,codelen);
										cmd5.MD5Final(result);
										//�����������ֵ����������ݿ�
										InsertDB(result);
									}
								}
								//Ӧ��HTTP����
								unsigned int scriptlen=0;
								unsigned char *scriptbuf=NULL;
								unsigned char *tcpbuf=NULL;
								unsigned int packetsize=0;
								CFile cfile; 
								if(cfile.Open("iisresponse.scri",CFile::modeRead))
								{
									scriptlen=cfile.GetLength();
									//Ϊ��ȡ�ű��ļ������ڴ�
									scriptbuf=(unsigned char *)malloc(scriptlen);						
									//�����ڴ�ʧ��
									if(!scriptbuf)
									{
										cfile.Close();
										return;
									}
									memset(scriptbuf,0,scriptlen);
									//��ȡ�ű��ļ�����
									cfile.Read(scriptbuf,scriptlen);
	
									//����Ӧ�����ݰ����ܳ���
									packetsize=scriptlen+14+((ip_header->ver_len) & 15)*4+((ptcp_header->dataoff)>>4)*4+4;
									//ΪӦ�����ݰ������ڴ�
									tcpbuf=(unsigned char *)malloc(packetsize);
									if(!tcpbuf)
									{
										cfile.Close();
										return;
									}
									
									memset(tcpbuf,0,packetsize);

									//�������ĵ�Э��ͷ���ֿ�������
									memcpy(tcpbuf,pkt_data,14+((ip_header->ver_len) & 15)*4+((ptcp_header->dataoff)>>4)*4);
									
									//����Ӧ��HTTP����
									memcpy(tcpbuf+14+((ip_header->ver_len) & 15)*4+((ptcp_header->dataoff)>>4)*4,
										scriptbuf,scriptlen);

									//dest mac
									memcpy((BYTE *)(((Dlc_Header *)tcpbuf)->desmac),(BYTE *)(dlc_header->srcmac),6);
									//src mac
									memcpy((BYTE *)(((Dlc_Header *)tcpbuf)->srcmac),(BYTE *)(dlc_header->desmac),6);
									//src ip
									((Ip_Header *)(tcpbuf+14))->sourceIP=ip_header->destIP;
									//dest ip
									((Ip_Header *)(tcpbuf+14))->destIP=ip_header->sourceIP;
									//total_len
									((Ip_Header *)(tcpbuf+14))->total_len=htons(packetsize-14);
									//src port
									((Tcp_Header *)(tcpbuf+14+20))->srcport=ptcp_header->dstport;
									//dest port
									((Tcp_Header *)(tcpbuf+14+20))->dstport=ptcp_header->srcport;
									//an
									((Tcp_Header *)(tcpbuf+14+20))->acknum=htonl(ntohl(ptcp_header->seqnum)+1);
									//sn
									((Tcp_Header *)(tcpbuf+14+20))->seqnum=htonl(ntohl(ptcp_header->seqnum)+10086);
									
									//����У���
									PacketCheckSum(tcpbuf);						
									//����
									sendpacket(adapter,(BYTE *)tcpbuf,packetsize);
								}
								cfile.Close();
								return;
							}
						}
						//�����������ݵ�ժҪֵ
						MD5_CTX  cmd5;
						char result[33]={0};
						cmd5.MD5Update(pscode,codelen);
						cmd5.MD5Final(result);
						//�����������ֵ����������ݿ�
						InsertDB(result);
						return;
					}
					break;
				}
			case 0x11: //udp
				{
					Udp_Header *pudp_header=NULL;
					pudp_header=(Udp_Header *)(pkt_data+14+((ip_header->ver_len) & 15)*4);//udpͷ
					
					//Ŀ�Ķ˿��Ƿ񿪷�
					if(!IsPortOpen(ip_header->destIP,0x11,pudp_header->dstport)) return;

					BYTE * pscode=NULL;//���ݰ���������
					int codelen;//�������ݳ���
					codelen=(ntohs(ip_header->total_len))-((ip_header->ver_len) & 15)*4-8;
					

					//�����������̫���򲻴���
					if(codelen<=10) return;

					pscode=(BYTE *)malloc(codelen);
					if(!pscode) return;

					memset(pscode,0,sizeof(BYTE)*codelen);
					memcpy(pscode,pkt_data+14+((ip_header->ver_len) & 15)*4+8,codelen);
					
					//�����������ݵ�ժҪֵ
					
					MD5_CTX  cmd5;
					char result[33]={0};
					cmd5.MD5Update(pscode,codelen);
					cmd5.MD5Final(result);
					
					//�����������ֵ����������ݿ�
					/*
					if(!bUseCodeDb)
					   InsertDB(result);*/

					free(pscode);
					break;
				}
			default:
				break;
			}
			return;
		}
	default://���������ݲ�����
		return;
	}
	return;
}

//���� ���ݰ�
void sendpacket(Adapter * adapter,BYTE * packet,int size)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	//��������
	if(!adapter->fp) //����ýӿ�û�б��򿪹�
	{
		if ((adapter->fp= pcap_open(adapter->name,  // �豸��
			100,                               // Ҫ��׽�����ݰ��Ĳ��� 
			PCAP_OPENFLAG_PROMISCUOUS,         // ����ģʽ
			1000,                              // ��ȡ��ʱʱ��
			NULL,                              // Զ�̻�����֤
			errbuf                             // ���󻺳��
			)) == NULL)
		{
			PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:������������!");
			return;
		}	
	}
	//����arp request���ݰ�
	 if (pcap_sendpacket(adapter->fp, (BYTE *)packet, size) != 0)
    {
		 PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:�������ݰ�����!");
    }
    return;
}


//����������Ϣ(iP,mac)����������
//����:(netgateip,����ip,netgatemac,����mac)
//����ֵ���ɹ�����0��ʧ�ܷ���1
int SendNetGateInfor(unsigned int netgateip,unsigned char netgatemac[6])
{
	HostInfor hostinfor;
	BOOL ret=0;
	DWORD bytesReturned=0;
	memset(&hostinfor,0,sizeof(HostInfor));
	hostinfor.ip=netgateip;
	memcpy(hostinfor.mac,netgatemac,6);
	if(!driverHandle)//��������豸û�д�
	{
		driverHandle = CreateFile(driverDosName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if(driverHandle == INVALID_HANDLE_VALUE)
		{
			AfxMessageBox("�������豸ʧ��!");
			return 1;
		}
	}
	ret=DeviceIoControl(driverHandle,
		IADS_DRIVER_NETGATE,
		(unsigned char *)&hostinfor,
		sizeof(HostInfor),
		NULL,
		0,
		&bytesReturned,
		NULL //ͬ������
		);
	if(!ret) return 1;
	 return 0;

}


//������������ݿ⣬�����µ��������͸���������
void InsertDB(char *shellcode)
{
	int count=0;
	count=p_this->GetDlgItemInt(IDC_TESTCOUNT);
	p_this->SetDlgItemInt(IDC_TESTCOUNT,count+1);

	bUseCodeDb=TRUE;
	
	DWORD bytesReturned=0;
	CString csSql=_T("");
	
	CADODatabase* pAdoDb = new CADODatabase();
    if(!pAdoDb) return;

	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=code.mdb");
	pAdoDb->SetConnectionString(strConnection);
	
	if(pAdoDb->Open())
	{
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		if(!pAdoRs)
		{
		   pAdoDb->Close();
		   return;
		 }
		
		csSql="select * from shellcode where shellcode ='";
		csSql=csSql+shellcode;
		csSql=csSql+"'";

		if(pAdoRs->Open(csSql, CADORecordset::openQuery))
		{
			
			if(pAdoRs->IsEOF()) //��������
			{
				pAdoRs->AddNew();
				pAdoRs->SetFieldValue("shellcode",_variant_t(shellcode));
				pAdoRs->Update();
				pAdoRs->Close();
			
				if(driverHandle)
				{
					//���µ��������͸���������
					DeviceIoControl(driverHandle,
						IADS_DRIVER_ADDCODE,
						(LPVOID)shellcode,
						32,
						NULL,
						0,
						&bytesReturned,
						NULL //ͬ������
						);
				}
				//״̬����ʾ
				PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:�����µ�����������������!");		
				//ʵʱ��Ϣ��ʾ
				if(checknet)
					PostMessage(p_this->m_hWnd,UM_MESSLIST,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:�����µ�����������������!");
				
				//ͳ����Ϣ ����������+1
				unsigned int uiScount=0;
				uiScount=p_this->GetDlgItemInt(IDC_SCCOUNT);
				p_this->SetDlgItemInt(IDC_SCCOUNT,++uiScount);
			}
			
			pAdoRs->Close();
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		MessageBox(0,"д��������ʧ��!","������������ϵͳ",MB_OK);
	}
	delete pAdoDb;
	bUseCodeDb=FALSE;
	return;
}

//������ȡ�������͵Ĺ�����־��Ϣ�߳�
void StartReadAttackLog()
{
	HANDLE PThread;//�߳̾��
	DWORD dwThreadId;
	PThread=CreateThread(NULL,
		0,
		ReadAttackLog,
		NULL,
		0,
		&dwThreadId);
	CloseHandle(PThread);//�ر��߳̾��
	return;
}
//��ȡ�������͵Ĺ�����־��Ϣ
DWORD WINAPI ReadAttackLog(
	LPVOID lpParameter   // thread data
)
{
	DWORD bytesReturned=0;
	BOOL ret=0;
	Attack_Infor attack_infor;
	struct in_addr in;
	char tempbuf[100];

	if(!driverHandle)//��������豸û�д�
	{
		driverHandle = CreateFile(driverDosName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if(driverHandle == INVALID_HANDLE_VALUE)
		{
			return 1;
		}
	}
	//����ͨ���¼�
	m_hCommEvent = CreateEvent(NULL, false, false, NULL);
	//����ͨ���¼�������
	DeviceIoControl(driverHandle, 
		IO_REFERENCE_EVENT, 
		(LPVOID)m_hCommEvent,
		0, 
		NULL, 
		0, 
		&bytesReturned, 
		NULL);

	while(1)
	{
		//������������ݰ����ˣ����ö�ȡ��־
		if (!PACKET_FILTER) continue;
		WaitForSingleObject(m_hCommEvent, INFINITE);
		memset(&attack_infor,0,sizeof(Attack_Infor));
		
		//��ȡ�µ���־
		ret=DeviceIoControl(driverHandle,
			IADS_NEWATTACK_EVENT,
			NULL,
			0,
			(LPVOID)&attack_infor,
			sizeof(Attack_Infor),
			&bytesReturned,
			NULL);//ͬ������,���������ؾ�����
		if(!ret) continue;

		//�澯��Ϣ
		in.S_un.S_addr=attack_infor.srcip;
		g_iDefenceTimes++;
		memset(tempbuf,0,100);
		if (attack_infor.flag==1)
			sprintf(tempbuf,"��ʾ��Ϣ:ϵͳ�ѳɹ�����%d�ι���! ԴIP:%s ������:%s ������:%s",g_iDefenceTimes
			,inet_ntoa(in),"ARP",attack_infor.code);
        else if(attack_infor.flag==2)
			 sprintf(tempbuf,"��ʾ��Ϣ:ϵͳ�ѳɹ�����%d�ι���! ԴIP:%s ������:%s ������:%s",g_iDefenceTimes
			,inet_ntoa(in),"TCP",attack_infor.code);
		else
			sprintf(tempbuf,"��ʾ��Ϣ:ϵͳ�ѳɹ�����%d�ι���! ԴIP:%s ������:%s ������:%s",g_iDefenceTimes
			,inet_ntoa(in),"UDP",attack_infor.code);

		//���������ص�����д����־���ݿ�
		WriteLogToMdb(tempbuf);
	   
		//״̬����ʾ
		PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)tempbuf);
		//ʵʱ��Ϣ��ʾ
		if(checknet)
			PostMessage(p_this->m_hWnd,UM_MESSLIST,(WPARAM)0,(LPARAM)tempbuf);
	}
	return 0;
}

//������־��Ϣд�����ݿ�
void WriteLogToMdb(char *logcontent)
{
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=log.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		if(pAdoRs->Open("iadslog", CADORecordset::openTable))
		{
			pAdoRs->AddNew();
			pAdoRs->SetFieldValue("log_content",_variant_t(logcontent));
			pAdoRs->Update();
			pAdoRs->Close();
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		MessageBox(0,"д����־ʧ��!","������������ϵͳ",MB_OK);
	}
	delete pAdoDb;
	return;
}

//��ʱ���´�������б�
void CALLBACK EXPORT UpdateLiveHost(
   HWND hWnd,     
   UINT nMsg, 
   UINT nIDEvent,
   DWORD dwTime
){
	p_this->KillTimer(gnIDEvent);//ֹͣ��ʱ��
	HANDLE PThreads[8];//�߳̾��
	DWORD dwThreadId;
	int i=0;
	while(!IsAllZero((BYTE *)&netcard[i],sizeof(Adapter)))
	{
		PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:����ɨ��������...");

		netcard[i].scanliveover=FALSE;       //����ɨ��δ����
		netcard[i].ipnow=netcard[i].ipstart; //ɨ����ʼIP
		//��������б� ɨ���־��λ
		ResetNodeFlag(netcard[i].livehostlist);
		PThreads[i]=CreateThread(NULL,
			0,
			ArpRequestToAll,
			(LPVOID)&netcard[i],
			0,
			&dwThreadId);
		i++;
	}
	for(;i;i--)
	{
		CloseHandle(PThreads[i-1]);//�ر��߳̾��
	}
	return;
}

//�򱾽ӿ����ڵľ����������л�������arp request
DWORD WINAPI ArpRequestToAll(
	LPVOID lpParameter   // thread data
)
{
	Adapter * adapter=(Adapter *)lpParameter ;//���ݰ����Ըýӿ�
	ARP_Packet arp_packet;
	memset((BYTE *)&arp_packet,0,sizeof(ARP_Packet));
	//destmac
	memset(arp_packet.dlcheader.desmac,0xFF,6);
    //srcmac
	memcpy(arp_packet.dlcheader.srcmac,adapter->mac,6);
	//ethertype
	arp_packet.dlcheader.ethertype=htons(0x0806);
	arp_packet.arpframe.flag=htons(0x01);
	arp_packet.arpframe.hw_addr_len=0x06;
	arp_packet.arpframe.prot_addr_len=0x04;
	arp_packet.arpframe.hw_type=htons(0x01);
	arp_packet.arpframe.prot_type=htons(0x0800);

	memcpy(arp_packet.arpframe.send_hw_addr,adapter->mac,6);
	arp_packet.arpframe.send_prot_addr=adapter->ipaddress;

	//�ڸ����ֽ��� ���뱾����ı�־
	arp_packet.arpframe.padding[0]='x';
	arp_packet.arpframe.padding[1]='d';
	while(adapter->ipnow<=adapter->ipend)
	{
		arp_packet.arpframe.targ_prot_addr=htonl(adapter->ipnow);
		sendpacket(adapter,(BYTE *)&arp_packet,sizeof(ARP_Packet));
		if((adapter->ipnow)>=(adapter->ipend))
		{
			int i=0;
			BOOL flag=TRUE;
			Sleep(2000);
			//���ӿڱ���ɨ�����
			adapter->scanliveover=TRUE;
			//���ӿڴ�������б�׼�����
			if(!adapter->hostlistok)
				adapter->hostlistok=TRUE;
			while(!IsAllZero((BYTE *)&netcard[i],sizeof(Adapter)))
			{
				if(!netcard[i].scanliveover) 
				{
					flag=FALSE;
					break;
				}
				i++;
			}
			//������нӿڶ�ɨ���������ָ���ʱ��
			if(flag) 
			{
				PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"��ʾ��Ϣ:ɨ�����������!");				
				p_this->SetTimer(gnIDEvent,UPDATE_LIVEHOST_INTERVAL,UpdateLiveHost);
			}
			::ExitThread(0);
			
		}
		(adapter->ipnow)++;
	}
	return 0;
}


//�ж������Ƿ���αװ�����б���
//����:   ip-Ҫ�жϵ�����IP(������)
//����ֵ: ����-TRUE,������-FALSE
BOOL IsInCheateHostList(unsigned int ip)
{
	CString csSql=_T("");
	struct in_addr in;

	//���αװ���в�������������Ѵ�
	if(PLAYALLROLE) return TRUE;

	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		//����IP�б�
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		in.S_un.S_addr=ip;
		csSql.Format("select * from host where host_ip='%s'",inet_ntoa(in));
		if(pAdoRs->Open(csSql, CADORecordset::openQuery))
		{
			if(!pAdoRs->IsEOF())
			{
				pAdoRs->Close();
				delete pAdoRs;
				pAdoDb->Close();
				delete pAdoDb;
				return TRUE;
			}
			pAdoRs->Close();
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
	}
	delete pAdoDb;
	return FALSE;
}

//�ж�αװ�����Ƿ����ICMP
//����:   ip-Ҫ�жϵ�����IP(������)
//����ֵ: ����-����TTLֵ,������-0
unsigned char IsAcceptIcmp(unsigned int ip)
{
	CString csSql=_T("");
	struct in_addr in;

	//���αװ���в�������������Ѵ�
	if(PLAYALLROLE) return 128;

	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		//����IP�б�
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		in.S_un.S_addr=ip;
		csSql.Format("select * from [host] where host_ip='%s'",inet_ntoa(in));
		if(pAdoRs->Open(csSql, CADORecordset::openQuery))
		{
			if(!pAdoRs->IsEOF())
			{
				int iIcmpAccept=0;
				CString szOs=_T("");
				int ttl=0;
				pAdoRs->GetFieldValue("host_icmp",iIcmpAccept);
				pAdoRs->GetFieldValue("host_os",szOs);
				if(iIcmpAccept)
				{
					pAdoRs->Close();
					//ȡ�ò���ϵͳ��Ӧ��TTLֵ
					csSql.Format("select * from [os] where os_name='%s'",szOs);
					if(pAdoRs->Open(csSql, CADORecordset::openQuery))
					{
						if(!pAdoRs->IsEOF())
						{
							pAdoRs->GetFieldValue("os_ttl",ttl);
						}
					}
					delete pAdoRs;
					pAdoDb->Close();
					delete pAdoDb;
					return (unsigned char)ttl;
				}
			}
			pAdoRs->Close();
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
	}
	delete pAdoDb;
	return 0;
}
//��ѯĳ�����������ĳ�˿��Ƿ񿪷�
//����:   ip- ip-Ҫ�жϵ�����IP(������) protocol-Э������ port-Ҫ��ѯ�Ķ˿�(������)
//����ֵ: �ҵ�-TRUE,δ�ҵ�-FALSE
BOOL IsPortOpen(unsigned int ip,unsigned char protocol,unsigned short port)
{
	CString csSql=_T("");
	struct in_addr in;
	
	//���αװ���в�������������Ѵ�
	if(PLAYALLROLE) return TRUE;

	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		in.S_un.S_addr=ip;
		csSql.Format("select * from [port] where port_ip='%s' and port_number=%d and port_protocol=%d and port_state=1",
			inet_ntoa(in),ntohs(port),protocol);
		if(pAdoRs->Open(csSql, CADORecordset::openQuery))
		{
			if(!pAdoRs->IsEOF())
			{
				pAdoRs->Close();
				delete pAdoRs;
				pAdoDb->Close();
				delete pAdoDb;
				return TRUE;
			}
			pAdoRs->Close();
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
	}
	delete pAdoDb;
	return FALSE;
}


//-------------------------------------------------------------------------
// PacketCheckSum
// �������ݰ���У���
// ����:packet-����������
//-------------------------------------------------------------------------
void PacketCheckSum(unsigned char packet[])
{
	Dlc_Header *pdlc_header=NULL; //��̫ͷָ��
	Ip_Header  *pip_header=NULL;  //IPͷָ��
	Udp_Header *pudp_header=NULL; //UDPͷָ��

	pdlc_header=(Dlc_Header *)packet;

	//�ж�ethertype,�������IP�����账��
	if(ntohs(pdlc_header->ethertype)!=0x0800) return;

	pip_header=(Ip_Header  *)(packet+14);
	//TCP��
	if(0x06==pip_header->proto)
	{
		//TCPͷ�Լ��������ݵ��ܳ���
		unsigned int attachsize=0;
		
		Tcp_Header *ptcp_header=NULL; //TCPͷָ��
		Tcp_Psd_Header *ptcp_psd_header=NULL;
		
		ptcp_header=(Tcp_Header *)(packet+14+((pip_header->ver_len)&15)*4);

		attachsize=ntohs(pip_header->total_len)-((pip_header->ver_len)&15)*4;
		ptcp_psd_header=(Tcp_Psd_Header *)malloc(attachsize+sizeof(Tcp_Psd_Header));
		memset(ptcp_psd_header,0,attachsize+sizeof(Tcp_Psd_Header));

		//���αTCPͷ
		ptcp_psd_header->destip=pip_header->destIP;
		ptcp_psd_header->sourceip=pip_header->sourceIP;
		ptcp_psd_header->mbz=0;
		ptcp_psd_header->ptcl=0x06;
		ptcp_psd_header->tcpl=htons(attachsize);

		ptcp_header->chksum=0;
		memcpy((unsigned char *)ptcp_psd_header+sizeof(Tcp_Psd_Header),
			(unsigned char *)ptcp_header,attachsize);
		
		
		ptcp_header->chksum=checksum((unsigned short *)ptcp_psd_header,
			attachsize+sizeof(Tcp_Psd_Header));
		
		//����ipͷ��У���
		pip_header->checksum=0;
		pip_header->checksum=checksum((unsigned short *)pip_header,((pip_header->ver_len)&15)*4);
		return;
	}
	return;
}

//�����Ѵ�������ݰ���
LRESULT CLocalPage::OnMessagePcount(WPARAM wParam, LPARAM lParam)
{
 	unsigned int uiPcount=0;
 	uiPcount=this->GetDlgItemInt(IDC_PCOUNT);
 	this->SetDlgItemInt(IDC_PCOUNT,++uiPcount);
	return 0;
}

//����ʵʱ��Ϣ�б���Ϣ
LRESULT CLocalPage::OnMessageListMessage(WPARAM wParam, LPARAM lParam)
{
	char timebuf[512]={0};

	SYSTEMTIME systime;
	int iIndex=0;
    static int number=0;

	if (strlen((LPCSTR)lParam)<5)  return 0;

	memset(&systime,0,sizeof(SYSTEMTIME));
	::GetLocalTime(&systime);


	sprintf(timebuf,"%04d-%02d-%02d %02d:%02d:%02d  ",
			systime.wYear,
			systime.wMonth,
			systime.wDay,
			systime.wHour,
			systime.wMinute,
			systime.wSecond);


	int total=((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->GetItemCount();
	((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertItem(total,timebuf);

	if(checknet==FALSE&&checkhost==TRUE)
	{	
		strcpy(timebuf,(LPCSTR)lParam);
	    int loc=0;
	    while(((LPCSTR)lParam)[loc]!='\t')
			loc++;
		loc++;
		memset(timebuf,0,sizeof(timebuf));
		strncpy(timebuf,(LPCSTR)lParam,loc-1);
        ((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->SetItemText(total, 1,timebuf);

		int index=loc;	
		while(((LPCSTR)lParam)[index]!='\t')
			index++;
		index++;
		memset(timebuf,0,sizeof(timebuf));
		strncpy(timebuf,&((LPCSTR)lParam)[loc],index-1-loc);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->SetItemText(total, 2,timebuf);

		memset(timebuf,0,sizeof(timebuf));
		strcpy(timebuf,&((LPCSTR)lParam)[index]);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->SetItemText(total, 3,timebuf);
		
	}else 
	{
	    int i=0;
		while(((LPCSTR)lParam)[i]==' '||((LPCSTR)lParam)[i]=='\t')
			i++;
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->SetItemText(total, 1,&((LPCSTR)lParam)[i]);
	}
    return 0;
}

void CLocalPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
}

void CLocalPage::OnRclickMeslist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CRect rect;
	GetWindowRect(&rect);

	
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	int x=point.x+rect.left;
	int y=point.y+rect.top;


	CMenu *m_PopMenu=new CMenu;
	m_PopMenu->LoadMenu(IDR_MESLIST);//���ز˵�
 
	if(checknet)
		m_PopMenu->GetSubMenu(0)->CheckMenuItem(0,MF_CHECKED|MF_BYPOSITION);
	else
		m_PopMenu->GetSubMenu(0)->CheckMenuItem(0,MF_UNCHECKED|MF_BYPOSITION);
	if(checkhost)
		m_PopMenu->GetSubMenu(0)->CheckMenuItem(1,MF_CHECKED|MF_BYPOSITION);
	else
		m_PopMenu->GetSubMenu(0)->CheckMenuItem(1,MF_UNCHECKED|MF_BYPOSITION);

	TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);

	*pResult = 0;
}


void CLocalPage::OnDelete() 
{
	if(!((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->GetSelectedCount())
	{
		return;
	}
	POSITION pos =((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->GetFirstSelectedItemPosition();
	int index;
	while(pos)
	{
		index=((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->GetNextSelectedItem(pos);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->DeleteItem(index);

	}
}

void CLocalPage::OnDeleteall() 
{
	if(IDYES==AfxMessageBox("ȷ��������м����Ϣ��",MB_ICONQUESTION|MB_YESNO,0))
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->DeleteAllItems();
	}
}

void CLocalPage::UpdateTitle()
{
	//��ɾ����������
	int nColumnCount = ((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->GetHeaderCtrl()->GetItemCount();
	for (int i=0;i < nColumnCount;i++)
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->DeleteColumn(0);
	}
	if(checkhost&&checknet)
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "ʱ��",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "�����Ϣ",LVCFMT_LEFT,600);
	}else if(checknet)
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "ʱ��",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "��������Ϣ",LVCFMT_LEFT,600);
	}else if(checkhost)
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "ʱ��",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "������",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(2, "������",LVCFMT_LEFT,150);
	    ((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(3, "·��",LVCFMT_LEFT,400);
	}else
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "ʱ��",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "�����Ϣ",LVCFMT_LEFT,600);
	}
}

void CLocalPage::OnNetshow() 
{
	checknet=!checknet;
	((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->DeleteAllItems();
    UpdateTitle();
}

void CLocalPage::OnHostshow() 
{
	
	checkhost=!checkhost;	
	((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->DeleteAllItems();
	UpdateTitle();
}


HBRUSH CLocalPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
 {
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor); 
 }

//��ȡͳ����Ϣ
void CLocalPage::GetStatistics()
{
	if(PACKET_FILTER)
	{
		this->SetDlgItemText(IDC_PACKETFILTER,"�ѿ���");
	}
	else
	{
		this->SetDlgItemText(IDC_PACKETFILTER,"δ����");
	}
	if(ANTIARP)
	{
		this->SetDlgItemText(IDC_ARPFILTER,"�ѿ���");
	}
	else
	{
		this->SetDlgItemText(IDC_ARPFILTER,"δ����");
	}
	if(ICMP_FILTER)
	{
		this->SetDlgItemText(IDC_ICMPFILTER,"�ѿ���");
	}
	else
	{
		this->SetDlgItemText(IDC_ICMPFILTER,"δ����");
	}
	return;
}

//��ȡ�����������
void GetShellCodeCount()
{
	//��ȡ����������
	unsigned int uiCodeCount=0;
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=code.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		if(pAdoRs->Open("shellcode", CADORecordset::openTable))
		{
			uiCodeCount=pAdoRs->GetRecordCount();
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	delete pAdoDb;
	p_this->SetDlgItemInt(IDC_SCCOUNT,uiCodeCount);
}

