// LoadcodeDlg.cpp : implementation file

#include "stdafx.h"
#include "defender.h"

#include   "afxole.h" 
#include   <ole2.h>   
#include "LoadcodeDlg.h"
#include "basic/Ado.h"
#include "net/winpcap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//设备驱动句柄
extern HANDLE driverHandle;	
extern LPSTR driverDosName;

//过滤开关
extern UCHAR ICMP_FILTER; //是否允许Ping入
extern UCHAR PACKET_FILTER; //是否进行数据包过滤
extern UCHAR ANTIARP;       //是否启用ARP防火墙
extern UCHAR PLAYALLROLE;   //是否伪装成所有不存活的主机

//特征码临时列表
CodeNode codelist[100]={0};
CLoadcodeDlg *p_loadcode;

extern char szCurrentDirectory[256];
//读取数据库中的特征码,传递给驱动程序
//每次传递100条
DWORD WINAPI ReadCode( LPVOID lpParameter);
//读取ini配置文件
int  ReadIni(UCHAR *icmp_filter,UCHAR *packet_filter,UCHAR *antiarp,UCHAR *playallrole);
/////////////////////////////////////////////////////////////////////////////
// LoadCode dialog


CLoadcodeDlg::CLoadcodeDlg(CWnd* pParent /*=NULL*/)
: CDialog(CLoadcodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadcodeDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLoadcodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadcodeDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadcodeDlg, CDialog)
//{{AFX_MSG_MAP(CLoadcodeDlg)
ON_WM_SHOWWINDOW()
ON_WM_CLOSE()
ON_WM_PAINT()
ON_WM_DESTROY()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//屏蔽Enter键
void CLoadcodeDlg::OnOK() 
{
	return;
}
//屏蔽"X"
void CLoadcodeDlg::OnClose() 
{
	return;
}
//屏蔽ESC键
void CLoadcodeDlg::OnCancel()
{
	//return;

	CDialog::OnCancel();
}

void CLoadcodeDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	//隐藏标题栏
	this->ModifyStyle(WS_CAPTION,0,SWP_FRAMECHANGED);
	//设置窗口尺寸及风格
	::SetWindowPos(this->m_hWnd, HWND_TOPMOST,0, 0, 400, 260,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);   
	
	HANDLE hThread;
	DWORD  dwThreadid=0;
	CDialog::OnShowWindow(bShow, nStatus);
	p_loadcode=this;
	driverDosName=(char *)malloc(strlen(DRIVERNAME)+5);
	sprintf(driverDosName, "\\\\.\\%s", DRIVERNAME);
	
	
	//打开设备
	driverHandle = CreateFile(driverDosName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
    if(driverHandle == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("打开NDIS驱动失败!");
		this->PostMessage(WM_DESTROY,0,0);
	}
	else
	{
		//读取配置文件(过滤规则)
		ReadIni(&ICMP_FILTER,&PACKET_FILTER,&ANTIARP,&PLAYALLROLE);
		
		//加载特征码		
		//创建读取特征码线程
		hThread=CreateThread(NULL,
			0,
			ReadCode,
			NULL,
			0,
			&dwThreadid);
		CloseHandle(hThread);
	}
}


//读取数据库中的特征码，传递给驱动程序
DWORD WINAPI ReadCode(LPVOID lpParameter)
{
	UINT i=0;
	UINT finishcount=0;
	UINT totalcount=0;
	UINT percent=0;
	DWORD bytesReturned=0;
	BOOL ret=0;
	CString csPercent;
	CString csShellCode;

	SetCurrentDirectory(szCurrentDirectory);
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=code.mdb");
	pAdoDb->SetConnectionString(strConnection);
	
	if(pAdoDb->Open())
	{
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		if(pAdoRs->Open("shellcode", CADORecordset::openTable))
		{
			//取得总特征码数
			totalcount=pAdoRs->GetRecordCount();
			while(!pAdoRs->IsEOF())
			{
				if(i==100)
				{
					//发送给驱动程序
					ret=DeviceIoControl(driverHandle,
						IADS_DRIVER_ADDCODE,
						(LPVOID)&codelist,
						100*sizeof(CodeNode),
						NULL,
						0,
						&bytesReturned,
						NULL //同步进行
						);
					//初始化
					i=0;
					memset((unsigned char *)&codelist,0,sizeof(CodeNode)*100);
				}
				//读取数据(codelist[i])
				pAdoRs->GetFieldValue("shellcode",csShellCode);
				strncpy((char *)codelist[i++].code,csShellCode,32);
				Sleep(10);
		    	pAdoRs->MoveNext();
				finishcount++;

				//完成百分比
				percent=(UINT)(100*(float)finishcount/(float)totalcount);
				csPercent.Format("%d%%",percent);
				p_loadcode->SetDlgItemText(IDC_PERCENT,csPercent);
			}
			//剩余的i条特征码
			if(i)
			{
				//发送给驱动程序
				ret=DeviceIoControl(driverHandle,
					IADS_DRIVER_ADDCODE,
					(LPVOID)&codelist,
					i*sizeof(CodeNode),
					NULL,
					0,
					&bytesReturned,
					NULL //同步进行
					);
			}
			pAdoRs->Close();	
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		MessageBox(0,"加载数据失败!","智能主动防御系统",MB_OK);
	}
	delete pAdoDb;
	p_loadcode->EndDialog(0);
	return 0;
}


//读取ini配置文件
int  ReadIni(UCHAR *icmp_filter,UCHAR *packet_filter,UCHAR *antiarp,UCHAR *playallrole)
{
	char tempbuf[256]={0};//ini文件路径
	char smallbuf[20]={0};
	DWORD nSize=19;
	*icmp_filter=1;
	*packet_filter=1;
	*antiarp=1;
	SetCurrentDirectory(szCurrentDirectory);
	if(!GetCurrentDirectory(250,tempbuf)) return 1;
	if(tempbuf[strlen(tempbuf)-1]!='\\')
		tempbuf[strlen(tempbuf)]='\\';
	strcat(tempbuf,"iads.ini");
	
	memset(smallbuf,0,20);
	if(GetPrivateProfileString("FilterRules",
		"ICMP_FILTER",
		"1",
		smallbuf,
		nSize,
		tempbuf))
	*icmp_filter=atoi(smallbuf);
	
	memset(smallbuf,0,20);
	if(GetPrivateProfileString("FilterRules",
		"PACKET_FILTER",
		"1",
		smallbuf,
		nSize,
		tempbuf))
	*packet_filter=atoi(smallbuf);
	
	memset(smallbuf,0,20);
	if(GetPrivateProfileString("FilterRules",
		"ANTIARP",
		"1",
		smallbuf,
		nSize,
		tempbuf))
	*antiarp=atoi(smallbuf);

	memset(smallbuf,0,20);
	if(GetPrivateProfileString("FilterRules",
		"PLAYALLROLE",
		"1",
		smallbuf,
		nSize,
		tempbuf))
	*playallrole=atoi(smallbuf);
	return 0;
}

void CLoadcodeDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting	
	CRect rect;
	GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y;
	CPoint pointTop(x,y);
	GetClientRect(&rect);//获取窗体大小
	CDC  memDC; 
	CBitmap cBitmap;
	CBitmap* pOldMemBmp=NULL;
	cBitmap.LoadBitmap(IDB_LOADCODE);//加载位图（事先已经添加到资源文件中）
	memDC.CreateCompatibleDC(&dc);//创建与窗口DC兼容的DC
	pOldMemBmp=memDC.SelectObject(&cBitmap);//将新的位图选入设备
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&memDC,0,0,400,260,SRCCOPY);//设备之间的拷贝
	memDC.SelectObject(pOldMemBmp);//恢复设备
	CDialog::OnPaint();
}

void CLoadcodeDlg::OnDestroy() 
{
	CDialog::OnDestroy();
}


