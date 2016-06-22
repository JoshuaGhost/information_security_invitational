// EdithostsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "EdithostsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString csText;

/////////////////////////////////////////////////////////////////////////////
// CEdithostsDlg dialog


CEdithostsDlg::CEdithostsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEdithostsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEdithostsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEdithostsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEdithostsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEdithostsDlg, CDialog)
	//{{AFX_MSG_MAP(CEdithostsDlg)
	ON_BN_CLICKED(IDC_CHECK, OnCheck)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEdithostsDlg message handlers

void CEdithostsDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CEdithostsDlg::OnOK() 
{
	if(((CButton*)(this->GetDlgItem(IDC_CHECK)))->GetCheck())
	{
		this->GetDlgItemText(IDC_DOMAIN,csText);
		csText="#"+csText;
	}
	else
	{
		CString csTemp;
		this->GetDlgItemText(IDC_IPADDRESS,csText);
		this->GetDlgItemText(IDC_DOMAIN,csTemp);
		csText=csText+"       "+csTemp;
	}
	this->EndDialog(1);	
	
	CDialog::OnOK();
}

void CEdithostsDlg::OnCheck() 
{
	if(((CButton*)(this->GetDlgItem(IDC_CHECK)))->GetCheck())
	{
		((CWnd *)(this->GetDlgItem(IDC_IPADDRESS)))->EnableWindow(FALSE);
	}
	else
	{
		((CWnd *)(this->GetDlgItem(IDC_IPADDRESS)))->EnableWindow(TRUE);

	}
	
}

void CEdithostsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	char buf[256]={0},* pbuf=buf;
	unsigned int i=0;
	strcpy(buf,(LPCSTR)csText);
	while(i<=(strlen(buf)-1))
	{
		//遇到非空格字符就跳出
		if(buf[i]!=0x0d) break;
		i++;
	}
	//去掉前导空格
	pbuf=pbuf+i;
	//如果是注释行
	if(pbuf[0]=='#')
	{
		((CButton*)(this->GetDlgItem(IDC_CHECK)))->SetCheck(1);
		((CWnd *)(this->GetDlgItem(IDC_IPADDRESS)))->EnableWindow(FALSE);
		this->SetDlgItemText(IDC_DOMAIN,pbuf+1);
	}
	else
	{
		char chtemp[256]={0};
		unsigned beforeblanklen=0;
		//找第一个空格
		if(strstr(pbuf," "))
		{
			beforeblanklen=strstr(pbuf," ")-pbuf;
			strncpy(chtemp,pbuf,beforeblanklen);
			//IP地址
			this->SetDlgItemText(IDC_IPADDRESS,chtemp);
			//移动指针到空格处
			pbuf=pbuf+beforeblanklen;
			i=0;
			while(i<=(strlen(pbuf)-1))
			{
				//遇到非空格字符就跳出
				if(pbuf[i]!=0x0d) break;
				i++;
			}
			//移动指针到非空格处
			pbuf=pbuf+i;
			this->SetDlgItemText(IDC_DOMAIN,pbuf);
		}
	}
	
}
