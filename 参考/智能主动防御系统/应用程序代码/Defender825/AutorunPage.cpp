// AutorunPage.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "AutorunPage.h"
#include "basic/CRegOperate.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutorunPage property page

IMPLEMENT_DYNCREATE(CAutorunPage, CPropertyPage)

CAutorunPage::CAutorunPage() : CPropertyPage(CAutorunPage::IDD)
{
	//{{AFX_DATA_INIT(CAutorunPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAutorunPage::~CAutorunPage()
{
}

void CAutorunPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutorunPage)
	DDX_Control(pDX, IDC_AUTORUN_LIST, m_listAutorun);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutorunPage, CPropertyPage)
	//{{AFX_MSG_MAP(CAutorunPage)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_AUTORUN_LIST, OnRclickAutorunList)
	ON_COMMAND(IDM_AUTORUN_DELETE, OnAutorunDelete)
	ON_COMMAND(IDM_AUTORUN_REFRESH, OnAutorunRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutorunPage message handlers

BOOL CAutorunPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	//��ʼ�������б�
	ListView_SetExtendedListViewStyle(
		m_listAutorun.m_hWnd, 
		LVS_EX_GRIDLINES  | LVS_EX_FULLROWSELECT
		); 
	m_listAutorun.InsertColumn(0, "������������",LVCFMT_LEFT,150);

	m_listAutorun.InsertColumn(1, "��ִ���ļ�",LVCFMT_LEFT,250);

	m_listAutorun.InsertColumn(2, "������Ϣ",LVCFMT_LEFT,230);

	m_listAutorun.InsertColumn(3, "ע���·��",LVCFMT_LEFT,320);

	imageList=new CImageList; //systemImage.GetImageList(TRUE);
	imageList->Create(16, 16, ILC_COLOR24 | ILC_MASK, 1, 1);
	
	m_listAutorun.SetImageList(imageList,LVSIL_SMALL); //����ͼ������
	
	
	
	OnAutorunRefresh();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAutorunPage::OnPaint() 
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
	
    m_listAutorun.MoveWindow(CRect(pointTop,rect.BottomRight()));
   
}


void CAutorunPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CAutorunPage::OnRclickAutorunList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CRect rect;
	GetWindowRect(&rect);	
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	int x=point.x+rect.left;
	int y=point.y+rect.top;

	if(!m_listAutorun.GetSelectedCount())
	{
		return;
	}
	else
	{
		CMenu *m_PopMenu=new CMenu;
	    m_PopMenu->LoadMenu(IDR_AUTORUN);//���ز˵� 
		TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);
	}
	
	*pResult = 0;

}

void CAutorunPage::OnAutorunDelete() 
{
	int index ;
	POSITION pos =m_listAutorun.GetFirstSelectedItemPosition();
	
	if(pos==NULL)
		return;

	CRegOperate RegOperator;

	while(pos)
	{	
		index=m_listAutorun.GetNextSelectedItem(pos);
		if(IDNO==MessageBox("ȷ��Ҫɾ��������?","������������ϵͳ",MB_YESNO))
		  return;
		RegOperator.DeleteValue(m_listAutorun.GetItemText(index,3),m_listAutorun.GetItemText(index,0));
		//AfxMessageBox(m_listAuto.GetItemText(index,0));
	}

	OnAutorunRefresh();	
	
}

void CAutorunPage::OnAutorunRefresh() 
{
		m_listAutorun.DeleteAllItems();

    //·��
	LPCTSTR val[]={"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
		           "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run",
	               "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
                   "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx",
	               "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                   "HKCU\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows\\Run",
	               "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
	              };
	//·��---��ȫ��
	LPCTSTR  entry[]={"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon","Userinit",
                 	  "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon","Shell",
                      "HKCU\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon","Shell",
	                  "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows","Shell"};

	CRegOperate RegOperator;
	int i,iIndex = 0;
	int iconIndex;   //����ͼ�������е�λ�� 

	CString path;
	HICON icon;

	for(int enumtotal=0;enumtotal<7;enumtotal++)
	{
		RegOperator.EnumValue(val[enumtotal]);
		
		for(i=0;i<RegOperator.m_RegArray.GetSize();i++)
		{
			path=RegOperator.m_RegArray[i+1];
			if(path[0]=='\"')
				path=path.Mid(1,path.GetLength()-2);
			icon=systemImage.GetIcon(path,FALSE,TRUE);
			iconIndex=imageList->Add(icon);
		    //������
		    m_listAutorun.InsertItem(iIndex, RegOperator.m_RegArray[i++],iconIndex);//
	    	//��ִ���ļ�
		    m_listAutorun.SetItemText(iIndex,1,path);
	    	//ע���·��
	    	m_listAutorun.SetItemText(iIndex,3,val[enumtotal]);
	    	iIndex++;
		}
	}

	for(i=0;i<4;i++)
	{
		RegOperator.GetKeyValue(entry[2*i],entry[2*i+1]);
	    //������
    	m_listAutorun.InsertItem(iIndex,entry[2*i+1],-1);
	    //��ִ���ļ�
	    if(RegOperator.m_RegArray.GetSize())
			m_listAutorun.SetItemText(iIndex,1,RegOperator.m_RegArray[0]);
    	//ע���·��
		m_listAutorun.SetItemText(iIndex,3,entry[2*i]);	
		iIndex++;
	}
	
}


