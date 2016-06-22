// ProtectSheet.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "ProtectSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProtectSheet

IMPLEMENT_DYNAMIC(CProtectSheet, CPropertySheet)

CProtectSheet::CProtectSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	
	AddPage(&m_pageLocal);
	AddPage(&m_pageProcess);
	AddPage(&m_pageConnect);
	AddPage(&m_pageService);
	AddPage(&m_pageAutorun);
	AddPage(&m_pageHosts);		
    
}

CProtectSheet::CProtectSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pageLocal);
	AddPage(&m_pageProcess);
	AddPage(&m_pageConnect);	
	AddPage(&m_pageService);
	AddPage(&m_pageAutorun);
	AddPage(&m_pageHosts);	
	
}

CProtectSheet::~CProtectSheet()
{
}


BEGIN_MESSAGE_MAP(CProtectSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CProtectSheet)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProtectSheet message handlers

void CProtectSheet::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rect;

    GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y+30;
	CPoint pointTop(x,y);
	
    GetActivePage()->MoveWindow(CRect(pointTop,rect.BottomRight()));//

	

	// Do not call CPropertySheet::OnPaint() for painting messages
}

void CProtectSheet::OnDestroy() 
{
	CPropertySheet::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
