// NetlogPage.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "NetlogPage.h"

#include "basic/ado.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CListCtrl *p_loglist=NULL;
extern char szCurrentDirectory[256];

//��ȡ��־��ʾ����־�б���
void ReadLog();

/////////////////////////////////////////////////////////////////////////////
// CNetlogPage property page

IMPLEMENT_DYNCREATE(CNetlogPage, CPropertyPage)

CNetlogPage::CNetlogPage() : CPropertyPage(CNetlogPage::IDD)
{

}

CNetlogPage::~CNetlogPage()
{
}

void CNetlogPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNetlogPage)
	DDX_Control(pDX, IDC_ATTACKLOG_LIST, m_listAttacklog);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNetlogPage, CPropertyPage)
	//{{AFX_MSG_MAP(CNetlogPage)
	ON_WM_PAINT()
	ON_NOTIFY(NM_RCLICK, IDC_ATTACKLOG_LIST, OnRclickAttacklogList)
	ON_COMMAND(IDM_ATTACKLOG_REFRESH, OnAttacklogRefresh)
	ON_COMMAND(IDM_ATTACKLOG_CLEARALL, OnAttacklogClearall)
	ON_COMMAND(IDM_ATTACKLOG_SAVE, OnAttacklogSave)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetlogPage message handlers

BOOL CNetlogPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	//��ʼ�����̷��
	
	ListView_SetExtendedListViewStyle(
		m_listAttacklog.m_hWnd, 
		LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT
		); 
		
	int nColumnCount = m_listAttacklog.GetHeaderCtrl()->GetItemCount();
	for (int i=0;i < nColumnCount;i++)
		m_listAttacklog.DeleteColumn(0);
	
	m_listAttacklog.InsertColumn(0, "ʱ��",LVCFMT_LEFT,200);
	m_listAttacklog.InsertColumn(1, "�¼�",LVCFMT_LEFT,700);

    p_loglist=&m_listAttacklog;

	OnAttacklogRefresh();
	return TRUE; 
}

void CNetlogPage::OnPaint() 
{
	CPaintDC dc(this); 
	CRect rect;
	GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y;
	CPoint pointTop(x,y);
	
    m_listAttacklog.MoveWindow(CRect(pointTop,rect.BottomRight()));
}

void CNetlogPage::OnRclickAttacklogList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CRect rect;
	GetWindowRect(&rect);
	
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	int x=point.x+rect.left;
	int y=point.y+rect.top;

	CMenu *m_PopMenu=new CMenu;
	m_PopMenu->LoadMenu(IDR_NETLOG);//���ز˵�
	TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);
	*pResult = 0;
}

//ˢ���б�
void CNetlogPage::OnAttacklogRefresh() 
{
    ReadLog();
}


//�����־
void CNetlogPage::OnAttacklogClearall() 
{
	if(IDYES==AfxMessageBox("����󽫲��ɻָ���ȷ�����������־?",MB_ICONQUESTION|MB_YESNO,0))
	{
		CADODatabase* pAdoDb = new CADODatabase();
		CString strConnection = _T("");	
		SetCurrentDirectory(szCurrentDirectory);
		strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=log.mdb");
		pAdoDb->SetConnectionMode(CADODatabase::connectModeReadWrite);
		pAdoDb->SetConnectionString(strConnection);	
		if(pAdoDb->Open())
		{	
			if(pAdoDb->Execute("delete from iadslog"))
				p_loglist->DeleteAllItems();
			pAdoDb->Close();
		}
		else
		{
			::MessageBox(0,"����־ʧ��!","������������ϵͳ",MB_OK);
		}
		delete pAdoDb;
	}
}


//������־
void CNetlogPage::OnAttacklogSave() 
{
	CFileDialog savelogdlg(FALSE,"*.log","attacklog",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"�ı��ĵ�(*.txt)|*.txt||",NULL);	
	if(!savelogdlg.GetPathName().IsEmpty() && IDOK==savelogdlg.DoModal())
	{
		FILE *file=fopen(savelogdlg.GetPathName(),"w");
		int i=0;
		char tempbuf[100]={0};
		for(i=0;i<p_loglist->GetItemCount();i++)
		{
			memset(tempbuf,0,100);
			p_loglist->GetItemText(i,0,tempbuf,100);
			fwrite("ʱ��:",1,strlen("ʱ��:"),file);
			fwrite(tempbuf,1,strlen(tempbuf),file);
			fwrite(" ",1,1,file);
			memset(tempbuf,0,100);
			p_loglist->GetItemText(i,1,tempbuf,100);
			fwrite("�¼�:",1,strlen("�¼�:"),file);
			fwrite(tempbuf,1,strlen(tempbuf),file);
			fwrite("\n",1,1,file);
		}
		fclose(file);
		AfxMessageBox("������־�ɹ�!");
	}
	return;
}

//��ȡ��־��ʾ����־�б���
void ReadLog()
{
	unsigned int loglistindex=0;
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");

	p_loglist->DeleteAllItems();
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=log.mdb");
	pAdoDb->SetConnectionString(strConnection);	
	if(pAdoDb->Open())
	{	
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		if(pAdoRs->Open("iadslog", CADORecordset::openTable))
		{
			while(!pAdoRs->IsEOF())
			{
				//ʱ��
				CString csLogTime=_T("");
				CString csLogContent=_T("");
		    	pAdoRs->GetFieldValue(0,csLogTime);			
		    	p_loglist->InsertItem(loglistindex,csLogTime);
				
				//�¼�
				pAdoRs->GetFieldValue(1,csLogContent);
	        	p_loglist->SetItemText(loglistindex++,1,csLogContent);
				pAdoRs->MoveNext();
			}
			pAdoRs->Close();
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		MessageBox(0,"����־ʧ��!","������������ϵͳ",MB_OK);
	}
	delete pAdoDb;
	return;
}

void CNetlogPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
}
