// HostsPage.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "HostsPage.h"
#include "EdithostsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CString csText;
/////////////////////////////////////////////////////////////////////////////
// CHostsPage property page


IMPLEMENT_DYNCREATE(CHostsPage, CPropertyPage)

CHostsPage::CHostsPage() : CPropertyPage(CHostsPage::IDD)
{
	//{{AFX_DATA_INIT(CHostsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CHostsPage::~CHostsPage()
{
}

void CHostsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHostsPage)
	DDX_Control(pDX, IDC_HOSTS_LIST, m_listHosts);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHostsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CHostsPage)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_COMMAND(IDM_HOSTS_REFRESH, OnHostsRefresh)
	ON_COMMAND(IDM_HOSTS_NEW, OnHostsNew)
	ON_COMMAND(IDM_HOSTS_DELETE, OnHostsDelete)
	ON_COMMAND(IDM_HOSTS_CHANGE, OnHostsChange)
	ON_COMMAND(IDM_HOSTS_SAVE, OnHostsSave)
	ON_NOTIFY(NM_RCLICK, IDC_HOSTS_LIST, OnRclickHostsList)
	ON_NOTIFY(NM_DBLCLK, IDC_HOSTS_LIST, OnDblclkHostsList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHostsPage message handlers

BOOL CHostsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	OnHostsRefresh();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHostsPage::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if(GetParent()->IsWindowVisible()==false)
	{
		this->ShowWindow(SW_HIDE);
		return ;
	}	

	CRect rect;
	GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y+10;
	CPoint pointTop(x,y);	
    m_listHosts.MoveWindow(CRect(pointTop,rect.BottomRight()));
}

void CHostsPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CHostsPage::OnHostsRefresh() 
{
	m_listHosts.DeleteAllItems();
	
	char hostspath[]={"C:\\WINDOWS\\system32\\drivers\\etc\\hosts"};
	CFile cfile;
	char linetext[256]={0};
	char twobyte[2];
	int iIndex=0;
	if(!cfile.Open(hostspath,CFile::modeReadWrite)) 
	{
		AfxMessageBox("无法打开HOSTS文件!");
		return;
	}
	while(cfile.GetPosition()<cfile.GetLength())
	{
		memset(twobyte,0,2);
		cfile.Read(twobyte,1);

		if(strlen(linetext)==256)
		{
			m_listHosts.InsertItem(iIndex++,linetext);
			memset(linetext,0,256);
		}
		else if(twobyte[0]==0x0d)
		{
			m_listHosts.InsertItem(iIndex++,linetext);
			cfile.Read(twobyte,1); //跳过0x0A
			memset(linetext,0,256);
		}
		else
		{
			linetext[strlen(linetext)]=twobyte[0];
		}
	}
	cfile.Close();
	
}

void CHostsPage::OnHostsNew() 
{
	CEdithostsDlg edithostsdlg;
	if(edithostsdlg.DoModal()==1)
	{
		m_listHosts.InsertItem(m_listHosts.GetItemCount(),csText);
		//保存修改
		OnHostsSave();
	}
	csText="";	
}



void CHostsPage::OnHostsDelete() 
{
	//如果没有选中项目
	int iIndex=0;
	POSITION pos;
	if(!m_listHosts.GetSelectedCount())	 return;
	pos=m_listHosts.GetFirstSelectedItemPosition();
	iIndex=m_listHosts.GetNextSelectedItem(pos);
	if(IDYES==::MessageBox(0,"确定删除该行?","HOSTS文件管理",MB_YESNO))
	{
		m_listHosts.DeleteItem(iIndex);
		//保存修改
		OnHostsSave();
	}	
}

void CHostsPage::OnHostsChange() 
{
    int iIndex=0;
	POSITION pos;
	if(!m_listHosts.GetSelectedCount())	 return;
	pos=m_listHosts.GetFirstSelectedItemPosition();
	iIndex=m_listHosts.GetNextSelectedItem(pos);
  
	csText=m_listHosts.GetItemText(iIndex,0);
	CEdithostsDlg edithostsdlg;
	if(edithostsdlg.DoModal()==1)
	{
		m_listHosts.SetItemText(iIndex,0,csText);
		//保存修改
		OnHostsSave();
	}	
}

void CHostsPage::OnHostsSave() 
{
   char hostspath[]={"C:\\WINDOWS\\system32\\drivers\\etc\\hosts"};
	CFile cfile;
	CString cslinetext;
	char crlf[]={0x0d,0x0a};
	int i=0;
	if(!cfile.Open(hostspath,CFile::modeWrite)) 
	{
		return;
	}
	cfile.SetLength(0);
	for(i=0;i<m_listHosts.GetItemCount();i++)
	{
		cslinetext=m_listHosts.GetItemText(i,0);
		cfile.Write(cslinetext,cslinetext.GetLength());
		//写入换行符号
		cfile.Write(crlf,2);
	}
	//AfxMessageBox("保存HOSTS文件成功!");
	cfile.Close();
	
}

void CHostsPage::OnRclickHostsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	CRect rect;
	GetWindowRect(&rect);	
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	int x=point.x+rect.left;
	int y=point.y+rect.top;

	if(!m_listHosts.GetSelectedCount())
	{
		return;
	}
	else
	{
		CMenu *m_PopMenu=new CMenu;
	    m_PopMenu->LoadMenu(IDR_HOSTS);//加载菜单 
		TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);
	}

	*pResult = 0;
}

void CHostsPage::OnDblclkHostsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	pResult = 0;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    int intSelectedColumn = pNMLV->iItem;
	csText=m_listHosts.GetItemText(intSelectedColumn,0);
	CEdithostsDlg edithostsdlg;
	if(edithostsdlg.DoModal()==1)
	{
		m_listHosts.SetItemText(intSelectedColumn,0,csText);
	}
}
