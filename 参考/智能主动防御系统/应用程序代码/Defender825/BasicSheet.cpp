// BasicSheet.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "BasicSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBasicSheet

IMPLEMENT_DYNAMIC(CBasicSheet, CPropertySheet)

CBasicSheet::CBasicSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pageLocal);
}

CBasicSheet::CBasicSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pageLocal);
}

CBasicSheet::~CBasicSheet()
{
}


BEGIN_MESSAGE_MAP(CBasicSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CBasicSheet)
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBasicSheet message handlers

BOOL CBasicSheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	// TODO: Add your specialized code here
	//this->Create(NULL,WS_OVERLAPPED);

	CRect rect;

    GetParent()->GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y+30;
	CPoint pointTop(x,y);
	
    GetActivePage()->MoveWindow(CRect(pointTop,rect.BottomRight()));//
	return bResult;
}

//void CBasicSheet::OnRButtonDown(UINT nFlags, CPoint point) 
//{
//	// TODO: Add your message handler code here and/or call default
//	MessageBox("OK");
//	CPropertySheet::OnRButtonDown(nFlags, point);
//}

void CBasicSheet::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
//	
//	// TODO: Add your message handler code here

	CRect rect;

    GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y+30;
	CPoint pointTop(x,y);
	
    GetActivePage()->MoveWindow(CRect(pointTop,rect.BottomRight()));//

	
	
	// Do not call CPropertySheet::OnPaint() for painting messages
}

void CBasicSheet::OnDestroy() 
{
	CPropertySheet::OnDestroy();
	
}
