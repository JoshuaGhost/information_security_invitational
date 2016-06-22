// DefenderView.cpp : implementation of the CDefenderView class
//

#include "stdafx.h"
#include "Defender.h"

#include "DefenderDoc.h"
#include "DefenderView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern int start;
//extern void OnHook();

/////////////////////////////////////////////////////////////////////////////
// CDefenderView

IMPLEMENT_DYNCREATE(CDefenderView, CView)

BEGIN_MESSAGE_MAP(CDefenderView, CView)
	//{{AFX_MSG_MAP(CDefenderView)
	ON_WM_PAINT()
	ON_COMMAND(IDM_PROTECT, OnProtect)
	ON_COMMAND(IDM_HELP, OnHelp)
	ON_COMMAND(IDM_LOG, OnLog)
	ON_COMMAND(IDM_SET, OnSet)
	ON_UPDATE_COMMAND_UI(IDM_LOG, OnUpdateLog)
	ON_UPDATE_COMMAND_UI(IDM_PROTECT, OnUpdateProtect)
	ON_UPDATE_COMMAND_UI(IDM_SET, OnUpdateSet)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
//	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
//	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
//	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefenderView construction/destruction

CDefenderView::CDefenderView()
{
	// TODO: add construction code here
	m_intCurrent=0;
	m_intBefore=0;

	
	protectSheet=NULL;
	setSheet=NULL;
	logSheet=NULL;
	//helpSheet=NULL;
	
}

CDefenderView::~CDefenderView()
{
}

BOOL CDefenderView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CDefenderView drawing

void CDefenderView::OnDraw(CDC* pDC)
{
	CDefenderDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}



/////////////////////////////////////////////////////////////////////////////
// CDefenderView diagnostics

#ifdef _DEBUG
void CDefenderView::AssertValid() const
{
	CView::AssertValid();
}

void CDefenderView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDefenderDoc* CDefenderView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDefenderDoc)));
	return (CDefenderDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDefenderView message handlers

void CDefenderView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	CRect rect;
	this->GetClientRect(&rect);


	switch(m_intCurrent)
	{
	case 0:
		//m_intCurrent=1;
		OnProtect();
	case 1:
		
		
		protectSheet->MoveWindow(rect);		
		break;
	case 2:
		setSheet->MoveWindow(rect);		
		break;
	case 3:
		logSheet->MoveWindow(rect);
		break;
	//case 4:
	//	helpSheet->MoveWindow(rect);
	//	break;
	default:
		break;
	}


	// Do not call CView::OnPaint() for painting messages
}




//保护按钮响应 
void CDefenderView::OnProtect() 
{
	if(m_intCurrent==1)
		return;
	((CFrameWnd*)AfxGetMainWnd())->SetWindowText("智能主动防御系统-保护");
	m_intBefore=m_intCurrent;

	if(protectSheet==NULL)
	{
		protectSheet=new CProtectSheet("保护");
	    protectSheet->Create(this,WS_CHILD);//WS_OVERLAPPED
	}

	switch(m_intBefore)
	{
	case 2:
		setSheet->ShowWindow(SW_HIDE);
		break;
	case 3:
		logSheet->ShowWindow(SW_HIDE);
		break;
	case 4:
		//helpSheet->ShowWindow(SW_HIDE);
		break;
	default:
		break;
	}
	protectSheet->ShowWindow(SW_SHOW);
	
	m_intCurrent=1;

}

void CDefenderView::OnSet() 
{
	if(m_intCurrent==2)
		return;
	((CFrameWnd*)AfxGetMainWnd())->SetWindowText("智能主动防御系统-设置");
	m_intBefore=m_intCurrent;

	if(setSheet==NULL)
	{
		setSheet=new CSetSheet("设置");
	    setSheet->Create(this,WS_CHILD);//WS_OVERLAPPED
	}

	switch(m_intBefore)
	{
	case 1:
		
		protectSheet->ShowWindow(SW_HIDE);
		break;
	case 3:
		logSheet->ShowWindow(SW_HIDE);
		break;
	case 4:
		//helpSheet->ShowWindow(SW_HIDE);
		//break;
	default:
		break;
	}
	setSheet->ShowWindow(SW_SHOW);
	
	m_intCurrent=2;
}


void CDefenderView::OnLog() 
{
	if(m_intCurrent==3)
		return;
	

	((CFrameWnd*)AfxGetMainWnd())->SetWindowText("智能主动防御系统-日志");
	m_intBefore=m_intCurrent;

	if(logSheet==NULL)
	{
		logSheet=new CLogSheet("日志");
	    logSheet->Create(this,WS_CHILD);//WS_OVERLAPPED
	}

	switch(m_intBefore)
	{
	case 1:
		protectSheet->ShowWindow(SW_HIDE);
		break;
	case 2:
		setSheet->ShowWindow(SW_HIDE);
		break;
	case 4:
		break;
	default:
		break;
	}
	logSheet->ShowWindow(SW_SHOW);
	
	m_intCurrent=3;

}

void CDefenderView::OnHelp() 
{
	ShellExecute(0,"open","Help.chm",NULL,NULL,SW_SHOW);
}



void CDefenderView::OnUpdateProtect(CCmdUI* pCmdUI) 
{
	if(m_intCurrent==1)
	{
		pCmdUI->Enable();  
        pCmdUI->SetCheck(TRUE);
	}else
	{
		pCmdUI->SetCheck(FALSE);
	}
}

void CDefenderView::OnUpdateSet(CCmdUI* pCmdUI) 
{
	if(m_intCurrent==2)
	{
		pCmdUI->Enable();  
        pCmdUI->SetCheck(TRUE);
	}else
	{
		pCmdUI->SetCheck(FALSE);
	}
}

void CDefenderView::OnUpdateLog(CCmdUI* pCmdUI) 
{
    if(m_intCurrent==3)
	{
		pCmdUI->Enable();  
        pCmdUI->SetCheck(TRUE);
	}else
	{
		pCmdUI->SetCheck(FALSE);
	}
}

void CDefenderView::OnDestroy() 
{
	CView::OnDestroy();
	
	// TODO: Add your message handler code here
	/*if(protectSheet!=NULL)
	{
		delete protectSheet;
	    protectSheet=NULL;
	}

	if(setSheet!=NULL)
	{
		delete setSheet;
	    setSheet=NULL;
	}

	if(logSheet!=NULL)
	{
		delete logSheet;
	    logSheet=NULL;
	}*/

}
