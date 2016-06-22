// LoadcodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "LoadcodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadcodeDlg dialog


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
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadcodeDlg message handlers
