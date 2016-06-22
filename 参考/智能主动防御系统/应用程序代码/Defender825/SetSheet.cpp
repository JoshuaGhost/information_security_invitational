// SetSheet.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "SetSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetSheet

IMPLEMENT_DYNAMIC(CSetSheet, CPropertySheet)

CSetSheet::CSetSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pageSet);
}

CSetSheet::CSetSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pageSet);
}

CSetSheet::~CSetSheet()
{
}


BEGIN_MESSAGE_MAP(CSetSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CSetSheet)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetSheet message handlers

void CSetSheet::OnPaint() 
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

void CSetSheet::OnDestroy() 
{
	CPropertySheet::OnDestroy();
		
}
