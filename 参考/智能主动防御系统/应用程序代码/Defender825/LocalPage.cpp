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

//要连接的库
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"ws2_32.lib")


BOOL checknet=TRUE;
BOOL checkhost=TRUE;

//设备驱动句柄
HANDLE driverHandle=NULL;	
LPSTR driverDosName=NULL;

UINT gnIDEvent=1;//定时器标识

//过滤开关
UCHAR ICMP_FILTER=1;   //是否允许Ping入
UCHAR PACKET_FILTER=1; //是否进行数据包过滤
UCHAR ANTIARP=1;       //是否启用ARP防火墙
UCHAR PLAYALLROLE=1;   //是否伪装成所有不存活的主机

Adapter netcard[8];//网络适配器

CLocalPage *p_this=NULL;


//列表框指针
CComboBox *p_listAdapter=NULL;
HANDLE m_hNewAttactEvent;
HANDLE m_hCommEvent;

//拦截的攻击次数
UINT g_iDefenceTimes=0;

BOOL bUseCodeDb=FALSE;

//状态栏消息
#define UM_STATUS (WM_USER +12)    //自定义状态栏消息
#define UM_MESSLIST (WM_USER +13)  //实时信息列表消息
#define UM_PCOUNT (WM_USER +15)    //更新已过滤的数据包数消息

//程序当前路径
extern char szCurrentDirectory[256];

//获取网卡相关信息
int GetAdapterInfor(Adapter *adapter);
//判断一块缓冲区是否为全0:全零返回TRUE,非全零返回FALSE
BOOL IsAllZero(unsigned char dat[],int length);
//设置过滤规则(驱动)
int SetFilterRules(UCHAR icmp_filter,UCHAR packet_filter,UCHAR antiarp);
//开始监听
void startlisten();
//监听线程函数
DWORD WINAPI ListenThreadFun( LPVOID lpParameter );
//定时更新存活主机列表
void CALLBACK EXPORT UpdateLiveHost(HWND hWnd,UINT nMsg,UINT nIDEvent,DWORD dwTime );
//向本接口所在的局域网中所有机器发送arp请求
DWORD WINAPI ArpRequestToAll(LPVOID lpParameter );
//数据包处理
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
//发送 数据包
void sendpacket(Adapter * adapter,BYTE * packet,int size);
//发送网关信息(iP,mac)到驱动程序
int SendNetGateInfor(unsigned int netgateip,unsigned char netgatemac[6]);
//开启读取驱动发送的攻击日志信息线程
void StartReadAttackLog();
//读取驱动发送的攻击日志信息
DWORD WINAPI ReadAttackLog(LPVOID lpParameter);
//将收到的新日志信息写入注册表中
void WriteLogToMdb(char *logcontent);
//添加特征到数据库，并将新的特征发送给驱动程序
void InsertDB(char *shellcode);
//判断主机是否在伪装主机列表中
//参数:   ip-要判断的主机IP(网络序)
//返回值: 存在-TRUE,不存在-FALSE
BOOL IsInCheateHostList(unsigned int ip);
//查询某不存活主机的某端口是否开放
//参数:   ip- ip-要判断的主机IP(网络序) protocol-协议类型 port-要查询的端口(网络序)
//返回值: 找到-TRUE,未找到-FALSE
BOOL IsPortOpen(unsigned int ip,unsigned char protocol,unsigned short port);
//判断伪装主机是否接受ICMP
//参数:   ip-要判断的主机IP(网络序)
//返回值: 开放-返回TTL值,不开放-0
unsigned char IsAcceptIcmp(unsigned int ip);
//计算TCP数据包校验和
void PacketCheckSum(unsigned char packet[]);
//获取特征码的数量
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
	

	//初始化列表
	ListView_SetExtendedListViewStyle(
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->m_hWnd, 
		LVS_EX_GRIDLINES  | LVS_EX_FULLROWSELECT
		); 
	
	
	((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "时间",LVCFMT_LEFT,150);

	((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "监控信息",LVCFMT_LEFT,600);


	//设置过滤规则
    SetFilterRules(ICMP_FILTER,PACKET_FILTER,ANTIARP);
	memset(netcard,0,sizeof(Adapter) *8);
	//获取网卡相关信息
	if(GetAdapterInfor(&netcard[0]))
	{
		AfxMessageBox("获取网卡信息失败!");
		PostQuitMessage(0);
	}else
	{
		m_listAdapter.SetCurSel(0);
		OnSelchangeAdapterList();
	}
	//启动监听线程
	startlisten();	
	gnIDEvent=1; //计时器表示符
	//启动定时更新存活主机列表定时器
	this->SetTimer(gnIDEvent,50,UpdateLiveHost);
	//开启读取驱动发送的攻击日志信息线程
	StartReadAttackLog();
	//显示统计信息-特征码数量
	GetShellCodeCount();
	return TRUE; 
}

//显示该网络接口信息
void CLocalPage::OnSelchangeAdapterList() 
{
	Adapter * padapter=NULL;
	int i=0;
	char tempbuf[256];
	struct in_addr in;
	//获取当前网卡名称
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
	//显示接口信息
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
	//获取统计信息
	GetStatistics();
}

//获取网卡相关信息
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
	// 第一次调用GetAdapterInfo获取ulOutBufLen大小
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

				//网段内主机数
				iplen=0xFFFFFFFF-ntohl(adapter->netmask);
				//起始IP
				adapter->ipstart=(ntohl(adapter->ipaddress) & ntohl(adapter->netmask))+1;
				//结束IP
				adapter->ipend=adapter->ipstart+iplen-2;
				
				//如果不是拨号上网接口
				if(adapter->ipstart<adapter->ipend)
				{
					//adapter name
					strcpy(adapter->name,"rpcap://\\Device\\NPF_");
					strcat(adapter->name,pAdapter->AdapterName);
					p_listAdapter->AddString(adapter->name);
					//mac
					memcpy(adapter->mac,pAdapter->Address,6);
					//将本机ip和mac发送给驱动程序
					SendNetGateInfor(adapter->ipaddress,adapter->mac);
					// livehostlist 初始化存活主机列表
					CreateCollection(adapter->livehostlist);			
					adapter++;
					i++;
				}
				else
					//重置
					memset(adapter,0,sizeof(Adapter));
			}
			pAdapter = pAdapter->Next;
		}
	}
	else
		return 1;
	return 0;
}



//判断一块缓冲区是否为全0
//全零返回TRUE,非全零返回FALSE
BOOL IsAllZero(unsigned char dat[],int length)
{
	int i=0;
	for(i=0;i<length;i++)
	{
		if(dat[i]) return FALSE;
	}
	return TRUE;
}


//设置过滤规则
int SetFilterRules(UCHAR icmp_filter,UCHAR packet_filter,UCHAR antiarp)
{
	unsigned char inBuffer[2]={0};
	DWORD bytesReturned=0;
	BOOL ret=0;

	if(!driverHandle)//如果驱动设备没有打开
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
			AfxMessageBox("打开驱动设备失败!");
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
		NULL);//同步进行
	if(!ret) return 1;
	inBuffer[0]=packet_filter;
	ret=DeviceIoControl(driverHandle,
		IADS_PACKET_FILTER,
		inBuffer,
		2,
		NULL,
		0,
		&bytesReturned,
		NULL);//同步进行
	if(!ret) return 1;
	inBuffer[0]=antiarp;
	ret=DeviceIoControl(driverHandle,
		IADS_ANTIARP,
		inBuffer,
		2,
		NULL,
		0,
		&bytesReturned,
		NULL);//同步进行
	if(!ret) return 1;
	return 0;
}



void startlisten()
{
	// 开始监听本机所有的网络接口
	HANDLE PThreads[8];
	DWORD dwThreadId;
	int i=0;
	while(!IsAllZero((BYTE *)&netcard[i],sizeof(Adapter)))
	{
		//PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"提示信息:正在建立监听线程...");
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
	//PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"提示信息:监听线程建立完成!");
}

//监听线程函数
DWORD WINAPI ListenThreadFun(
  LPVOID lpParameter // thread data
  )
{
	Adapter *adapter=(Adapter *)lpParameter;
	char errbuf[PCAP_ERRBUF_SIZE];
    //打开适配器 
	if(!adapter->fp)//如果该接口没有打开过
	{
		if ((adapter->fp= pcap_open(adapter->name,  // 设备名
			65536,                                  // 要捕捉的数据包的部分 
			PCAP_OPENFLAG_PROMISCUOUS,              // 混杂模式
			1000,                                   // 读取超时时间
			NULL,                                   // 远程机器验证
			errbuf                                  // 错误缓冲池
			)) == NULL)
		{
			PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"提示信息:打开适配器出错!");
			return 0;
		}
	}
	pcap_loop(adapter->fp, 0, packet_handler, (unsigned char *)lpParameter);
	return 0;
}

//数据包处理
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{

	Adapter * adapter=(Adapter *)param;//数据包来自该接口
	
	//已处理数据包数+1
	if(PACKET_FILTER)
		PostMessage(p_this->m_hWnd,UM_PCOUNT,(WPARAM)0,(LPARAM)0);
	
	if(adapter->scanliveover) //扫描该接口存活主机结束
	{
		//删除所有失活主机
		DeleteFlagZNode(adapter->livehostlist);
	}

	Dlc_Header *dlc_header;
	dlc_header=(Dlc_Header *)pkt_data;
	
	switch(ntohs(dlc_header->ethertype))//判断是IP包还是ARP包
	{
	case 0x0806://arp包
		{
			Arp_Frame *arp_frame;
			arp_frame=(Arp_Frame *)(pkt_data + sizeof(Dlc_Header));
		
			//如果dest ip与本机不在同一网段则不予处理
			if(ntohl(arp_frame->targ_prot_addr)<adapter->ipstart
			   ||ntohl(arp_frame->targ_prot_addr)>adapter->ipend
			   )  return;

			if(ntohs(arp_frame->flag)==0x01)//arp request
			{
				//开机arp广播 srcip=destip
				if(arp_frame->send_prot_addr==arp_frame->targ_prot_addr)
				{
					InsertColletion(adapter->livehostlist,arp_frame->send_prot_addr);
					return;
				}
			
				if(adapter->hostlistok)//如果存活主机列表已准备好
				{
					//如果请求的是不存活主机(ip),并且不是本程序的其它实例所发,并且不是发给网关的
					//并且伪装主机配置文件中有该主机，则给予虚假应答
					if(!FindNode(adapter->livehostlist,arp_frame->targ_prot_addr)
						&&(arp_frame->padding)[0]!='x'
						&&(arp_frame->padding)[1]!='d'
						&&arp_frame->targ_prot_addr!=adapter->netgate
						&&IsInCheateHostList(arp_frame->targ_prot_addr))
					{	
						struct in_addr in,in2;
						char szMesBuf[256]={0};

						//状态栏显示
						PostMessage(AfxGetMainWnd()->m_hWnd,UM_STATUS,
							(WPARAM)0,(LPARAM)"提示信息:系统发现向不存活主机发起的ARP请求,给予了虚假应答!");

						in.S_un.S_addr=arp_frame->send_prot_addr;						
						in2.S_un.S_addr=arp_frame->targ_prot_addr;
						strcpy(szMesBuf,"系统发现主机");
						strcat(szMesBuf,inet_ntoa(in));
						strcat(szMesBuf,"向不存活主机");
						strcat(szMesBuf,inet_ntoa(in2));
						strcat(szMesBuf,"发出ARP请求，并给予了虚假应答!");
                 
						//实时信息栏显示
						if(checknet)
						   PostMessage(p_this->m_hWnd,UM_MESSLIST,(WPARAM)0,(LPARAM)szMesBuf);
			

						//构造应答数据包
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
						
						//在附加字节区 加入本程序的标志
						arp_packet.arpframe.padding[0]='x';
						arp_packet.arpframe.padding[1]='d';
						
						//发送应答
						sendpacket(adapter,(BYTE *)&arp_packet,sizeof(ARP_Packet));
						return;
				  }
				}
				return;
			}
			else if(ntohs(arp_frame->flag)==0x02)//arp response
			{
				//如果是发给本机的应答
				if(arp_frame->targ_prot_addr==adapter->ipaddress)
				{
					//如果是网关的应答，则获取其mac地址
					if(arp_frame->send_prot_addr==adapter->netgate
						&&!adapter->netgatemac[0]
						&&!adapter->netgatemac[1]
						&&!adapter->netgatemac[2]
						&&!adapter->netgatemac[3]
						&&!adapter->netgatemac[4]
						&&!adapter->netgatemac[5])
					{
						//在窗体上显示网关MAC地址
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
						//发送网关信息给驱动程序
						SendNetGateInfor(arp_frame->send_prot_addr,arp_frame->send_hw_addr);
					}
					InsertColletion(adapter->livehostlist,arp_frame->send_prot_addr);
					return;
				}
			    return;
			}
			break;
		}
	case 0x0800://ip包
		{
			Ip_Header *ip_header;
			ip_header=(Ip_Header *)(pkt_data + sizeof(Dlc_Header));
		
			//只处理dest mac为本接口而dest ip非本接口的数据包
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
					//查询配置文件
					ucTtl=IsAcceptIcmp(picmp_packet->ip_header.destIP);
					if(!ucTtl) return;

					//构造icmp包
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
					BYTE syn=0,ack=0;//同步确认标志位
					Tcp_Header *ptcp_header;
					ptcp_header=(Tcp_Header *)(pkt_data+14+20);//tcp头
					
					//目的端口是否开放
					if(!IsPortOpen(ip_header->destIP,0x06,ptcp_header->dstport)) return;

					//提取syn标志位
					if(((ptcp_header->flags) & 2)==2) 
						syn=1;
					else 
						syn=0;
					//提取ack标志位
					if(((ptcp_header->flags) & 16)==16)
						ack=1;
					else
						ack=0;

					//回应第二次握手
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

						//计算校验和
						PacketCheckSum(tcpbuf);						
						//发送
						sendpacket(adapter,(BYTE *)tcpbuf,66);
						return;
					}
					else if(ack==1&&syn==0)
					{
						//两次握手后客户端发送的数据包
						BYTE * pscode;//数据包特征部分
						int codelen;//特征数据长度
						codelen=(ntohs(ip_header->total_len))-((ip_header->ver_len) & 15)*4-((ptcp_header->dataoff)>>4)*4;
						//如果附加数据太短则不处理
						if(codelen<=10) return;
						pscode=(BYTE *)malloc(codelen);
						memcpy(pscode,pkt_data+14+((ip_header->ver_len) & 15)*4+((ptcp_header->dataoff)>>4)*4,codelen);
						
						//如果是发给80端口
						if(80==ntohs(ptcp_header->dstport))
						{
							//取得http报文(即pscode)
							if(!strncmp((char *)pscode,"GET",3)||!strncmp((char *)pscode,"POST",4))
							{
								char szCrlf[5]={'\x0d','\x0a','\x0d','\x0a',0};
								char *pCrlf=NULL;
								pCrlf=strstr((char *)pscode,szCrlf);
								//如果HTTP报文中有附加数据
								if(pCrlf)
								{
									if((unsigned char)*(pCrlf+4))
									{
										//计算特征数据的摘要值
										MD5_CTX  cmd5;
										char result[33];
										cmd5.MD5Update(pscode,codelen);
										cmd5.MD5Final(result);
										//将计算的特征值结果存入数据库
										InsertDB(result);
									}
								}
								//应答HTTP请求
								unsigned int scriptlen=0;
								unsigned char *scriptbuf=NULL;
								unsigned char *tcpbuf=NULL;
								unsigned int packetsize=0;
								CFile cfile; 
								if(cfile.Open("iisresponse.scri",CFile::modeRead))
								{
									scriptlen=cfile.GetLength();
									//为读取脚本文件分配内存
									scriptbuf=(unsigned char *)malloc(scriptlen);						
									//分配内存失败
									if(!scriptbuf)
									{
										cfile.Close();
										return;
									}
									memset(scriptbuf,0,scriptlen);
									//读取脚本文件内容
									cfile.Read(scriptbuf,scriptlen);
	
									//计算应答数据包的总长度
									packetsize=scriptlen+14+((ip_header->ver_len) & 15)*4+((ptcp_header->dataoff)>>4)*4+4;
									//为应答数据包分配内存
									tcpbuf=(unsigned char *)malloc(packetsize);
									if(!tcpbuf)
									{
										cfile.Close();
										return;
									}
									
									memset(tcpbuf,0,packetsize);

									//将请求报文的协议头部分拷贝过来
									memcpy(tcpbuf,pkt_data,14+((ip_header->ver_len) & 15)*4+((ptcp_header->dataoff)>>4)*4);
									
									//复制应答HTTP报文
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
									
									//计算校验和
									PacketCheckSum(tcpbuf);						
									//发送
									sendpacket(adapter,(BYTE *)tcpbuf,packetsize);
								}
								cfile.Close();
								return;
							}
						}
						//计算特征数据的摘要值
						MD5_CTX  cmd5;
						char result[33]={0};
						cmd5.MD5Update(pscode,codelen);
						cmd5.MD5Final(result);
						//将计算的特征值结果存入数据库
						InsertDB(result);
						return;
					}
					break;
				}
			case 0x11: //udp
				{
					Udp_Header *pudp_header=NULL;
					pudp_header=(Udp_Header *)(pkt_data+14+((ip_header->ver_len) & 15)*4);//udp头
					
					//目的端口是否开放
					if(!IsPortOpen(ip_header->destIP,0x11,pudp_header->dstport)) return;

					BYTE * pscode=NULL;//数据包特征部分
					int codelen;//特征数据长度
					codelen=(ntohs(ip_header->total_len))-((ip_header->ver_len) & 15)*4-8;
					

					//如果附加数据太短则不处理
					if(codelen<=10) return;

					pscode=(BYTE *)malloc(codelen);
					if(!pscode) return;

					memset(pscode,0,sizeof(BYTE)*codelen);
					memcpy(pscode,pkt_data+14+((ip_header->ver_len) & 15)*4+8,codelen);
					
					//计算特征数据的摘要值
					
					MD5_CTX  cmd5;
					char result[33]={0};
					cmd5.MD5Update(pscode,codelen);
					cmd5.MD5Final(result);
					
					//将计算的特征值结果存入数据库
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
	default://其它包，暂不处理
		return;
	}
	return;
}

//发送 数据包
void sendpacket(Adapter * adapter,BYTE * packet,int size)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	//打开适配器
	if(!adapter->fp) //如果该接口没有被打开过
	{
		if ((adapter->fp= pcap_open(adapter->name,  // 设备名
			100,                               // 要捕捉的数据包的部分 
			PCAP_OPENFLAG_PROMISCUOUS,         // 混杂模式
			1000,                              // 读取超时时间
			NULL,                              // 远程机器验证
			errbuf                             // 错误缓冲池
			)) == NULL)
		{
			PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"提示信息:打开适配器出错!");
			return;
		}	
	}
	//发送arp request数据包
	 if (pcap_sendpacket(adapter->fp, (BYTE *)packet, size) != 0)
    {
		 PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"提示信息:发送数据包出错!");
    }
    return;
}


//发送网关信息(iP,mac)到驱动程序
//参数:(netgateip,网关ip,netgatemac,网关mac)
//返回值：成功返回0，失败返回1
int SendNetGateInfor(unsigned int netgateip,unsigned char netgatemac[6])
{
	HostInfor hostinfor;
	BOOL ret=0;
	DWORD bytesReturned=0;
	memset(&hostinfor,0,sizeof(HostInfor));
	hostinfor.ip=netgateip;
	memcpy(hostinfor.mac,netgatemac,6);
	if(!driverHandle)//如果驱动设备没有打开
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
			AfxMessageBox("打开驱动设备失败!");
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
		NULL //同步进行
		);
	if(!ret) return 1;
	 return 0;

}


//添加特征到数据库，并将新的特征发送给驱动程序
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
			
			if(pAdoRs->IsEOF()) //新特征码
			{
				pAdoRs->AddNew();
				pAdoRs->SetFieldValue("shellcode",_variant_t(shellcode));
				pAdoRs->Update();
				pAdoRs->Close();
			
				if(driverHandle)
				{
					//将新的特征发送给驱动程序
					DeviceIoControl(driverHandle,
						IADS_DRIVER_ADDCODE,
						(LPVOID)shellcode,
						32,
						NULL,
						0,
						&bytesReturned,
						NULL //同步进行
						);
				}
				//状态栏显示
				PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"提示信息:发现新的特征，已入特征库!");		
				//实时信息显示
				if(checknet)
					PostMessage(p_this->m_hWnd,UM_MESSLIST,(WPARAM)0,(LPARAM)"提示信息:发现新的特征，已入特征库!");
				
				//统计信息 特征码数量+1
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
		MessageBox(0,"写入特征库失败!","智能主动防御系统",MB_OK);
	}
	delete pAdoDb;
	bUseCodeDb=FALSE;
	return;
}

//开启读取驱动发送的攻击日志信息线程
void StartReadAttackLog()
{
	HANDLE PThread;//线程句柄
	DWORD dwThreadId;
	PThread=CreateThread(NULL,
		0,
		ReadAttackLog,
		NULL,
		0,
		&dwThreadId);
	CloseHandle(PThread);//关闭线程句柄
	return;
}
//读取驱动发送的攻击日志信息
DWORD WINAPI ReadAttackLog(
	LPVOID lpParameter   // thread data
)
{
	DWORD bytesReturned=0;
	BOOL ret=0;
	Attack_Infor attack_infor;
	struct in_addr in;
	char tempbuf[100];

	if(!driverHandle)//如果驱动设备没有打开
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
	//创建通信事件
	m_hCommEvent = CreateEvent(NULL, false, false, NULL);
	//传递通信事件到驱动
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
		//如果不进行数据包过滤，则不用读取日志
		if (!PACKET_FILTER) continue;
		WaitForSingleObject(m_hCommEvent, INFINITE);
		memset(&attack_infor,0,sizeof(Attack_Infor));
		
		//读取新的日志
		ret=DeviceIoControl(driverHandle,
			IADS_NEWATTACK_EVENT,
			NULL,
			0,
			(LPVOID)&attack_infor,
			sizeof(Attack_Infor),
			&bytesReturned,
			NULL);//同步进行,驱动不返回就阻塞
		if(!ret) continue;

		//告警信息
		in.S_un.S_addr=attack_infor.srcip;
		g_iDefenceTimes++;
		memset(tempbuf,0,100);
		if (attack_infor.flag==1)
			sprintf(tempbuf,"提示信息:系统已成功拦截%d次攻击! 源IP:%s 包类型:%s 特征码:%s",g_iDefenceTimes
			,inet_ntoa(in),"ARP",attack_infor.code);
        else if(attack_infor.flag==2)
			 sprintf(tempbuf,"提示信息:系统已成功拦截%d次攻击! 源IP:%s 包类型:%s 特征码:%s",g_iDefenceTimes
			,inet_ntoa(in),"TCP",attack_infor.code);
		else
			sprintf(tempbuf,"提示信息:系统已成功拦截%d次攻击! 源IP:%s 包类型:%s 特征码:%s",g_iDefenceTimes
			,inet_ntoa(in),"UDP",attack_infor.code);

		//将驱动返回的数据写入日志数据库
		WriteLogToMdb(tempbuf);
	   
		//状态栏提示
		PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)tempbuf);
		//实时信息显示
		if(checknet)
			PostMessage(p_this->m_hWnd,UM_MESSLIST,(WPARAM)0,(LPARAM)tempbuf);
	}
	return 0;
}

//将新日志信息写入数据库
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
		MessageBox(0,"写入日志失败!","智能主动防御系统",MB_OK);
	}
	delete pAdoDb;
	return;
}

//定时更新存活主机列表
void CALLBACK EXPORT UpdateLiveHost(
   HWND hWnd,     
   UINT nMsg, 
   UINT nIDEvent,
   DWORD dwTime
){
	p_this->KillTimer(gnIDEvent);//停止计时器
	HANDLE PThreads[8];//线程句柄
	DWORD dwThreadId;
	int i=0;
	while(!IsAllZero((BYTE *)&netcard[i],sizeof(Adapter)))
	{
		PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"提示信息:正在扫描存活主机...");

		netcard[i].scanliveover=FALSE;       //本次扫描未结束
		netcard[i].ipnow=netcard[i].ipstart; //扫描起始IP
		//存活主机列表 扫描标志复位
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
		CloseHandle(PThreads[i-1]);//关闭线程句柄
	}
	return;
}

//向本接口所在的局域网中所有机器发送arp request
DWORD WINAPI ArpRequestToAll(
	LPVOID lpParameter   // thread data
)
{
	Adapter * adapter=(Adapter *)lpParameter ;//数据包来自该接口
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

	//在附加字节区 加入本程序的标志
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
			//本接口本次扫描结束
			adapter->scanliveover=TRUE;
			//本接口存活主机列表准备完毕
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
			//如果所有接口都扫描结束，则恢复定时器
			if(flag) 
			{
				PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+12,(WPARAM)0,(LPARAM)"提示信息:扫描存活主机完毕!");				
				p_this->SetTimer(gnIDEvent,UPDATE_LIVEHOST_INTERVAL,UpdateLiveHost);
			}
			::ExitThread(0);
			
		}
		(adapter->ipnow)++;
	}
	return 0;
}


//判断主机是否在伪装主机列表中
//参数:   ip-要判断的主机IP(网络序)
//返回值: 存在-TRUE,不存在-FALSE
BOOL IsInCheateHostList(unsigned int ip)
{
	CString csSql=_T("");
	struct in_addr in;

	//如果伪装所有不存活主机开关已打开
	if(PLAYALLROLE) return TRUE;

	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		//加载IP列表
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
		::MessageBox(0,"打开数据库失败!","高级设置",MB_OK);
	}
	delete pAdoDb;
	return FALSE;
}

//判断伪装主机是否接受ICMP
//参数:   ip-要判断的主机IP(网络序)
//返回值: 开放-返回TTL值,不开放-0
unsigned char IsAcceptIcmp(unsigned int ip)
{
	CString csSql=_T("");
	struct in_addr in;

	//如果伪装所有不存活主机开关已打开
	if(PLAYALLROLE) return 128;

	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		//加载IP列表
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
					//取得操作系统对应的TTL值
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
		::MessageBox(0,"打开数据库失败!","高级设置",MB_OK);
	}
	delete pAdoDb;
	return 0;
}
//查询某不存活主机的某端口是否开放
//参数:   ip- ip-要判断的主机IP(网络序) protocol-协议类型 port-要查询的端口(网络序)
//返回值: 找到-TRUE,未找到-FALSE
BOOL IsPortOpen(unsigned int ip,unsigned char protocol,unsigned short port)
{
	CString csSql=_T("");
	struct in_addr in;
	
	//如果伪装所有不存活主机开关已打开
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
		::MessageBox(0,"打开数据库失败!","高级设置",MB_OK);
	}
	delete pAdoDb;
	return FALSE;
}


//-------------------------------------------------------------------------
// PacketCheckSum
// 计算数据包的校验和
// 参数:packet-待处理数据
//-------------------------------------------------------------------------
void PacketCheckSum(unsigned char packet[])
{
	Dlc_Header *pdlc_header=NULL; //以太头指针
	Ip_Header  *pip_header=NULL;  //IP头指针
	Udp_Header *pudp_header=NULL; //UDP头指针

	pdlc_header=(Dlc_Header *)packet;

	//判断ethertype,如果不是IP包则不予处理
	if(ntohs(pdlc_header->ethertype)!=0x0800) return;

	pip_header=(Ip_Header  *)(packet+14);
	//TCP包
	if(0x06==pip_header->proto)
	{
		//TCP头以及附加数据的总长度
		unsigned int attachsize=0;
		
		Tcp_Header *ptcp_header=NULL; //TCP头指针
		Tcp_Psd_Header *ptcp_psd_header=NULL;
		
		ptcp_header=(Tcp_Header *)(packet+14+((pip_header->ver_len)&15)*4);

		attachsize=ntohs(pip_header->total_len)-((pip_header->ver_len)&15)*4;
		ptcp_psd_header=(Tcp_Psd_Header *)malloc(attachsize+sizeof(Tcp_Psd_Header));
		memset(ptcp_psd_header,0,attachsize+sizeof(Tcp_Psd_Header));

		//填充伪TCP头
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
		
		//计算ip头的校验和
		pip_header->checksum=0;
		pip_header->checksum=checksum((unsigned short *)pip_header,((pip_header->ver_len)&15)*4);
		return;
	}
	return;
}

//更新已处理的数据包数
LRESULT CLocalPage::OnMessagePcount(WPARAM wParam, LPARAM lParam)
{
 	unsigned int uiPcount=0;
 	uiPcount=this->GetDlgItemInt(IDC_PCOUNT);
 	this->SetDlgItemInt(IDC_PCOUNT,++uiPcount);
	return 0;
}

//处理实时信息列表消息
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
	m_PopMenu->LoadMenu(IDR_MESLIST);//加载菜单
 
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
	if(IDYES==AfxMessageBox("确认清除所有监控信息？",MB_ICONQUESTION|MB_YESNO,0))
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->DeleteAllItems();
	}
}

void CLocalPage::UpdateTitle()
{
	//先删除所有列名
	int nColumnCount = ((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->GetHeaderCtrl()->GetItemCount();
	for (int i=0;i < nColumnCount;i++)
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->DeleteColumn(0);
	}
	if(checkhost&&checknet)
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "时间",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "监控信息",LVCFMT_LEFT,600);
	}else if(checknet)
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "时间",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "网络监控信息",LVCFMT_LEFT,600);
	}else if(checkhost)
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "时间",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "进程名",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(2, "函数名",LVCFMT_LEFT,150);
	    ((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(3, "路径",LVCFMT_LEFT,400);
	}else
	{
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(0, "时间",LVCFMT_LEFT,150);
		((CListCtrl *)this->GetDlgItem(IDC_MESLIST))->InsertColumn(1, "监控信息",LVCFMT_LEFT,600);
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

//获取统计信息
void CLocalPage::GetStatistics()
{
	if(PACKET_FILTER)
	{
		this->SetDlgItemText(IDC_PACKETFILTER,"已开启");
	}
	else
	{
		this->SetDlgItemText(IDC_PACKETFILTER,"未开启");
	}
	if(ANTIARP)
	{
		this->SetDlgItemText(IDC_ARPFILTER,"已开启");
	}
	else
	{
		this->SetDlgItemText(IDC_ARPFILTER,"未开启");
	}
	if(ICMP_FILTER)
	{
		this->SetDlgItemText(IDC_ICMPFILTER,"已开启");
	}
	else
	{
		this->SetDlgItemText(IDC_ICMPFILTER,"未开启");
	}
	return;
}

//获取特征码的数量
void GetShellCodeCount()
{
	//获取特征码数量
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

