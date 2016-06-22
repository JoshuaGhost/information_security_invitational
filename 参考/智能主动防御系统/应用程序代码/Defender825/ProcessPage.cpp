// ProcessPage.cpp : implementation file
//

#include "stdafx.h"
#include "Defender.h"
#include "ProcessPage.h"
#include "SystemImageList.h" //系统图标类
#include "ModuleDlg.h"
#include "MainFrm.h"

#include "tlhelp32.h"  //操控进程的函数
#include   <psapi.h> 
#include<vector>
using namespace std;

#pragma comment(lib,"Psapi.lib ")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//排序回调函数
int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
// 提高进程权限
BOOL EnableDebugPrivilege();
extern char processbuffer[MAX_STORE/4];
extern bool process_flag;
extern HANDLE hEvent;
extern HANDLE mutexBuffer;

vector<DWORD>vec_tochange;


/////////////////////////////////////////////////////////////////////////////
// CProcessPage property page

IMPLEMENT_DYNCREATE(CProcessPage, CPropertyPage)

CProcessPage::CProcessPage() : CPropertyPage(CProcessPage::IDD)
{
	//{{AFX_DATA_INIT(CProcessPage)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CProcessPage::~CProcessPage()
{
}

void CProcessPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessPage)
	DDX_Control(pDX, IDC_PROCESS_LIST, m_listProcess);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessPage, CPropertyPage)
//{{AFX_MSG_MAP(CProcessPage)
ON_WM_PAINT()
ON_COMMAND(IDM_PROCESS_REFRESH, OnProcessRefresh)
ON_COMMAND(IDM_PROCESS_END, OnProcessEnd)
ON_NOTIFY(NM_RCLICK, IDC_PROCESS_LIST, OnRclickProcessList)
ON_NOTIFY ( NM_CUSTOMDRAW, IDC_PROCESS_LIST, OnCustomdrawMyList )
ON_COMMAND(IDM_PROCESS_MODULE, OnProcessModule)
ON_WM_DESTROY()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessPage message handlers

BOOL CProcessPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	//初始化进程列表
	ListView_SetExtendedListViewStyle(
		m_listProcess.m_hWnd, 
		LVS_EX_GRIDLINES  | LVS_EX_FULLROWSELECT
		); 
	m_listProcess.InsertColumn(0, "进程名称",LVCFMT_LEFT,150);
	
	m_listProcess.InsertColumn(1, "进程ID",LVCFMT_LEFT,50);
	
	m_listProcess.InsertColumn(2, "进程路径",LVCFMT_LEFT,480);
	
	m_listProcess.InsertColumn(3, "描述信息",LVCFMT_LEFT,120);
	
	imageList=new CImageList; //systemImage.GetImageList(TRUE);
	imageList->Create(16, 16, ILC_COLOR24 | ILC_MASK, 1, 1);
	
	m_listProcess.SetImageList(imageList,LVSIL_SMALL); //设置图标链表
	
	
	OnProcessRefresh();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessPage::OnPaint() 
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
	int y=rect.TopLeft().y+10;
	CPoint pointTop(x,y);
	
    m_listProcess.MoveWindow(CRect(pointTop,rect.BottomRight()));
	
	// Do not call CPropertyPage::OnPaint() for painting messages
}

void CProcessPage::OnProcessRefresh() 
{
	
	m_listProcess.DeleteAllItems();
	int	iIndex = m_listProcess.GetItemCount();
	HANDLE hSnapshot;
	BOOL bLoop=FALSE;
	
	EnableDebugPrivilege();
	
	// 创建系统快照
	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	PROCESSENTRY32 pe;
	
	memset(&pe,0,sizeof(PROCESSENTRY32));
	pe.dwSize=sizeof(PROCESSENTRY32);
	bLoop=Process32First(hSnapshot,&pe);
	
	
	int iconIndex;   //项在图标链表中的位置 
	int TOTAL=0;
	CString path;  //进程路径
	CString idstr; //进程ID字符串
	int flag=1;
	char   szModName[MAX_PATH]; 
	HANDLE hProcess;
	
	while(bLoop)
	{
		path=_T("");
		
		memset(szModName,0,MAX_PATH);
		hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, //PROCESS_ALL_ACCESS,  
			FALSE,pe.th32ProcessID); 
		if(hProcess){
			
			if(GetModuleFileNameEx(hProcess,NULL,szModName,MAX_PATH))  
			{
				path.Format("%s",szModName);
			}
			
			CloseHandle(hProcess);
		}
		HICON icon=systemImage.GetIcon(TEXT(path),FALSE,TRUE);//pe.szExeFile
		iconIndex=imageList->Add(icon);
		m_listProcess.InsertItem(iIndex, pe.szExeFile,iconIndex);
		
		idstr.Format("%d",pe.th32ProcessID);
		m_listProcess.SetItemText(iIndex, 1,idstr);
		m_listProcess.SetItemData(iIndex,pe.th32ProcessID);
		m_listProcess.SetItemText(iIndex, 2,path);
		TOTAL++;
		bLoop=Process32Next(hSnapshot,&pe);
	}
	m_listProcess.SortItems(MyCompareProc,pe.th32ProcessID);
}

void CProcessPage::OnProcessEnd() 
{
	int index ;
	POSITION pos =m_listProcess.GetFirstSelectedItemPosition();
	if(pos==NULL)
		return;

	EnableDebugPrivilege();

	while(pos)
	{
		
		index=m_listProcess.GetNextSelectedItem(pos);
		DWORD data= m_listProcess.GetItemData(index);
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
	}

	OnProcessRefresh();
	
}


void CProcessPage::OnRclickProcessList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CRect rect;
	GetWindowRect(&rect);
	
	
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	int x=point.x+rect.left;
	int y=point.y+rect.top;
	
	if(!m_listProcess.GetSelectedCount())
	{
		return;
	}
	
	else
	{
		CMenu *m_PopMenu=new CMenu;
		m_PopMenu->LoadMenu(IDR_PROCESS);//加载菜单
		
		TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);//(HWND)(AfxGetApp()->m_pMainWnd)
	}
	
	*pResult = 0;
}


void CProcessPage::OnProcessModule() 
{
	// TODO: Add your command handler code here
	CString cs;
	POSITION pos = m_listProcess.GetFirstSelectedItemPosition();
	
	int nItem = m_listProcess.GetNextSelectedItem(pos);
		
	CModuleDlg dlg(m_listProcess.GetItemData(nItem));
	dlg.DoModal();
}


// 排序
static int CALLBACK 
MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return lParam1 > lParam2;
}

void CProcessPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
}




void CProcessPage::OnCustomdrawMyList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
	// Take the default processing unless we set this to something else below.
	*pResult = CDRF_DODEFAULT;
	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.
	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		// This is the notification message for an item. We'll request
		// notifications before each subitem's prepaint stage.
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		// This is the prepaint stage for a subitem. Here's where we set the
		// item's text and background colors. Our return value will tell
		// Windows to draw the subitem itself, but it will use the new colors
		// we set here.
		// The text color will cycle through red, green, and light blue.
		// The background color will be light blue for column 0, red for
		// column 1, and black for column 2.
		
		DWORD data=m_listProcess.GetItemData(pLVCD->nmcd.dwItemSpec);
		//CString cs;
		//cs.Format("%d  %d",data,vec_tochange.size());
		//MessageBox("cs");
		for(int i=0;i<vec_tochange.size();i++)
		{
			if(data==vec_tochange[i])
			{
				pLVCD->clrText = RGB(0, 255 , 0);
				pLVCD->clrTextBk =RGB(255,0,0);
				break;
			}
		}
		
		
		*pResult = CDRF_DODEFAULT;
	}
}

//-------------------------------------------------------------------
// EnableDebugPrivilege
// 提高进程权限
// 返回值: 成功返回TRUE 失败返回FALSE
//-------------------------------------------------------------------
BOOL EnableDebugPrivilege()
{
	HANDLE hToken;
	BOOL fOk=FALSE;
	if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount=1;
		LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL);
		fOk=(GetLastError()==ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return fOk;
}