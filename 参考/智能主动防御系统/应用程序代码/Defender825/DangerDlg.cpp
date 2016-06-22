// DangerDlg1.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "DangerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDangerDlg dialog


CDangerDlg::CDangerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDangerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDangerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDangerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDangerDlg)
	DDX_Control(pDX, IDC_INFO_STATIC, m_infoStatic);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_INFO_EDIT, m_infoEdit);
}



BEGIN_MESSAGE_MAP(CDangerDlg, CDialog)
	//{{AFX_MSG_MAP(CDangerDlg)
	ON_BN_CLICKED(IDC_END_BUTTON, OnEndButton)
	ON_BN_CLICKED(IDC_REFUSE_BUTTON, OnRefuseButton)
	ON_BN_CLICKED(IDC_IGNORE_BUTTON, OnIgnoreButton)
	ON_BN_CLICKED(IDC_ADD_BUTTON, OnAddButton)
	ON_WM_SHOWWINDOW()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDangerDlg message handlers




void CDangerDlg::OnIgnoreButton() 
{
	// TODO: Add your control notification handler code here
	m_choice=1;
	//::PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+14,(WPARAM)0,(LPARAM)m_choice);
}

void CDangerDlg::OnRefuseButton() 
{
	// TODO: Add your control notification handler code here
	m_choice=2;
	//::PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+14,(WPARAM)0,(LPARAM)m_choice);
}

void CDangerDlg::OnEndButton() 
{
	// TODO: Add your control notification handler code here
	m_choice=3;
	//::PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+14,(WPARAM)0,(LPARAM)m_choice);
}

void CDangerDlg::OnAddButton() 
{
	// TODO: Add your control notification handler code here
	m_choice=4;

	//::PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+14,(WPARAM)0,(LPARAM)m_choice);
}

void CDangerDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	CRect rectDesktopWithoutTaskbar;   // The desktop area 

    // Get the desktop area
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktopWithoutTaskbar, 0);
    
    // Calculate the actual width of the Window and its position in screen co-ordinates
   int  m_nWndLeft   = rectDesktopWithoutTaskbar.right - 300;
    int m_nWndTop    = rectDesktopWithoutTaskbar.bottom -350;
    //m_nWndRight  = m_nWndLeft + m_nWndWidth;
    //m_nWndBottom = m_nWndTop + m_nWndHeight;

	SetWindowPos(&wndTop,
                 m_nWndLeft,
                 m_nWndTop, 
                 300,
                 370,
                 SWP_SHOWWINDOW
                );
}

HBRUSH CDangerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH B;
 	switch (nCtlColor)
 	{
	case CTLCOLOR_STATIC : //在此加入你想要改变背景色的控件消息
	   if(pWnd->GetDlgCtrlID()==IDC_INFO_STATIC) //static   
	   {   

 		pDC->SetBkMode(TRANSPARENT);
 		B = CreateSolidBrush(RGB(125,153,44)); //COLOR是你想设置的颜色
		return (HBRUSH) B; 
	   }
 	case CTLCOLOR_MSGBOX:
 	case CTLCOLOR_DLG : //在此加入你想要改变背景色的控件消息
 		pDC->SetBkMode(TRANSPARENT);
 		B = CreateSolidBrush(RGB(41,156,88)); //COLOR是你想设置的颜色
		return (HBRUSH) B; 
 	case CTLCOLOR_EDIT: //其他控件设置自己默认的颜色和背景刷.
 		pDC->SetBkMode(TRANSPARENT);
 		B = CreateSolidBrush(COLOR1); //COLOR是你想设置的颜色
 		return (HBRUSH) B; 
 	}
    return CDialog::OnCtlColor(pDC, pWnd, nCtlColor); 
}

void CDangerDlg::OnOK() 
{
	// TODO: Add extra validation here
	::PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+14,(WPARAM)0,(LPARAM)m_choice);
	CDialog::OnOK();
}

BOOL CDangerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	((CButton*)GetDlgItem(IDC_END_BUTTON))->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
