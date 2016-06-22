// ProtectSetPage.cpp : implementation file
//

#include "stdafx.h"
#include "Defender.h"
#include "ProtectSetPage.h"
#include "AdvanceSetting.h"
#include "WhiteFile.h"
#include "ProtectFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//设备驱动句柄
extern HANDLE driverHandle;	

//过滤开关
extern UCHAR ICMP_FILTER; //是否允许Ping入
extern UCHAR PACKET_FILTER; //是否进行数据包过滤
extern UCHAR ANTIARP;       //是否启用ARP防火墙
extern UCHAR PLAYALLROLE;   //是否伪装成所有不存活的主机

//程序当前路径
extern char szCurrentDirectory[256];

//设置过滤规则
extern int SetFilterRules(UCHAR icmp_filter,UCHAR packet_filter,UCHAR antiarp);
//写入ini配置文件
int  WriteIni(UCHAR icmp_filter,UCHAR packet_filter,UCHAR antiarp,UCHAR playallrole);

////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
// CProtectSetPage property page

IMPLEMENT_DYNCREATE(CProtectSetPage, CPropertyPage)

CProtectSetPage::CProtectSetPage() : CPropertyPage(CProtectSetPage::IDD)
{
	//{{AFX_DATA_INIT(CProtectSetPage)
	//}}AFX_DATA_INIT
}

CProtectSetPage::~CProtectSetPage()
{
}

void CProtectSetPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProtectSetPage)
	DDX_Control(pDX, IDC_PLAYALLROLE, m_checkplayallrole);
	DDX_Control(pDX, IDC_PACKETFILTER_CHECK, m_checkPacketfilter);
	DDX_Control(pDX, IDC_ICMPFILTER_CHECK, m_checkIcmpfilter);
	DDX_Control(pDX, IDC_ANTIARP_CHECK, m_checkAntiarp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProtectSetPage, CPropertyPage)
	//{{AFX_MSG_MAP(CProtectSetPage)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_ANTIARP_CHECK, OnAntiarpCheck)
	ON_BN_CLICKED(IDC_ICMPFILTER_CHECK, OnIcmpfilterCheck)
	ON_BN_CLICKED(IDC_PACKETFILTER_CHECK, OnPacketfilterCheck)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_APPLY, OnApplySetting)
	ON_BN_CLICKED(IDC_ADVANCESETTING, OnAdvancesetting)
	ON_BN_CLICKED(IDC_PLAYALLROLE, OnPlayallrole)
	ON_BN_CLICKED(IDC_WHITE_BUTTON, OnWhiteButton)
	ON_BN_CLICKED(IDC_PROTECT_BUTTON, OnProtectButton)
	ON_BN_CLICKED(IDC_BEGIN_BUTTON, OnBeginButton)
	ON_BN_CLICKED(IDC_STOP_BUTTON, OnStopButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProtectSetPage message handlers

void CProtectSetPage::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
		if(GetParent()->IsWindowVisible()==false)
	{
		this->ShowWindow(SW_HIDE);
		return ;
	}
	// Do not call CPropertyPage::OnPaint() for painting messages
}

void CProtectSetPage::OnAntiarpCheck() 
{
	// TODO: Add your control notification handler code here
	if(m_checkAntiarp.GetCheck())
	{
		m_checkPacketfilter.SetCheck(1);
	}
}

void CProtectSetPage::OnIcmpfilterCheck() 
{
	if(m_checkIcmpfilter.GetCheck())
	{
		m_checkPacketfilter.SetCheck(1);
	}
}

void CProtectSetPage::OnPacketfilterCheck() 
{
	if(!m_checkPacketfilter.GetCheck())
	{
		m_checkIcmpfilter.SetCheck(0);
		m_checkAntiarp.SetCheck(0);
	}	
}

BOOL CProtectSetPage::OnSetActive() 
{
	if(ICMP_FILTER)
		m_checkIcmpfilter.SetCheck(1);
	else
		m_checkIcmpfilter.SetCheck(0);

	if(PACKET_FILTER)
		m_checkPacketfilter.SetCheck(1);
	else
		m_checkPacketfilter.SetCheck(0);

	if(ANTIARP)
		m_checkAntiarp.SetCheck(1);
	else
		m_checkAntiarp.SetCheck(0);	

	if(PLAYALLROLE)
		m_checkplayallrole.SetCheck(1);
	else
		m_checkplayallrole.SetCheck(0);

	return CPropertyPage::OnSetActive();
}


//应用设置
void CProtectSetPage::OnApplySetting() 
{
	if(m_checkIcmpfilter.GetCheck())
		ICMP_FILTER=1;
	else
		ICMP_FILTER=0;

	if(m_checkPacketfilter.GetCheck())
		PACKET_FILTER=1;
	else
		PACKET_FILTER=0;

	if(m_checkAntiarp.GetCheck())
		ANTIARP=1;
	else
		ANTIARP=0;

	if(m_checkplayallrole.GetCheck())
		PLAYALLROLE=1;
	else
		PLAYALLROLE=0;
	
	WriteIni(ICMP_FILTER,PACKET_FILTER,ANTIARP,PLAYALLROLE);
	SetFilterRules(ICMP_FILTER,PACKET_FILTER,ANTIARP);	
	AfxMessageBox("设置成功!");
	return;
}

//高级设置
void CProtectSetPage::OnAdvancesetting() 
{
	CAdvanceSetting advsettingdlg;
	advsettingdlg.DoModal();
	
}
void CProtectSetPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
}

//写入ini配置文件
int  WriteIni(UCHAR icmp_filter,UCHAR packet_filter,UCHAR antiarp,UCHAR playallrole)
{
	CString csText=_T("");
	char tempbuf[256]={0};

	SetCurrentDirectory(szCurrentDirectory);
	if(!GetCurrentDirectory(250,tempbuf)) return 1;
	if(tempbuf[strlen(tempbuf)-1]!='\\')
		tempbuf[strlen(tempbuf)]='\\';
	strcat(tempbuf,"iads.ini");

	csText.Format("%d",icmp_filter);
	WritePrivateProfileString("FilterRules",
		"ICMP_FILTER",
		csText,
		tempbuf);

    csText.Format("%d",packet_filter);
	WritePrivateProfileString("FilterRules",
		"PACKET_FILTER",
		csText,
		tempbuf);

	csText.Format("%d",antiarp);
	WritePrivateProfileString("FilterRules",
		"ANTIARP",
		csText,
		tempbuf);
	
	csText.Format("%d",playallrole);
	WritePrivateProfileString("FilterRules",
		"PLAYALLROLE",
		csText,
		tempbuf);
	return 0;
}

//警告
void CProtectSetPage::OnPlayallrole() 
{
	if(m_checkplayallrole.GetCheck())
	{
		if(IDNO==::MessageBox(this->m_hWnd,"选择该项，系统将伪装成所有不存活的主机。是否继续?","智能主动防御系统",MB_YESNO))
		{
			m_checkplayallrole.SetCheck(0);
		}
	}
}

void CProtectSetPage::OnWhiteButton() 
{
	CWhiteFile   WhiteFileDlg;
	WhiteFileDlg.DoModal();
	
	
}

void CProtectSetPage::OnProtectButton() 
{
	CProtectFile   ProtectFileDlg;
	ProtectFileDlg.DoModal();
}

void CProtectSetPage::OnBeginButton() 
{
	//((CButton*)GetDlgItem(IDC_END_BUTTON))->SetCheck(1);
	if(((CButton*)GetDlgItem(IDC_BEGIN_BUTTON))->GetCheck())
	{
		::PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+16,(WPARAM)0,(LPARAM)0);
		//m_checkIcmpfilter.SetCheck(0);
	}
	else
	{
		::PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+17,(WPARAM)0,(LPARAM)0);	
	}
	
}

void CProtectSetPage::OnStopButton() 
{
    ::PostMessage(AfxGetMainWnd()->m_hWnd,WM_USER+17,(WPARAM)0,(LPARAM)0);	
}
