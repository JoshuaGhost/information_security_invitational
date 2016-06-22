// ModuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "ModuleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HINSTANCE  hDll=NULL;  //指向模块dll的句柄
typedef struct
{
	unsigned int modBaseAddr;  //模块基地址
	char szExePath[256];
}ModuleInfor;
typedef (* UnLoadModuleInAllProcess)(char unloadType ,unsigned int modBaseAddr);
typedef (* GetProcessModule)        (DWORD dwPID,char OutBuf[]);
typedef (* UnLoadModuleSoft)        (DWORD dwPID,unsigned modBaseAddr);
typedef (* UnLoadModuleHard)        (DWORD dwPID, unsigned int modBaseAddr);
/////////////////////////////////////////////////////////////////////////////
// CModuleDlg dialog


CModuleDlg::CModuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModuleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModuleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
CModuleDlg::CModuleDlg(DWORD pid):CDialog(CModuleDlg::IDD, NULL)
{
	m_pid=pid;
	hDll=LoadLibrary("ModuleOperate.dll"); //初始化
	if(hDll==NULL)
	   MessageBox("ERrror");
}


void CModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModuleDlg)
	DDX_Control(pDX, IDC_MODULE_LIST, m_listModule);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModuleDlg, CDialog)
	//{{AFX_MSG_MAP(CModuleDlg)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_REFRESH_BUTTON, OnRefreshButton)
	ON_BN_CLICKED(IDC_SOFT_BUTTON, OnSoftButton)
	ON_BN_CLICKED(IDC_FORCE_BUTTON, OnForceButton)
	ON_BN_CLICKED(IDC_FORCEALL_BUTTON, OnForceallButton)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModuleDlg message handlers

void CModuleDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	//设置列表样式
	m_listModule.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	//添加表头
	m_listModule.InsertColumn( 0, "基址", LVCFMT_CENTER, 120); 
	m_listModule.InsertColumn( 1, "模块名称", LVCFMT_LEFT, 400); 	
	OnRefreshButton();
}

void CModuleDlg::OnRefreshButton() 
{
	// TODO: Add your control notification handler code here
	char* buf;
	int num;       //模块数
    int i,iIndex=0;

	m_listModule.DeleteAllItems();

	GetProcessModule MyGet=(GetProcessModule)GetProcAddress(hDll,"GetProcessModule");
	if(MyGet==NULL)
	{
		MessageBox("Error!");
	}
    num=MyGet(m_pid,NULL);
	buf=(char*)malloc(num*sizeof(ModuleInfor));
    memset(buf,0,num*sizeof(ModuleInfor));
	MyGet(m_pid,buf);
	
	for(i=0;i<num;i++)
	{
		CString cs;
		cs.Format("0X%0X",((ModuleInfor*)buf)[i].modBaseAddr);

		

		m_listModule.InsertItem(iIndex, cs);
		m_listModule.SetItemData(iIndex, ((ModuleInfor*)buf)[i].modBaseAddr);
        cs.Format("%s",((ModuleInfor*)buf)[i].szExePath);
		m_listModule.SetItemText(iIndex, 1,((ModuleInfor*)buf)[i].szExePath);
		iIndex++;
	}
	if(buf!=NULL)
		free(buf);


}

void CModuleDlg::OnSoftButton() 
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	POSITION pos = m_listModule.GetFirstSelectedItemPosition();
	int nItem = m_listModule.GetNextSelectedItem(pos);

	UnLoadModuleSoft unloadsoft=NULL;
    unloadsoft=(UnLoadModuleSoft)GetProcAddress(hDll,"UnLoadModuleSoft");
	
	if(unloadsoft(m_pid,m_listModule.GetItemData(nItem))==0)
	{
		m_listModule.DeleteItem(nItem);
	}
	  
}

void CModuleDlg::OnForceButton() 
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_listModule.GetFirstSelectedItemPosition();
	int nItem = m_listModule.GetNextSelectedItem(pos);

	UnLoadModuleHard unloadhard=NULL;
    unloadhard=(UnLoadModuleHard)GetProcAddress(hDll,"UnLoadModuleHard");

	if(unloadhard(m_pid,m_listModule.GetItemData(nItem)))
	{
		m_listModule.DeleteItem(nItem);
	}
}

void CModuleDlg::OnForceallButton() 
{
	POSITION pos = m_listModule.GetFirstSelectedItemPosition();
	int nItem = m_listModule.GetNextSelectedItem(pos);

	UnLoadModuleInAllProcess unloadallforce=NULL;
    unloadallforce=(UnLoadModuleInAllProcess)GetProcAddress(hDll,"UnLoadModuleInAllProcess");

	if(unloadallforce(1,m_listModule.GetItemData(nItem))==0)
	{
		m_listModule.DeleteItem(nItem);
	}
}


void CModuleDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
