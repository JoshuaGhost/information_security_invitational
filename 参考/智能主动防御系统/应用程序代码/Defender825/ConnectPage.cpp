// ConnectPage.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "ConnectPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include   "psapi.h" 

//获取的进程及其模块路径
//total=1:只获得进程路径
//total返回实际的路径数目

extern BOOL EnableDebugPrivilege();

//////////获得网络连接信息

#include <stdio.h>
#include <windows.h>
#include <iprtrmib.h>
#include <tlhelp32.h>
#include <iphlpapi.h>
#include  <winsock.h>

#pragma comment(lib, "ws2_32.lib")

   
//   Maximum   string   lengths   for   ASCII   ip   address   and   port   names  //  
#define   HOSTNAMELEN   256  
#define   PORTNAMELEN   256  
#define   ADDRESSLEN   HOSTNAMELEN+PORTNAMELEN  
//  
//   Our   option   flags  
//  
#define   FLAG_ALL_ENDPOINTS   1  
#define   FLAG_SHOW_NUMBERS   2  





//  
//   Undocumented   extended   information   structures   available    
//   only   on   XP   and   higher     
typedef   struct   tagMIB_TCPEXROW
{
	DWORD   dwState;   //   state   of   the   connection  
    DWORD   dwLocalAddr;   //   address   on   local   computer  
    DWORD   dwLocalPort;   //   port   number   on   local   computer  
    DWORD   dwRemoteAddr;   //   address   on   remote   computer  
    DWORD   dwRemotePort;   //   port   number   on   remote   computer  
    DWORD   dwProcessId;  
} MIB_TCPEXROW,   *PMIB_TCPEXROW;  
   
typedef   struct   tagMIB_TCPEXTABLE
{  
	  DWORD   dwNumEntries;  
      MIB_TCPEXROW   table[ANY_SIZE];  
}   MIB_TCPEXTABLE,   *PMIB_TCPEXTABLE;  
   
typedef   struct   tagMIB_UDPEXROW
{ 
	DWORD   dwLocalAddr;   //   address   on   local   computer  
    DWORD   dwLocalPort;   //   port   number   on   local   computer  
    DWORD   dwProcessId;  
}   MIB_UDPEXROW,   *PMIB_UDPEXROW;  
   
typedef   struct   tagMIB_UDPEXTABLE
{  
	DWORD   dwNumEntries;  
    MIB_UDPEXROW   table[ANY_SIZE];  
}   MIB_UDPEXTABLE,   *PMIB_UDPEXTABLE;  
   
typedef   DWORD   (WINAPI   *PALLOCATE_AND_GET_TCPEXTABLE_FROM_STACK)
(PMIB_TCPEXTABLE   *pTcpTable,   //   buffer   for   the   connection   table  
 BOOL   bOrder,   //   sort   the   table?  
 HANDLE   heap,  
 DWORD   zero,  
 DWORD   flags  
 );  

typedef   DWORD   (WINAPI   *PALLOCATE_AND_GET_UDPEXTABLE_FROM_STACK)
(  
  PMIB_UDPEXTABLE   *pUdpTable,   //   buffer   for   the   connection   table  
  BOOL   bOrder,   //   sort   the   table?  
  HANDLE   heap,  
  DWORD   zero,  
  DWORD   flags  
);  


 
static   PALLOCATE_AND_GET_TCPEXTABLE_FROM_STACK   pAllocateAndGetTcpExTableFromStack   =   NULL;  
static   PALLOCATE_AND_GET_UDPEXTABLE_FROM_STACK   pAllocateAndGetUdpExTableFromStack   =   NULL;  

 
//  
//   Possible   TCP   endpoint   states  
//  
static   char   TcpState[][32]   =  
{  
	"???",  
    "CLOSED",  
    "LISTENING",  
    "SYN_SENT",  
    "SYN_RCVD",  
    "ESTABLISHED",  
    "FIN_WAIT1",  
    "FIN_WAIT2",  
    "CLOSE_WAIT",  
    "CLOSING",  
    "LAST_ACK",  
    "TIME_WAIT",  
    "DELETE_TCB"  
};  
  ///////////////////////////////////////////////////////////  
  PCHAR   GetPort(unsigned   int   port,   char*   pPort)  
  {  
  sprintf(pPort,   "%d",   htons((WORD)   port));  
  return   pPort;  
  }  
   
  PCHAR   GetIp(unsigned   int   ipaddr,   char*   pIP)  
  {  
  unsigned   int   nipaddr;  
  nipaddr   =   htonl(ipaddr);  
  sprintf(pIP,   "%d.%d.%d.%d",    
  (nipaddr   >>   24)   &   0xFF,  
  (nipaddr   >>   16)   &   0xFF,  
  (nipaddr   >>   8)   &   0xFF,  
  (nipaddr)   &   0xFF);  
  return   pIP;  
  }  


  BOOL   LoadAPI()  
  {  
  pAllocateAndGetTcpExTableFromStack   =   (PALLOCATE_AND_GET_TCPEXTABLE_FROM_STACK)  
	  GetProcAddress(   LoadLibrary(   "iphlpapi.dll"),    
      "AllocateAndGetTcpExTableFromStack"   );  
  if(   !pAllocateAndGetTcpExTableFromStack   )    
	  return   FALSE;  
  
  pAllocateAndGetUdpExTableFromStack   =   (PALLOCATE_AND_GET_UDPEXTABLE_FROM_STACK)   
	  GetProcAddress(   LoadLibrary(   "iphlpapi.dll"),    "AllocateAndGetUdpExTableFromStack"   );  
  if(   !pAllocateAndGetUdpExTableFromStack   )    
	  return   FALSE;  

  return   TRUE;  
  }  

   	WSADATA   WSAData;  
    HANDLE   hSnapshot;  
    PMIB_TCPEXTABLE   TCPExTable;  
    PMIB_UDPEXTABLE   UDPExTable;  
    char   szProcessName[MAX_PATH];  
    char   szLocalName[HOSTNAMELEN],   szRemoteName[HOSTNAMELEN];  
    char   szLocalPort[PORTNAMELEN],   szRemotePort[PORTNAMELEN]  ;  
    char   szLocalAddress[ADDRESSLEN],   szRemoteAddress[ADDRESSLEN];  
//////////End:获得网络连接信息









/////////////////////////////////////////////////////////////////////////////
// CConnectPage property page

IMPLEMENT_DYNCREATE(CConnectPage, CPropertyPage)

CConnectPage::CConnectPage() : CPropertyPage(CConnectPage::IDD)
{
	//{{AFX_DATA_INIT(CConnectPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CConnectPage::~CConnectPage()
{
}

void CConnectPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectPage)
	DDX_Control(pDX, IDC_CONNECT_LIST, m_listConnect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConnectPage, CPropertyPage)
	//{{AFX_MSG_MAP(CConnectPage)
	ON_WM_PAINT()
	ON_NOTIFY(NM_RCLICK, IDC_CONNECT_LIST, OnRclickConnectList)
	ON_COMMAND(IDM_CONNECT_END, OnConnectEnd)
	ON_COMMAND(IDM_CONNECT_REFRESH, OnConnectRefresh)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectPage message handlers



BOOL CConnectPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
		//初始化进程列表
	ListView_SetExtendedListViewStyle(
		m_listConnect.m_hWnd, 
		LVS_EX_GRIDLINES  | LVS_EX_FULLROWSELECT
		); 
	m_listConnect.InsertColumn(0, "协议",LVCFMT_LEFT,50);
	m_listConnect.InsertColumn(1, "本机IP:端口",LVCFMT_LEFT,150);
	m_listConnect.InsertColumn(2, "远程IP:端口",LVCFMT_LEFT,150);
	m_listConnect.InsertColumn(3, "状态",LVCFMT_LEFT,100);
	m_listConnect.InsertColumn(4, "进程名称",LVCFMT_LEFT,150);
	m_listConnect.InsertColumn(5, "进程ID",LVCFMT_LEFT,50);
	m_listConnect.InsertColumn(6, "进程路径",LVCFMT_LEFT,480);

	imageList=new CImageList; //systemImage.GetImageList(TRUE);
	imageList->Create(16, 16, ILC_COLOR24 | ILC_MASK, 1, 1);
	
	m_listConnect.SetImageList(imageList,LVSIL_SMALL); //设置图标链表
	
	int   nRetCode   =   LoadAPI(); //加载查看网络连接所需要的函数
	if (nRetCode   ==   FALSE)  
	{  
		MessageBox("Error LoadAPI");
	}  

	OnConnectRefresh();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//DEL void CConnectPage::OnProcessEnd() 
//DEL {
//DEL 	// TODO: Add your command handler code here
//DEL 	
//DEL }

void CConnectPage::OnConnectRefresh() 
{
  // TODO: Add your command handler code here


  m_listConnect.DeleteAllItems();//先删除所有元素 


 
  EnableDebugPrivilege();

  char   szModName[MAX_PATH]; 
  HANDLE hProcess;

  if(WSAStartup(MAKEWORD(1,1),&WSAData ))  
  {  
  printf("WSAStartup   error!\n");  
  return  ;  
  }  

  int nRetCode   = pAllocateAndGetTcpExTableFromStack(&TCPExTable,   TRUE,   GetProcessHeap(),   2,   2);  
  if(nRetCode)    
  {  
	  MessageBox("AllocateAndGetTcpExTableFromStack   Error!\n");  
  }  
  nRetCode   =   pAllocateAndGetUdpExTableFromStack(&UDPExTable,   TRUE,   GetProcessHeap(),   2,   2   );  
  if(   nRetCode   )    
  {  
  MessageBox("AllocateAndGetUdpExTableFromStack   Error!.\n");  
  }  
  

  // 创建系统快照
  hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

  if(   hSnapshot   ==   INVALID_HANDLE_VALUE   )    
  {  
  printf("CreateToolhelp32Snapshot   Error!\n");  
  } 
  int	iIndex = m_listConnect.GetItemCount(); 
  int		flag=1; 

	int iconIndex;   //项在图标链表中的位置 
	int TOTAL=0;
	CString path;  //进程路径
	CString idstr; //进程ID字符串 

  //   Get   TCP   List  
  for(unsigned int i   =   0;   i   <   TCPExTable->dwNumEntries;   i++   )    
  {  
  sprintf(   szLocalAddress,   "%s:%s",    
  GetIp(TCPExTable->table[i].dwLocalAddr,   szLocalName),  
  GetPort(TCPExTable->table[i].dwLocalPort,   szLocalPort));  
  sprintf(   szRemoteAddress,   "%s:%s",    
  GetIp(TCPExTable->table[i].dwRemoteAddr,   szRemoteName),  
  GetPort(TCPExTable->table[i].dwRemotePort,   szRemotePort));  
   
  CString str=_T("");

  memset(szModName,0,MAX_PATH);
  hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, //PROCESS_ALL_ACCESS,  
			FALSE,TCPExTable->table[i].dwProcessId); 
  if(hProcess){
			
			if(GetModuleFileNameEx(hProcess,NULL,szModName,MAX_PATH))  
			{
				str.Format("%s",szModName);
			}
			
			CloseHandle(hProcess);
		}
  //GetModules( TCPExTable->table[i].dwProcessId,flag, &str,NULL); 
  
  
  //printf("%-6s%-22s%-22s%-11s%s:%d\n",   "TCP",    
  //szLocalAddress,   szRemoteAddress,  
  //TcpState[TCPExTable->table[i].dwState],  
  //ProcessPidToName(   hSnapshot,   TCPExTable->table[i].dwProcessId,   szProcessName), 
  HICON icon=systemImage.GetIcon(str,FALSE,TRUE);//pe.szExeFile E:\\Program Files\\Tencent\\QQ\\QQ.exe
  if(icon==NULL)
  {
			MessageBox("Error!");
}
		iconIndex=imageList->Add(icon);
		m_listConnect.InsertItem(iIndex, "TCP",iconIndex);
		
		m_listConnect.SetItemText(iIndex, 1,szLocalAddress);
		m_listConnect.SetItemText(iIndex, 2,szRemoteAddress);
		m_listConnect.SetItemText(iIndex, 3,TcpState[TCPExTable->table[i].dwState]);
		
		m_listConnect.SetItemText(iIndex, 4,str.Right(str.GetLength()-str.ReverseFind('\\')-1));

	    idstr.Format("%d",TCPExTable->table[i].dwProcessId);
		m_listConnect.SetItemText(iIndex, 5,idstr);
		m_listConnect.SetItemData(iIndex,TCPExTable->table[i].dwProcessId);

		m_listConnect.SetItemText(iIndex, 6,str);
		//GetModules(pe.th32ProcessID,flag,&path,NULL);
		//m_listConnect.SetItemText(iIndex, 2,path);
		iIndex++;
		TOTAL++;
  
  }  
  //   Get   UDP   List  
  
  for(i   =   0;   i   <   UDPExTable->dwNumEntries;   i++   )    
  {  
  sprintf(   szLocalAddress,   "%s:%s",    
  GetIp(UDPExTable->table[i].dwLocalAddr,   szLocalName),  
  GetPort(UDPExTable->table[i].dwLocalPort,   szLocalPort));  
  sprintf(   szRemoteAddress, "*.*");   
  //GetIp(TCPExTable->table[i].dwRemoteAddr,   szRemoteName),  
  //GetPort(TCPExTable->table[i].dwRemotePort,   szRemotePort));  
   
  //CString str;
  //GetModules( UDPExTable->table[i].dwProcessId,flag, &str,NULL); 
  CString str=_T("");

  memset(szModName,0,MAX_PATH);
  hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, //PROCESS_ALL_ACCESS,  
			FALSE,UDPExTable->table[i].dwProcessId); 
  if(hProcess){
			
			if(GetModuleFileNameEx(hProcess,NULL,szModName,MAX_PATH))  
			{
				str.Format("%s",szModName);
			}
			
			CloseHandle(hProcess);
		}
  
  //printf("%-6s%-22s%-22s%-11s%s:%d\n",   "TCP",    
  //szLocalAddress,   szRemoteAddress,  
  //TcpState[TCPExTable->table[i].dwState],  
  //ProcessPidToName(   hSnapshot,   TCPExTable->table[i].dwProcessId,   szProcessName), 
  HICON icon=systemImage.GetIcon(str,FALSE,TRUE);//pe.szExeFile E:\\Program Files\\Tencent\\QQ\\QQ.exe
  if(icon==NULL)
  {
			MessageBox("Error!");
}
		iconIndex=imageList->Add(icon);
		m_listConnect.InsertItem(iIndex, "UDP",iconIndex);
		
		m_listConnect.SetItemText(iIndex, 1,szLocalAddress);
		m_listConnect.SetItemText(iIndex, 2,szRemoteAddress);
		m_listConnect.SetItemText(iIndex, 3,"");//TcpState[TCPExTable->table[i].dwState]
		
		m_listConnect.SetItemText(iIndex, 4,str.Right(str.GetLength()-str.ReverseFind('\\')-1));

	    idstr.Format("%d",UDPExTable->table[i].dwProcessId);
		m_listConnect.SetItemText(iIndex, 5,idstr);
		m_listConnect.SetItemData(iIndex,UDPExTable->table[i].dwProcessId);

		m_listConnect.SetItemText(iIndex, 6,str);
		//GetModules(pe.th32ProcessID,flag,&path,NULL);
		//m_listConnect.SetItemText(iIndex, 2,path);
		
		TOTAL++;
  
  } 
  
  
  
 
 
  WSACleanup();  


}



void CConnectPage::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	if(GetParent()->IsWindowVisible()==false)
	{
		this->ShowWindow(SW_HIDE);
		return ;
	}
   

	CRect rect;
	GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y;
	CPoint pointTop(x,y);
	
    m_listConnect.MoveWindow(CRect(pointTop,rect.BottomRight()));

	// Do not call CPropertyPage::OnPaint() for painting messages
}

//DEL void CConnectPage::OnRButtonDown(UINT nFlags, CPoint point) 
//DEL {
//DEL 	// TODO: Add your message handler code here and/or call default
//DEL 	
//DEL 	CPropertyPage::OnRButtonDown(nFlags, point);
//DEL }

void CConnectPage::OnRclickConnectList(NMHDR* pNMHDR, LRESULT* pResult) 
{
		// TODO: Add your control notification handler code here
	
	CRect rect;
	GetWindowRect(&rect);

	
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	int x=point.x+rect.left;
	int y=point.y+rect.top;

	if(!m_listConnect.GetSelectedCount())
	{
		return;
	}
	
	else
	{
		
		CMenu *m_PopMenu=new CMenu;
	    m_PopMenu->LoadMenu(IDR_CONNECT);//加载菜单
 
		TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);//(HWND)(AfxGetApp()->m_pMainWnd)


	}
	*pResult = 0;
}

void CConnectPage::OnConnectEnd() 
{
	// TODO: Add your command handler code here
	POSITION pos =m_listConnect.GetFirstSelectedItemPosition();
	
	if(pos==NULL)
		return;
	
	int index = m_listConnect.GetNextSelectedItem(pos);
        

	DWORD data= m_listConnect.GetItemData(index);
	
	HANDLE hProcess;
	// 打开进程
	hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,data);
	if(hProcess)
	{
	   if(!TerminateProcess(hProcess,0))
	   {
		  CString strError;
	      strError.Format("错误号:%d",GetLastError());
	      AfxMessageBox(strError,MB_OK|MB_ICONINFORMATION,NULL);
	   }
	}
	else
	{
	    CString strError;
	    strError.Format("错误号:%d",GetLastError());
	    if(GetLastError()==ERROR_ACCESS_DENIED)
		   strError=_T("拒绝访问!")+strError;
	    AfxMessageBox(strError,MB_OK|MB_ICONINFORMATION,NULL);
	}

	m_listConnect.DeleteItem(index);
	
	//Sleep(300);
	//OnConnectRefresh();	

}


	



void CConnectPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	
	// TODO: Add your message handler code here
	/*if(imageList!=NULL)
	{
		delete imageList;
	    imageList=NULL;
	}*/
}
