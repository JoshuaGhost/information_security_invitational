// ServicePage.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "ServicePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HINSTANCE  servicehDll=NULL;  //指向模块dll的句柄

typedef struct
{
	char lpServiceName[256];    //服务名
	char lpDisplayName[256];    //显示名称
	char lpCurrentState[20];     //当前状态
	char lpServiceType[20];     //服务类型
	char lpStartType[20];        //启动类型
	char lpBinaryPathName[256]; //服务模块路径
	char lpDescription[256];    //服务描述
}SERVICE_INFOR;//服务信息

typedef int (*GetServicesCount)(BOOL state);//获取服务数
typedef int (*GetServicesList)(char buf[]);//获取服务列表
typedef int (*StartServiceByName)(char strServiceName[]);//启动服务
typedef int (*StopServiceByName)(char strServiceName[]); //停止服务
typedef int  (*AutomaticServiceByName)(char strServiceName[]);//根据服务名称修改服务启动类型为"自动"
typedef int  (*ManualServiceByName)(char strServiceName[]);// 根据服务名称修改服务启动类型为"手动"
typedef int  (*DisableServiceByName)(char strServiceName[]);// 根据服务名称修改服务启动类型为"禁止"

/////////////////////////////////////////////////////////////////////////////
// CServicePage property page

IMPLEMENT_DYNCREATE(CServicePage, CPropertyPage)

CServicePage::CServicePage() : CPropertyPage(CServicePage::IDD)
{
	//{{AFX_DATA_INIT(CServicePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CServicePage::~CServicePage()
{
}

void CServicePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServicePage)
	DDX_Control(pDX, IDC_SERVICE_LIST, m_listService);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServicePage, CPropertyPage)
	//{{AFX_MSG_MAP(CServicePage)
	ON_COMMAND(IDM_SERVICE_REFRESH, OnServiceRefresh)
	ON_COMMAND(IDM_SERVICE_START, OnServiceStart)
	ON_COMMAND(IDM_SERVICE_STOP, OnServiceStop)
	ON_WM_PAINT()
	ON_NOTIFY(NM_RCLICK, IDC_SERVICE_LIST, OnRclickServiceList)
	ON_COMMAND(IDM_SERVICE_AUTO, OnServiceAuto)
	ON_COMMAND(IDM_SERVICE_DISABLE, OnServiceDisable)
	ON_COMMAND(IDM_SERVICE_MANUAL, OnServiceManual)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServicePage message handlers

BOOL CServicePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	//初始化进程列表
	ListView_SetExtendedListViewStyle(
		m_listService.m_hWnd, 
		LVS_EX_GRIDLINES  | LVS_EX_FULLROWSELECT
		); 
	m_listService.InsertColumn(0, "服务名称",LVCFMT_LEFT,150);

	m_listService.InsertColumn(1, "服务显示名称",LVCFMT_LEFT,150);

	m_listService.InsertColumn(2, "状态",LVCFMT_LEFT,50);

	m_listService.InsertColumn(3, "服务类型",LVCFMT_LEFT,100);

	m_listService.InsertColumn(4, "启动类型",LVCFMT_LEFT,80);
	m_listService.InsertColumn(5, "服务模块路径",LVCFMT_LEFT,220);
	m_listService.InsertColumn(6, "服务描述",LVCFMT_LEFT,220);

	imageList=new CImageList; //systemImage.GetImageList(TRUE);
	imageList->Create(16, 16, ILC_COLOR24 | ILC_MASK, 1, 1);
	
	m_listService.SetImageList(imageList,LVSIL_SMALL); //设置图标链表


	servicehDll=LoadLibrary("ModuleOperate.dll");  //初始化

	OnServiceRefresh();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CServicePage::OnServiceRefresh() 
{

	int ServicesCount;       //服务数
    int i,iIndex=0;

	m_listService.DeleteAllItems();

	
	GetServicesCount	getservicescount;
	getservicescount=(GetServicesCount)GetProcAddress((HMODULE)servicehDll,"GetServicesCount");
	

	//获取服务数
	ServicesCount=getservicescount(0);
    //获取服务列表
	SERVICE_INFOR *p_service_infor=NULL;
	int returncount=0;
   
	GetServicesList getserviceslist=(GetServicesList)GetProcAddress((HMODULE)servicehDll,"GetServicesList");
	//分配内存
	p_service_infor=(SERVICE_INFOR *)malloc(ServicesCount*sizeof(SERVICE_INFOR));
	memset(p_service_infor,0,ServicesCount*sizeof(SERVICE_INFOR));

	returncount=getserviceslist((char *)p_service_infor);
	
	CString cs,cs2;
	//if(path[0]=='\"')
	//			path=path.Mid(1,path.GetLength()-2);

	int end,start,iconIndex;
	HICON icon;
	for(i=0;i<returncount;i++)
	{
		start=0;
		cs.Format("%s",p_service_infor->lpBinaryPathName);
		end=cs.Find(".exe");
		if(end==-1)
			end=cs.Find(".EXE");
		if(end==-1)
			end=cs.Find(' ');
		if(cs[0]=='\"')
			start=1;
		cs2=cs.Mid(start,end-start)+".exe";
		//MessageBox(cs2);
		icon=systemImage.GetIcon(cs2,FALSE,TRUE);
		iconIndex=imageList->Add(icon);

		cs.Format("%s",p_service_infor->lpServiceName);
		m_listService.InsertItem(iIndex,cs,iconIndex);
		cs.Format("%s",p_service_infor->lpDisplayName);
        m_listService.SetItemText(iIndex, 1,cs);
		cs.Format("%s",p_service_infor->lpCurrentState);
		m_listService.SetItemText(iIndex, 2,cs);
		cs.Format("%s",p_service_infor->lpServiceType);
		m_listService.SetItemText(iIndex, 3,cs);
		cs.Format("%s",p_service_infor->lpStartType);
		m_listService.SetItemText(iIndex,4,cs);
		cs.Format("%s",p_service_infor->lpBinaryPathName);
		m_listService.SetItemText(iIndex, 5,cs);
		cs.Format("%s",p_service_infor->lpDescription);
		m_listService.SetItemText(iIndex, 6,cs2);

		if(i==returncount-1)
			break;
		p_service_infor++;
	}

}

void CServicePage::OnServiceStart() 
{
	// TODO: Add your command handler code here
	StartServiceByName  startservicebyname;
	startservicebyname=(StartServiceByName)GetProcAddress((HMODULE)servicehDll,"StartServiceByName");

	int index ;
	POSITION pos =m_listService.GetFirstSelectedItemPosition();

	if(pos==NULL)
		return;
	//total=m_listProcess.GetSelectedCount();
	//loc=new int[total];

	CString cs;
	while(pos)
	{
		
		index=m_listService.GetNextSelectedItem(pos);
		cs=m_listService.GetItemText(index, 0);
		startservicebyname((char*)((LPCTSTR)cs));
	
	
	}
	OnServiceRefresh();
}

void CServicePage::OnServiceStop() 
{
	StopServiceByName  stopservicebyname;
	stopservicebyname=(StopServiceByName)GetProcAddress((HMODULE)servicehDll,"StopServiceByName");

	int index ;
	POSITION pos =m_listService.GetFirstSelectedItemPosition();

	if(pos==NULL)
		return;
	//total=m_listProcess.GetSelectedCount();
	//loc=new int[total];

	CString cs;
	while(pos)
	{
		
		index=m_listService.GetNextSelectedItem(pos);
		cs=m_listService.GetItemText(index, 0);
		stopservicebyname((char*)((LPCTSTR)cs));
	
	
	}
	OnServiceRefresh();
	
}

void CServicePage::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	
	if(GetParent()->IsWindowVisible()==false)
	{
		this->ShowWindow(SW_HIDE);
		return ;
	}

	CRect rect;
	GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y+10;
	CPoint pointTop(x,y);
	
    m_listService.MoveWindow(CRect(pointTop,rect.BottomRight()));

}

void CServicePage::OnRclickServiceList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CRect rect;
	GetWindowRect(&rect);
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	int x=point.x+rect.left;
	int y=point.y+rect.top;

	if(!m_listService.GetSelectedCount())
	{
		return;
	}
	
	else
	{
		
		CMenu *m_PopMenu=new CMenu;
	    m_PopMenu->LoadMenu(IDR_SERVICE);//加载菜单
 
		TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);//(HWND)(AfxGetApp()->m_pMainWnd)


	}
	
	*pResult = 0;
}

void CServicePage::OnServiceAuto() 
{
	// TODO: Add your command handler code here
	AutomaticServiceByName  automaticservicebyname;
	automaticservicebyname=(AutomaticServiceByName)GetProcAddress((HMODULE)servicehDll,"AutomaticServiceByName");

	int index ;
	POSITION pos =m_listService.GetFirstSelectedItemPosition();

	if(pos==NULL)
		return;
	//total=m_listProcess.GetSelectedCount();
	//loc=new int[total];

	CString cs;
	while(pos)
	{
		
		index=m_listService.GetNextSelectedItem(pos);
		cs=m_listService.GetItemText(index, 0);
		automaticservicebyname((char*)((LPCTSTR)cs));
	
	
	}
	OnServiceRefresh();
	
}


void CServicePage::OnServiceManual() 
{
	ManualServiceByName  manualservicebyname;
	manualservicebyname=(ManualServiceByName)GetProcAddress((HMODULE)servicehDll,"ManualServiceByName");

	int index ;
	POSITION pos =m_listService.GetFirstSelectedItemPosition();

	if(pos==NULL)
		return;
	//total=m_listProcess.GetSelectedCount();
	//loc=new int[total];

	CString cs;
	while(pos)
	{
		
		index=m_listService.GetNextSelectedItem(pos);
		cs=m_listService.GetItemText(index, 0);
		manualservicebyname((char*)((LPCTSTR)cs));
	
	
	}
	OnServiceRefresh();

}

void CServicePage::OnServiceDisable() 
{
	DisableServiceByName  disableservicebyname;
	disableservicebyname=(DisableServiceByName)GetProcAddress((HMODULE)servicehDll,"DisableServiceByName");

	int index ;
	POSITION pos =m_listService.GetFirstSelectedItemPosition();

	if(pos==NULL)
		return;
	//total=m_listProcess.GetSelectedCount();
	//loc=new int[total];

	CString cs;
	while(pos)
	{
		
		index=m_listService.GetNextSelectedItem(pos);
		cs=m_listService.GetItemText(index, 0);
		disableservicebyname((char*)((LPCTSTR)cs));
	
	
	}
	OnServiceRefresh();
	
}

void CServicePage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
