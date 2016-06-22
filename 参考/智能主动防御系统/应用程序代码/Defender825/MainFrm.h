// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__D7B80817_36B1_46E1_9828_ADCD97A558B7__INCLUDED_)
#define AFX_MAINFRM_H__D7B80817_36B1_46E1_9828_ADCD97A558B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxwin.h"
#include "afxcmn.h"
#include "ioctl.h"
#include <winioctl.h>
#include "Winsvc.h"


#define MAXPROCESS 20
#define MAXNAMELEN 32
#define MAXPATHLEN 1024
#define INFOLEN    512
#define MAXBUFLENGTH 2048
#define protectIndex  10

typedef  struct _PROTECT_INFO
{
	char  symbol[3];
	char  processName[MAXPROCESS];
	char  functionName[MAXNAMELEN];
	char  info[MAXBUFLENGTH];
	char  pathName[MAXPATHLEN];
	char  suggInfo[MAXBUFLENGTH];
	char  actInfo[MAXBUFLENGTH];
}
PROTECT_INFO,*PPROTECT_INFO;

static       HANDLE hDevice=NULL;
static       DWORD buflen;
static       BOOL   flag=FALSE;
static       BOOL   flag2=TRUE;
static       bool process_flag=true;
static       HANDLE  hEvent;//hMutex,
static       PROTECT_INFO  info[6];

static       char  *table[1024];
static       int    num[1024];
static       int    index=0;
static       int    outflag=0;

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	int beginMonitor;
	char output[MAXBUFLENGTH];

	void LoadHostDriver();
	void OnHook();
	void OnUnhook();
	void OnUnloadHostDriver(); 
	void OnOutput() ;

	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;

#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	//CToolBar    m_wndToolBar;
	//CToolBar    m_coldToolBar;
	CToolBar    m_hotToolBar;
    char *security[protectIndex];
private:
	void ChangeUper(char *,char *,int);
	CRect m_rectShow;
	CRect m_rectTray;
	void ToTray();
	

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnOpen();
	afx_msg void OnExit();
	afx_msg void OnDestroy();
	afx_msg void OnHelp();
	//}}AFX_MSG

	afx_msg LRESULT OnProcessMessage(WPARAM wParam,LPARAM lParam) ;         //显示进程响应函数
	afx_msg LRESULT OnChoiceMessage(WPARAM wParam,LPARAM lParam) ;         //用户选择消息响应函数
	afx_msg LRESULT OnDangerMessage(WPARAM wParam,LPARAM lParam) ;         //危险进程消息响应函数
	afx_msg LRESULT OnUMTray(WPARAM wParam,LPARAM lParam) ;         //托盘消息响应函数
	afx_msg LRESULT OnStatusMessage(WPARAM wParam, LPARAM lParam);  //状态栏实时显示
    afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);//工具条提示信息
    afx_msg LRESULT BeginMonitor(WPARAM wParam,LPARAM lParam) ;
	afx_msg LRESULT EndMonitor(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__D7B80817_36B1_46E1_9828_ADCD97A558B7__INCLUDED_)
