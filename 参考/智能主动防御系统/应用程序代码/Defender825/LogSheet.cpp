// LogSheet.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "LogSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogSheet

IMPLEMENT_DYNAMIC(CLogSheet, CPropertySheet)

CLogSheet::CLogSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pageLog);
}

CLogSheet::CLogSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pageLog);
}

CLogSheet::~CLogSheet()
{
}


BEGIN_MESSAGE_MAP(CLogSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CLogSheet)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogSheet message handlers

void CLogSheet::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rect;

    GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y+30;
	CPoint pointTop(x,y);
	
    GetActivePage()->MoveWindow(CRect(pointTop,rect.BottomRight()));//


}

void CLogSheet::OnDestroy() 
{
	CPropertySheet::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
