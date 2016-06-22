// MainFrm.cpp : implementation of the CMainFrame class


#include "stdafx.h"
#include "Defender.h"
#include "MainFrm.h"
#include "net/IOCommon.h"
#include "DangerDlg.h"
#include "LocalPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char buffer[MAX_STORE+1];
char processbuffer[MAX_STORE/4]={0};

//�豸�������
extern HANDLE driverHandle;	
extern LPSTR driverDosName;

HANDLE eventReceive=NULL;
HANDLE eventSend=NULL;

HANDLE handleDanger=NULL;
HANDLE handleOutput=NULL;


extern CLocalPage *p_this;


extern checkhost;


//����
NOTIFYICONDATA nid;
int start;
#define UM_TRAY (WM_USER +11)  //�Զ���������Ϣ
#define UM_STATUS (WM_USER +12)  //�Զ���״̬����Ϣ
#define UM_DANGER (WM_USER +25)  //�Զ���Σ�ս�����Ϣ
#define UM_CHOICE (WM_USER +14)  //�Զ����û�ѡ��ť��Ϣ,
#define UM_PROCESS (WM_USER +15)  //�Զ�����ʾ������Ϣ
#define UM_BEGINMONITOR (WM_USER+16)
#define UM_ENDMONITOR (WM_USER +17)


CMenu *m_PopMenu;//���̵����˵��˵�
BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam);//�������̴���

CDangerDlg *dangerDlg=NULL;
int choice=0;


void thread(LPVOID pdata);
void  outputproc(LPVOID  param);


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_COMMAND(IDR_OPEN, OnOpen)
	ON_COMMAND(IDR_EXIT, OnExit)
	ON_WM_DESTROY()
	ON_COMMAND(IDM_HELP, OnHelp)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UM_TRAY,OnUMTray)
	ON_MESSAGE(UM_STATUS,OnStatusMessage)
	ON_MESSAGE(UM_DANGER,OnDangerMessage)
	ON_MESSAGE(UM_CHOICE,OnChoiceMessage)
	ON_MESSAGE(UM_PROCESS,OnProcessMessage)
	ON_MESSAGE(UM_BEGINMONITOR,BeginMonitor)
	ON_MESSAGE(UM_ENDMONITOR,EndMonitor)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipText )
END_MESSAGE_MAP()

static UINT indicators[] =
{
	//ID_SEPARATOR,            //status line indicator
	IDS_MYSTATUS, //ID_INDICATOR_CAPS,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
	CWnd* pWnd = FindWindow("Shell_TrayWnd", NULL);
    if (pWnd)
    {
        pWnd->GetWindowRect(m_rectTray);
		EnumChildWindows(pWnd->m_hWnd, FindTrayWnd, (LPARAM)&m_rectTray);
	}
	beginMonitor=0;
	for(int n=0;n<protectIndex;n++)
	{
		security[n]=(char *)malloc(MAXPROCESS*sizeof(char));
		memset(security[n],0,MAXPROCESS);
	}
	
	//GetWindowRect(m_rectShow);
}

CMainFrame::~CMainFrame()
{
	for(int i=0;i<protectIndex;i++)
	{
		free(security[i]);
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	if (!m_hotToolBar.Create(this) ||
		!m_hotToolBar.LoadToolBar(IDR_HOTBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_hotToolBar.ModifyStyle(0, TBSTYLE_FLAT|CBRS_TOOLTIPS );

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	EnableDocking(CBRS_ALIGN_ANY);

	SetMenu(NULL);

	// Set up hot bar image lists.
	CImageList	imageList;
	CBitmap		bitmap;

	// Create and set the normal toolbar image list.
	bitmap.LoadBitmap(IDB_TOOLBAR_COLD);
	imageList.Create(76, 68, ILC_COLORDDB|ILC_MASK, 13, 1);
	imageList.Add(&bitmap, RGB(255,0,255));
	m_hotToolBar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
	imageList.Detach();
	bitmap.Detach();

	bitmap.LoadBitmap(IDB_TOOLBAR_HOT);
	imageList.Create(76, 68, ILC_COLORDDB|ILC_MASK, 13, 1);
	imageList.Add(&bitmap, RGB(255,0,255));
	m_hotToolBar.SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
	imageList.Detach();
	bitmap.Detach();

	
	//�������� 
    nid.cbSize=(DWORD)sizeof(NOTIFYICONDATA); 
	nid.hWnd=this->m_hWnd;
	nid.uID=IDR_MAINFRAME; 
	nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP ; 
	nid.uCallbackMessage=UM_TRAY;//�Զ������Ϣ���� 
	nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME)); 
	strcpy(nid.szTip,"������������ϵͳ"); //��Ϣ��ʾ�� 
	Shell_NotifyIcon(NIM_ADD,&nid); //�����������ͼ��
	
	m_PopMenu=new CMenu;
	m_PopMenu->LoadMenu(IDR_TRAY);//���ز˵�

	start=1;  //һ��ʼ��ʾ  ����

	
	EnableToolTips(TRUE);

	LoadHostDriver();
	OnHook();
//	OnOutput();



	//MessageBox("Start servece ok!");
	hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);

		//��ʼ��Σ����Ϣ��ʾ

memset(info[0].symbol,0,3);
memset(info[0].info,0,sizeof(info[0].info)/sizeof(char));

memset(info[1].symbol,0,3);
memset(info[1].info,0,sizeof(info[0].info)/sizeof(char));

memset(info[2].symbol,0,3);
memset(info[2].info,0,sizeof(info[0].info)/sizeof(char));

memset(info[3].symbol,0,3);
memset(info[3].info,0,sizeof(info[0].info)/sizeof(char));

memset(info[4].symbol,0,3);
memset(info[4].info,0,sizeof(info[0].info)/sizeof(char));
memset(info[5].symbol,0,3);
memset(info[5].info,0,sizeof(info[0].info)/sizeof(char));

	strcpy(info[0].symbol,"$$");
	strcpy(info[0].info,"�޸�ע���");

	strcpy(info[1].symbol,"**");
	strcpy(info[1].info,"��������");
	
	strcpy(info[2].symbol,"!!");
	strcpy(info[2].info,"��ͼע���̵߳�");

	strcpy(info[4].symbol,"##");
	strcpy(info[4].info,"��������");
	
	strcpy(info[5].symbol,"##");
	strcpy(info[5].info,"��ͼ��������");
    

	strcpy(security[0],"SVCHOST");
	strcpy(security[1],"EXPLORER");
	strcpy(security[2],"SYSTEM");
	strcpy(security[3],"LSASS");
	strcpy(security[4],"CSRSS");
	strcpy(security[5],"WINLOGON");
	strcpy(security[6],"360TRAY");
	strcpy(security[7],"SMSS");
	strcpy(security[8],"WUAUCLT");
	strcpy(security[9],"SERVICES");



	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	//ȡ����Ļ��Ⱥ͸߶�
	int ScreenWidth=GetSystemMetrics(SM_CXSCREEN);
	int ScreenHeight=GetSystemMetrics(SM_CYSCREEN);
	int width=770; //��������
	int height=600;//������߶�

	//�����ʼ����ʾ����Ļ����
	cs.x=ScreenWidth/2-width/2;
	cs.y=ScreenHeight/2-height/2;

    cs.style   &=   ~WS_THICKFRAME;
    cs.style   &=   ~WS_MAXIMIZEBOX;
	//�޸Ĵ�����ʾ��С
	cs.cx=width;
	cs.cy=height;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
	// TODO: Add your message handler code here and/or call default
	if(nID==SC_MINIMIZE) 
	{
		m_PopMenu->ModifyMenu(IDR_OPEN,MF_BYCOMMAND,IDR_OPEN,_T("��ʾ������"));
		GetWindowRect(m_rectShow);
        DrawAnimatedRects(GetSafeHwnd(),IDANI_CAPTION,m_rectShow,m_rectTray);
		ShowWindow(SW_HIDE); //��С�������̲�����������
		
		return;
	}
	CFrameWnd::OnSysCommand(nID, lParam);
}


//�Զ���������Ϣ��Ӧ����
LRESULT CMainFrame::OnUMTray(WPARAM wParam,LPARAM lParam) 
{ 
	if(wParam!=IDR_MAINFRAME) 
		return 1; 
	switch(lParam) 
	{ 
	case WM_RBUTTONUP: 
		{ 
			LPPOINT lpoint=new tagPOINT; 
			::GetCursorPos(lpoint);//�õ����λ��	    	
			
	    	TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,lpoint->x,lpoint->y,0,this->GetSafeHwnd(),NULL);//(HWND)(AfxGetApp()->m_pMainWnd)
            
			CString cs=_T("");
			m_PopMenu->GetMenuString(IDR_OPEN,cs,MF_BYCOMMAND);
			if(cs=="����������")
			{				
				m_PopMenu->ModifyMenu(IDR_OPEN,MF_BYCOMMAND,IDR_OPEN,_T("��ʾ������"));
			}else
			{
				m_PopMenu->ModifyMenu(IDR_OPEN,MF_BYCOMMAND,IDR_OPEN,_T("����������"));
			}
			delete lpoint; 
		}
		break; 
	case WM_LBUTTONDOWN://��������Ĵ��� 
		{
			if(this->IsWindowVisible()==FALSE)
			{
				DrawAnimatedRects(GetSafeHwnd(),IDANI_CAPTION,m_rectTray,m_rectShow);
			    this->ShowWindow(SW_SHOW);//�򵥵���ʾ���������¶� 
				m_PopMenu->ModifyMenu(IDR_OPEN,MF_BYCOMMAND,IDR_OPEN,_T("����������"));
			}
		} 
		break;
	default: 
		break; 
	} 
	return 0;
 }

//�Զ���״̬����Ϣ��Ӧ����
LRESULT CMainFrame::OnStatusMessage(WPARAM wParam, LPARAM lParam)
{
	this->m_wndStatusBar.SetPaneText(0,(char *)lParam,TRUE);
	return 0;
}
static Addflag=0;
//�Զ���Σ����Ϣ��Ӧ����
LRESULT CMainFrame::OnDangerMessage(WPARAM wParam, LPARAM lParam)
{
	//this->m_wndStatusBar.SetPaneText(0,(char *)lParam,TRUE);
	CString  d_info=_T("");
	CString  info_static=_T("");
	char  *buffer=(char *)wParam;
	char  filter1[]="$$";//regedit
	char  filter2[]="**";//driver
	char  filter3[]="!!";//memory
	char  filter4[]="##";//process
	char  processname[]="ZwCreateSection";
	char  process[MAXPROCESS]={0},fnName[MAXNAMELEN]={0},pathname[MAXPATHLEN]={0};
	char  str[MAXPROCESS]={0};
	int i,j,n,m;

	char ch='$';
	int index=0,len;
	j=0;
	i=0;
	strcpy(info[0].actInfo,"δ֪��Ϊ");


	if(strstr(buffer,filter1))
	{
		ch='$';
		Addflag=1;
		index=0;
		if(strstr(buffer,"CURRENTVERSION\\RUN"))
		{
		    strcpy(info[0].actInfo,"�޸�������");
		}
		else if(strstr(buffer,"CURRENTVERSION\\RUNSERVICE"))
		{
			strcpy(info[0].actInfo,"�޸ķ�����");
		}
		else if(strstr(buffer,"OPEN\\COMMAND	EXE"))
		{
			strcpy(info[0].actInfo,"�����ļ���");
		}
		else if(strstr(buffer,"CURRENTCONTROLSET\\SERVICE"));
		{
			strcpy(info[0].actInfo,"���ط�����");
		}

	}else if(strstr(buffer,filter2))
	{
		ch='*';
		Addflag=2;
		index=1;
		strcpy(info[1].actInfo,"��������");
//		strcpy(str,info[1].info);
	}else if(strstr(buffer,filter3))
	{
		ch='!';
		Addflag=3;
		index=2;
		strcpy(info[2].actInfo,"Զ��ע����뵽�����߳�");
//		strcpy(str,info[2].info);
	}else if(strstr(buffer,filter4))
	{
		ch='#';
		Addflag=4;
		index=3;
		if(strstr(buffer,processname))//��������
		{
			strcpy(info[3].info,info[4].info);
			strcpy(info[3].actInfo,"����һ���������̵Ĳ���");
		}
		else
		{
			if(!(pathname[0] & 0x80))//���Ǻ���
			{
			   strcpy(info[3].info,info[5].info);
			   strcpy(info[3].actInfo,"�����������ڵĹ��ӣ�Ϊ����Ӧ������Ϣ");
			}
			else
			{
				strcpy(info[3].info,"����ȫ�ֹ���");
				strcpy(info[3].actInfo,"����ȫ�ֹ��ӣ����Ṵסϵͳ������Ϣ,��Σ��");
			}
		}
	

	}
	

	while (buffer[i]!=ch)
	{
		process[j++]=buffer[i++];//find the operate process name
	}
	i+=3;
	n=0;
	while (buffer[i]!='\t')
	{
		fnName[n++]=buffer[i++];//find the function name
	}
	i++;
	m=0;
	while (buffer[i]!='\0')
	{
		pathname[m++]=buffer[i++];//find the target path name
	}
	memset(info[index].functionName,0,sizeof(info[index].functionName)/sizeof(char));
	memset(info[index].pathName,0,sizeof(info[index].pathName)/sizeof(char));
	memset(info[index].processName,0,sizeof(info[index].processName)/sizeof(char));
	memset(info[index].suggInfo,0,sizeof(info[index].suggInfo)/sizeof(char));
	
	strncpy(info[index].processName,process,j);
	strncpy(info[index].functionName,fnName,n);
	
	strncpy(info[index].pathName,pathname,m);
	len=strlen(process);
	memset(str,0,MAXPROCESS);
	ChangeUper(process,str,len);

	for(int c=0;c<protectIndex;c++)
	{
		
		if(strstr(str,security[c]))
		{
			strcpy(info[index].suggInfo,"����һ����ȫ�Ķ�������������ȡ����");
			break;
		}
	}
	if(c==protectIndex)
		strcpy(info[index].suggInfo,"����һ��δ֪�Ķ���");

	d_info.Format("%s %s %s\r\n%s:%s",info[index].processName,info[index].info,info[index].pathName,"��Ϊ����",info[index].actInfo);
	info_static.Format("\r\n    %s:%s","��ʾ��Ϣ",info[index].suggInfo);


	if(dangerDlg==NULL)
	{

		dangerDlg=new CDangerDlg();

		if(dangerDlg != NULL)
		{
			BOOL ret = ((CDialog*)dangerDlg)->Create(IDD_DANGER);
			if(!ret)   //Create failed.
				AfxMessageBox("Error creating Dialog");
			dangerDlg->m_infoStatic.SetWindowText((LPCTSTR)info_static);
			dangerDlg->m_infoEdit.SetWindowText((LPCTSTR)d_info);
			dangerDlg->ShowWindow(SW_SHOW);
		}
		//else
			//AfxMessageBox("Error Creating Dialog Object");
	}
	dangerDlg->m_infoStatic.SetWindowText((LPCTSTR)info_static);

	dangerDlg->m_infoEdit.SetWindowText((LPCTSTR)d_info);
	dangerDlg->ShowWindow(SW_SHOW);
	((CWnd *)dangerDlg)->Invalidate();
	return 0;
}

//�Զ����û�ѡ��ť����Ϣ��Ӧ����
LRESULT CMainFrame::OnChoiceMessage(WPARAM wParam, LPARAM lParam)
{
	char dst[20]={0};
	int len=0;

	
	if (lParam==4)//��Ӱ�����.
	{
		char str[]="ע���";
		CFile file;
		if(Addflag==1)//���ע�������
		{
			file.Open("c:\\protect.txt",CFile::modeReadWrite);
			file.SeekToEnd();
			file.Write("\r\n",2);
			file.Write(info[0].pathName,strlen(info[0].pathName));
			file.Write("\t",1);
			file.Write(str,strlen(str));
			file.Close();
		}
		else if(Addflag==2)//��������
		{
			file.Open("c:\\whitelist.txt",CFile::modeReadWrite);
			file.SeekToEnd();
			file.Write("\r\n",2);
			len = strlen(info[1].processName);
			ChangeUper(info[1].processName,dst,len);
			file.Write(dst,strlen(dst));
			file.Write("\t",1);
			file.Write(info[1].functionName,strlen(info[1].functionName));
			file.Close();
		}
		else if(Addflag==3)
		{
			file.Open("c:\\whitelist.txt",CFile::modeReadWrite);
			file.SeekToEnd();
			file.Write("\r\n",2);
			len = strlen(info[2].processName);
            ChangeUper(info[2].processName,dst,len);
		
			file.Write(dst,strlen(dst));
			file.Write("\t",1);
			file.Write(info[2].functionName,strlen(info[2].functionName));
			file.Close();
		}
		else if(Addflag==4)
		{
			file.Open("c:\\whitelist.txt",CFile::modeReadWrite);
			file.SeekToEnd();
			file.Write("\r\n",2);
			len = strlen(info[3].processName);
			ChangeUper(info[3].processName,dst,len);
			file.Write(dst,strlen(dst));
			file.Write("\t",1);
			file.Write(info[3].functionName,strlen(info[3].functionName));
			file.Close();
		}
	}
	choice=lParam;
	if(dangerDlg!=NULL)
	{
		dangerDlg->ShowWindow(SW_HIDE);
	}

	return 0;
}



void CMainFrame::OnClose() 
{
	//��̬��С��������
	GetWindowRect(m_rectShow);
	DrawAnimatedRects(GetSafeHwnd(),IDANI_CAPTION,m_rectShow,m_rectTray);

	m_PopMenu->ModifyMenu(IDR_OPEN,MF_BYCOMMAND,IDR_OPEN,_T("��ʾ������"));
	this->ShowWindow(SW_HIDE);
	
}

void CMainFrame::OnOpen() 
{
	
	if(this->IsWindowVisible())
	{
		GetWindowRect(m_rectShow);
        DrawAnimatedRects(GetSafeHwnd(),IDANI_CAPTION,m_rectShow,m_rectTray);
		this->ShowWindow(SW_HIDE);
	}
	else
	{
		DrawAnimatedRects(GetSafeHwnd(),IDANI_CAPTION,m_rectTray,m_rectShow);
		this->ShowWindow(SW_SHOW);
	}
}

void CMainFrame::OnExit() 
{
	DWORD bytesReturned=0;
	BOOL ret=0;
	//DWORD wd;

	flag=TRUE;
	flag2=FALSE;
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
			//AfxMessageBox("�������豸ʧ��!");
		}
	}
	
	ret=DeviceIoControl(driverHandle,
		IO_CLEAR_EVENT,
		NULL,
		0,
		NULL,
		0,
		&bytesReturned,
		NULL);//ͬ������	
	if(!driverHandle) CloseHandle(driverHandle);//���ͨ���¼�
	
	//ɾ������
	Shell_NotifyIcon(NIM_DELETE,&nid);
	
	//free(driverDosName);
	   
	//GetExitCodeThread(handleDanger,&wd);
	//TerminateThread(handleDanger,wd);
	//GetExitCodeThread(handleOutput,&wd);
	//TerminateThread(handleOutput,wd);

	if(beginMonitor==1)
	{
	OnUnhook();
	beginMonitor=0;
	}
	Sleep(2000);
	OnUnloadHostDriver(); //ж������������������
	Sleep(2000);
	::PostQuitMessage(0);
}


BOOL CMainFrame::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    UINT nID =pNMHDR->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
        if(nID)
        {
            pTTT->lpszText = MAKEINTRESOURCE(nID);
            pTTT->hinst = AfxGetResourceHandle();
            return(TRUE);
        }
    }
    return(FALSE);
}

void CMainFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();
}


//�������̴���
BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam)
{
    TCHAR szClassName[256];
    GetClassName(hwnd, szClassName, 255);

    // �Ƚϴ�������
    if (_tcscmp(szClassName, _T("TrayNotifyWnd")) == 0)
    {
        CRect *pRect = (CRect*) lParam;
        ::GetWindowRect(hwnd, pRect);
        return TRUE;
    }

    // ���ҵ�ʱ�Ӵ���ʱ��ʾ���Խ�����
    if (_tcscmp(szClassName, _T("TrayClockWClass")) == 0)
    {
        CRect *pRect = (CRect*) lParam;
        CRect rectClock;
        ::GetWindowRect(hwnd, rectClock);
        pRect->right = rectClock.left;
        return FALSE;
    }
 
    return TRUE;
}


void CMainFrame::LoadHostDriver() 
{


    char aPath[1024];
	char aCurrentDirectory[515];
	SC_HANDLE sh,service;
	SERVICE_STATUS stat;

	sh=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
	
	if(!sh)
	{
		//MessageBox("�޷��򿪷�����ƹ�������");
		return;
	}
	GetCurrentDirectory(512,aCurrentDirectory);
	_snprintf(aPath,1022,"%s\\%s.sys",aCurrentDirectory,"protect");

	//MessageBox(aPath);
	
	//��������Ѵ��ڣ���ɾ��֮
	service=OpenService(sh,"protectservice",SERVICE_ALL_ACCESS);
	if(service)
	{
		//MessageBox("Impossible");
		if(ControlService(service,SERVICE_CONTROL_STOP,&stat)==0)
			//MessageBox("Error!");
		DeleteService(service);
    	CloseServiceHandle(service);
	}
	//���´�������
	service=CreateService(sh,
		                  "protectservice",
						  "protectservice",
						  SERVICE_ALL_ACCESS,
						  SERVICE_KERNEL_DRIVER,
						  SERVICE_DEMAND_START,
						  SERVICE_ERROR_NORMAL,
						  aPath,
						  0,
						  0,
						  0,
						  0,
						  0);

	if(service==NULL)
	{
		CloseServiceHandle(sh);
		/*switch( GetLastError())
		{
		case ERROR_ACCESS_DENIED:
			MessageBox("ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			MessageBox("ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME :
			MessageBox("ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE:
			MessageBox("ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME:
			MessageBox("ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER:
			MessageBox("ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			MessageBox("ERROR_INVALID_SERVICE_ACCOUNT");
			break; 
		case ERROR_SERVICE_EXISTS:
			MessageBox("ERROR_SERVICE_EXISTS");
			break;
		default:
			CString cs=_T("");
			cs.Format("%d",GetLastError());
			//MessageBox("��֪����Error"+cs);
			break;
		}*/
        //MessageBox("��������ʧ�ܣ�");
		return;
	}

	if(0==StartService(service,0,0))
	{
		//MessageBox("��������ʧ��");
	}
	else
		//MessageBox("�ɹ���������!");
	CloseServiceHandle(sh);
	CloseServiceHandle(service);

}

//�Զ���Σ����Ϣ��Ӧ����
LRESULT CMainFrame::OnProcessMessage(WPARAM wParam, LPARAM lParam)
{
	int length=sizeof(processbuffer);

//	struct_process *p_process;

	((unsigned char*)processbuffer)[0]=0;

	if(hDevice==NULL)
	{
		hDevice = CreateFile( "\\\\.\\protect",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_SYSTEM,
		NULL); 
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			SetEvent(hEvent);
			//MessageBox(TEXT("INVALID DEVICE"));	
			return 0;
		}
	}

	BOOL ret=DeviceIoControl(hDevice,REGMON_getprocess,NULL,0,&processbuffer,length,&buflen,NULL);
   
	if (!ret)
	{
		SetEvent(hEvent);
		//::AfxMessageBox("send get status ioctl error");
		return 0;
	}
	
	SetEvent(hEvent);
	return 0;
}

void CMainFrame::OnHook() 
{

	//MessageBox("Hooking.....");
	DWORD dw,i,length;
	BOOL ret;
	DWORD input[5];
	DWORD index[2];
	length = sizeof(buffer);
	hDevice = CreateFile( "\\\\.\\protect",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_SYSTEM,
		NULL); 



	if (hDevice == INVALID_HANDLE_VALUE) {

		//MessageBox(TEXT("INVALID DEVICE"));
		return ;}
	memset(output,0,MAXBUFLENGTH);
	
	HMODULE handle =GetModuleHandle("ntdll.dll");
	//index[0]=*(DWORD *)(1+(DWORD)GetProcAddress( handle,"NtCreateProcessEx"));//�õ�����ָ�������ĵ�ַ����������������
    index[0]=*(DWORD *)(1+(DWORD)GetProcAddress( handle,"NtWriteVirtualMemory"));
	//input[2]=index[1];
	input[0]=(DWORD)&output[0];
	input[1]=index[0];
	input[2]=GetCurrentProcessId();//���ݽ���ID

	//����ͨ���¼�
	eventReceive = CreateEvent(NULL, false, false, NULL);
	eventSend = CreateEvent(NULL, false, false, NULL);

	if(eventReceive==NULL||eventSend==NULL)
	{
		//MessageBox("����ͬ���¼�ʧ��!");
	}
	input[3]=(DWORD)eventReceive;
	input[4]=(DWORD)eventSend;
	
	//GetWindowThreadProcessId(AfxGetMainWnd()->m_hWnd,&input[3]);//���ݽ���ID

	
	//CString cs;
	//cs.Format("%d",input[2]);
	//MessageBox(cs);

	ret=DeviceIoControl(hDevice,REGMON_hook,input,sizeof(input),NULL,0,&i,NULL);
	if (!ret)
	{
		//MessageBox(TEXT("send ioctl error"));
		return ;
	}
	//MessageBox(TEXT("hook ok"));

	handleDanger=CreateThread(0,0,(LPTHREAD_START_ROUTINE)thread,&output,0,&dw);
	
}

void CMainFrame::OnUnhook() 
{
	BOOL ret;
	DWORD i;
	ret=DeviceIoControl(hDevice,REGMON_unhook,NULL,0,NULL,0,&i,NULL);
	if (!ret)
	{
		//MessageBox(TEXT("send ioctl error"));
		return ;
	}
	//MessageBox(TEXT("unhook ok"));
	
}

void CMainFrame::OnUnloadHostDriver() 
{
	char aPath[1024];
	char aCurrentDirectory[515];
	SC_HANDLE sh,service;
	SERVICE_STATUS stat;

	GetCurrentDirectory(512,aCurrentDirectory);
	_snprintf(aPath,1022,"%s\\%s.sys",aCurrentDirectory,"protect");


	if((sh=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS))==NULL)
	{
		//MessageBox("�޷��򿪷�����ƹ�������");
		return;
	}
	if((service=OpenService(sh,"protectservice",SERVICE_ALL_ACCESS))==NULL)
	{
		CloseServiceHandle(sh);
		//MessageBox("�򿪷���ʧ�ܣ�");
		return;
	}
	if(ControlService(service,SERVICE_CONTROL_STOP,&stat)==0)
	{
		CloseServiceHandle(sh);
		CloseServiceHandle(service);
		//MessageBox("���Ʒ���ʧ�ܣ�");
		return;
	}

	/*while(1)
	{
		if((sh=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS))==NULL)
	{
		MessageBox("�޷��򿪷�����ƹ�������");
		return;
	}else if((service=OpenService(sh,"protectservice",SERVICE_ALL_ACCESS))==NULL)
	{
		CloseServiceHandle(sh);
		MessageBox("�򿪷���ʧ�ܣ�");
		break;
	}
		Sleep(300);
		if(QueryServiceStatus( service, &stat)==0)
		{
			CloseServiceHandle(sh);
			CloseServiceHandle(service);
			MessageBox("��ѯ״̬ʧ�ܣ�");
		    return;
		}
		if(stat.dwCurrentState == SERVICE_STOPPED)
			break;
	    else //if(stat.dwCurrentState != SERVICE_STOP_PENDING)
		{
			if(ControlService(service,SERVICE_CONTROL_STOP,&stat)==0)
			{
				CloseServiceHandle(sh);
		    	CloseServiceHandle(service);
			    MessageBox("ֹͣ����ʧ�ܣ�");
		        break;
			}
		}
	}*/

    if(DeleteService(service)==0)
	{
		CloseServiceHandle(sh);
		CloseServiceHandle(service);
		//MessageBox("�豸����ж��ʧ�ܣ�");
		return;
	}  

//	CloseServiceHandle(sh);
	CloseServiceHandle(service);
	//MessageBox("�ɹ�ж�أ�");
}


void CMainFrame::OnOutput() 
{
	//MessageBox("Output!!!");
	outflag=1;
	//CantiDlg *Dlg=this;
    AfxBeginThread((AFX_THREADPROC)outputproc,this);	
}

//------------------------------------------------------------
// Volume2Label
// ��������ת��Ϊ���
// ������uiNum-������ szResult-���ؾ��(eg.c:\)
// -----------------------------------------------------------
void Volume2Label(unsigned int uiNum,char szResult[])
{
	char szBuf[256]={0};
	char * pBuf=szBuf;
	szResult[0]='\0';
	GetLogicalDriveStrings(256,szBuf);
	if(uiNum>64)
		return;
	pBuf=pBuf+(uiNum-1)*4;
	if(!pBuf)
		return;
	if(GetDriveType(pBuf)==DRIVE_FIXED||GetDriveType(pBuf)==DRIVE_REMOVABLE)
	{
		strcpy(szResult,pBuf);
		return;
	}	
	while(pBuf)
	{
		if(GetDriveType(pBuf)!=DRIVE_FIXED||GetDriveType(pBuf)==DRIVE_REMOVABLE)
		{
			pBuf=pBuf+4;
		}
		else
		{
			strcpy(szResult,pBuf);
			return;
		}
	}
	return;
}

void  outputproc(LPVOID  param)//����һ���µ��̣߳�������ԭ���ģ����Բ���������ı���
{
	
	//CMainFrame *pData=(CMainFrame *)param;
	PENTRY EntryData;
	char process[MAXPROCESS];
	//char process[30]; //����process��name������30���ַ�
	char name[MAXNAMELEN];
	//char name[30];
	char path[MAXPATHLEN];
	char str[256]={0};	

	BOOL ret;
	int length =sizeof(buffer);
	int num=0,j=0,m=0;
   while(1)//��unhook�󣬻��ǻ�ѭ�����Ƿ����ڴ�й©��
   {
	   if (flag)//���flagҪ��Ҫ������
	   {
		   return;
	   }

	   if(checkhost==false)
	   {
		   continue;
	   }

	    
		ret=DeviceIoControl(hDevice,REGMON_getstats,NULL,0,&buffer,length,&buflen,NULL);
		if (!ret)
		{
			//::AfxMessageBox("send get status ioctl error");
			break;
		}
		for ( EntryData = (PENTRY)buffer; (char *)EntryData < min(buffer+buflen,buffer + sizeof (buffer)); )
		{
			j=0;
			num++;
			//memset(process,' ',sizeof(process));
			//process[29]='\0';


		

			while (EntryData->text[j]!='\t')//get the process name
			{
				process[j]=EntryData->text[j];
				j++;
			}
			process[j]='\0';

			j++;

			m=0;
			//memset(name,' ',sizeof(name));
			//name[29]='\0';

			while (EntryData->text[j]!='\t')//get the function name
			{
				name[m++]=EntryData->text[j++];
			}
			name[m]='\0';
			j++;
			m=0;
			memset(path,0,sizeof(path));
			while ((EntryData->text[j]!='\t')&&(EntryData->text[j]!='\0'))//get the path name
			{
				path[m++]=EntryData->text[j++];
			}
			path[m]='\0';
	
		
			memset(str,0,sizeof(str));
			
			str[0]='\0';
			strcat(str,process);
			strcat(str,"\t  ");
			strcat(str,name);
			strcat(str,"\t  ");
			//AfxMessageBox(str);
		
	
			int loc;
			char harddisk[]="\\device\\harddiskvolume";
		
			for(loc=0;loc<22;loc++)
			{
				if(tolower(path[loc])!=harddisk[loc])
					break;
			}
			
			if(loc>=22)
			{
				char result[10]={0};
				Volume2Label(path[22]-'0',result);	
				
				strcat(str,result);
				
				strcat(str,&path[24]);
				
			}
			else
				strcat(str,path);


			if(p_this!=NULL&&checkhost)
			{
				PostMessage(p_this->m_hWnd,WM_USER +13,(WPARAM)0,(LPARAM)str);//UM_MESSLIST(LPCTSTR)str
			}
			else
				break;
		

			ULONG len = strlen(EntryData->text);
			len+=1;
			EntryData = (PENTRY)(EntryData ->text + len);
		}
	
		Sleep(1000);
   }


}


void thread(LPVOID pdata)
{
	char *param=(char *)pdata;
	char   save[1024];
	//CString  str;
	DWORD a=0;
	int n=0,i;
	while (flag2)
	{
		
        memset(save,0,1024);
		WaitForSingleObject(eventSend, INFINITE);

		//memmove(&a,&param[0],4);//begin send the flag,then at last to see the flag is changed?
		//if(!a)
		//{
	    //	Sleep(100);
		//	continue;
		//}

		//char *procname=(char *)&param[8];
		
        n=0;
	    while (param[n]!='\t')
		{
			n++;
		}
//		procname[n]='\0';
		n++;
		while (param[n]!='\t')  n++;
		
		strncpy(save,param,n);//�����ǽ������ͺ�����
		for(i=0;i<index;i++)
		{
			if(!stricmp(save,table[i]))
			{
				a=num[i];
				goto skip;
			}
		}
		table[index]=(char *)malloc(n*sizeof(char));//�����ڴ棬save����
		memset(table[index],0,n);
		strncpy(table[index],save,n);
		table[index][n]='\0';
	


	if(AfxGetMainWnd())
		PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+25,(WPARAM)param,(LPARAM)0);//send the buffer to deal with
	while(choice==0)
	{
		Sleep(100);
	}
	
		//	choice=1:���� 2 ���ܾ� 3���������� 4����Ӱ�����
//	str.Format("%d",choice);
//	AfxMessageBox(str);
		a=choice;
		num[index]=a;
		index++;
	
skip:	memmove(&param[0],&a,4);//memmove(&param[4],&a,4);

		SetEvent(eventReceive);

		//a=0;
		//memmove(&param[0],&a,4);//let the thread  run again

		choice=0;

	}
}


void CMainFrame::ChangeUper(char *src,char *dst,int len)
{
     int i=0;
//	 MessageBox(src);
	 for(i=0;i<len;i++)
	 {
		 if(( src[i] >= 'a' && src[i] <= 'z' ) )
		 {
			 dst[i] = src[i] - 'a' + 'A';
		 }
		 else
		 {
			 dst[i]=src[i];
		 }
	 }
	 dst[i]='\0';
}

LRESULT CMainFrame::BeginMonitor(WPARAM wParam, LPARAM lParam)//��ʼ���
{
	DWORD  ret,i;
	if(beginMonitor==0)
	{
		if(outflag==0)//�鿴�����û�д�
		{
			OnOutput();
			outflag=1;
		}
		ret=DeviceIoControl(hDevice,Begin_hook,NULL,0,NULL,0,&i,NULL);
		if (!ret)
	{
		//MessageBox(TEXT("send ioctl error"));
		return 0;
	}
	//MessageBox(TEXT("unhook ok"));
		
	     beginMonitor=1;
	}
	return 0;
}

LRESULT CMainFrame::EndMonitor(WPARAM wParam, LPARAM lParam)
{

	if(beginMonitor==1)
	{

	OnUnhook();
	beginMonitor=0;
	}
	return 0;
	
}

void CMainFrame::OnHelp() 
{
	ShellExecute(0,"open","Help.chm",NULL,NULL,SW_SHOW);
	return;	
}
