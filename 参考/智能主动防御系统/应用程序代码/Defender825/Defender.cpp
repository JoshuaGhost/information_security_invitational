// Defender.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Defender.h"

#include "MainFrm.h"
#include "DefenderDoc.h"
#include "DefenderView.h"
#include "LoadcodeDlg.h"

#include "DangerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char szCurrentDirectory[256]={0};
/////////////////////////////////////////////////////////////////////////////
// CDefenderApp

BEGIN_MESSAGE_MAP(CDefenderApp, CWinApp)
	//{{AFX_MSG_MAP(CDefenderApp)
	//ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	//ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefenderApp construction

CDefenderApp::CDefenderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDefenderApp object

CDefenderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDefenderApp initialization

BOOL CDefenderApp::InitInstance()
{
	//防止程序的多个实例同时运行
	HANDLE hMutex=CreateMutex(NULL,FALSE,"DefenderOnlyOnce");//命名Mutex是全局对象
	if(ERROR_ALREADY_EXISTS==::GetLastError())
	{     
	   AfxMessageBox("该程序已经在运行中...");
	   ExitProcess(1);
	}     


	//获取当前路径
	GetModuleFileName(this->m_hInstance,szCurrentDirectory,256);
	int i=strlen(szCurrentDirectory)-1;
	for(;i>=1;i--)
	{
		if(szCurrentDirectory[i]!='\\') 
			szCurrentDirectory[i]=0;
		else
			break;
	}

	//打开设备驱动，加载特征码
	CLoadcodeDlg loadcodedlg;
	loadcodedlg.DoModal();


	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDefenderDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDefenderView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	
	m_pMainWnd->SetWindowText("智能主动防御系统");
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


