// AdvanceSetting.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "AdvanceSetting.h"
#include <winsock2.h>

#include "basic/Ado.h"
#pragma comment(lib,"ws2_32.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//����ǰ·��
extern char szCurrentDirectory[256];
/////////////////////////////////////////////////////////////////////////////
// CAdvanceSetting dialog


CAdvanceSetting::CAdvanceSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvanceSetting::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvanceSetting)
	//}}AFX_DATA_INIT
}


void CAdvanceSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvanceSetting)
	DDX_Control(pDX, IDC_IPLIST, m_iplist);
	DDX_Control(pDX, IDC_STATELIST, m_statelist);
	DDX_Control(pDX, IDC_SCRIPTLIST, m_scriptlist);
	DDX_Control(pDX, IDC_PROTOCOLLIST, m_protocollist);
	DDX_Control(pDX, IDC_PORTLIST, m_portlist);
	DDX_Control(pDX, IDC_OSLIST, m_oslist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvanceSetting, CDialog)
	//{{AFX_MSG_MAP(CAdvanceSetting)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADDPORT, OnAddport)
	ON_NOTIFY(NM_RCLICK, IDC_PORTLIST, OnRclickPortlist)
	ON_COMMAND(IDM_DEL, OnDel)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ADDHOST, OnAddhost)
	ON_BN_CLICKED(IDC_SAVEHOST, OnSavehost)
	ON_COMMAND(IDM_NEWHOST, OnNewhost)
	ON_NOTIFY(NM_RCLICK, IDC_IPLIST, OnRclickIplist)
	ON_COMMAND(IDM_DELHOST, OnDelhost)
	ON_BN_CLICKED(IDM_NEWHOST, OnNewhost)
	ON_NOTIFY(NM_DBLCLK, IDC_IPLIST, OnDblclkIplist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvanceSetting message handlers

void CAdvanceSetting::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	//��������
	LoadCheateSetting();

	//script�б�
	m_scriptlist.AddString("default");
    //protocol�б�
	m_protocollist.AddString("TCP");
	m_protocollist.AddString("UDP");
	//portstate
	m_statelist.AddString("open");
	m_statelist.AddString("close");

	OnNewhost();

	m_portlist.InsertColumn(0,"�˿ں�",LVCFMT_CENTER,100);
	m_portlist.InsertColumn(1,"Э��",LVCFMT_CENTER,100);
	m_portlist.InsertColumn(2,"״̬",LVCFMT_CENTER,100);
	m_portlist.InsertColumn(3,"�ű�",LVCFMT_CENTER,200);

}

//��������
void CAdvanceSetting::LoadCheateSetting()
{
	CString csText;
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		//����IP�б�
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		if(pAdoRs->Open("host", CADORecordset::openTable))
		{
			while(!pAdoRs->IsEOF())
			{
				pAdoRs->GetFieldValue("host_ip",csText);
				m_iplist.InsertItem(m_iplist.GetItemCount(),csText);
				pAdoRs->MoveNext();
			}
			pAdoRs->Close();
		}

		//����OS�б�
		if(pAdoRs->Open("os", CADORecordset::openTable))
		{
			while(!pAdoRs->IsEOF())
			{
				pAdoRs->GetFieldValue("os_name",csText);
				m_oslist.AddString(csText);
				pAdoRs->MoveNext();
			}
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
	}
	delete pAdoDb;
	return;
}

//��Ӷ˿�
void CAdvanceSetting::OnAddport() 
{
	unsigned int uiPort=0;
	unsigned int uiIndex=0;
	CString csText;
	uiPort=this->GetDlgItemInt(IDC_PORTNUM);
	if(!uiPort)
	{
		AfxMessageBox("��ָ��Ҫ��ӵĶ˿�!");
		return;
	}
	else if(uiPort>65535)
	{
		AfxMessageBox("�˿ںŲ��ܴ���65535!");
		return;
	}
	else
	{
		int i=0;
		char szPortNum[10]={0};
		for(i=0;i<m_portlist.GetItemCount();i++)
		{
			m_portlist.GetItemText(i,0,szPortNum,10);
			if((unsigned int)atoi(szPortNum)==uiPort)
			{
				AfxMessageBox("�ö˿����б����Ѿ�����!");
				return;
			}
		}
	}
	uiIndex=m_portlist.GetItemCount();
	csText.Format("%d",uiPort);
	//port
	m_portlist.InsertItem(uiIndex,csText);
	//MessageBox(csText);
    //protocol
    this->GetDlgItemText(IDC_PROTOCOLLIST,csText);
	m_portlist.SetItemText(uiIndex,1,csText);
    //state
    this->GetDlgItemText(IDC_STATELIST,csText);
	m_portlist.SetItemText(uiIndex,2,csText);
	//script
	this->GetDlgItemText(IDC_SCRIPTLIST,csText);
	m_portlist.SetItemText(uiIndex,3,csText);
}

//Port�б�����Ҽ���Ӧ
void CAdvanceSetting::OnRclickPortlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CRect rect;
	GetWindowRect(&rect);	
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	int x=point.x+rect.left;
	int y=point.y+rect.top;

	if(!m_portlist.GetSelectedCount())
	{
		return;
	}
	else
	{
		CMenu *m_PopMenu=new CMenu;
	    m_PopMenu->LoadMenu(IDR_PORTLIST);//���ز˵� 
		TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);
	}	
	*pResult = 0;
}
//ɾ���˿�
void CAdvanceSetting::OnDel() 
{
	//���û��ѡ����Ŀ
	int iIndex=0;
	POSITION pos;
	if(!m_portlist.GetSelectedCount())	 return;
	pos=m_portlist.GetFirstSelectedItemPosition();
	iIndex=m_portlist.GetNextSelectedItem(pos);
	if(IDYES==::MessageBox(0,"�Ƿ�ɾ���ö˿�?","�߼�����",MB_YESNO))
	{
		CString csSql;
		char szIp[20]={0};
		char szPort[20]={0};
		m_portlist.DeleteItem(iIndex);
		//�����ݿ���ɾ���ö˿�
		CADODatabase* pAdoDb = new CADODatabase();
		CString strConnection = _T("");
		SetCurrentDirectory(szCurrentDirectory);
		strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
		this->GetDlgItemText(IDC_IPADDRESS,szIp,20);
		m_portlist.GetItemText(iIndex,0,szPort,20);
		csSql.Format("delete from port where port_ip='%s' and port_number=%s",szIp,szPort);
		pAdoDb->SetConnectionString(strConnection);
		if(pAdoDb->Open())
		{	pAdoDb->Execute(csSql);
			pAdoDb->Close();
		}
		else
		{
			::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
		}
		delete pAdoDb;		
	}	
}

void CAdvanceSetting::OnDestroy() 
{
	CDialog::OnDestroy();	
}

//�������
void CAdvanceSetting::OnAddhost() 
{
	char szIp[20]={0};
	int i=0;
	char szBuf[20]={0};
	char szPortNum[20]={0};
	char szOs[256]={0};
	CString csSql;
	int iIcmpAccept=0;
    this->GetDlgItemText(IDC_IPADDRESS,szIp,20);
	if(!strcmp(szIp,"0.0.0.0"))
	{
		AfxMessageBox("��������ȷ������IP!");
		return;
	}
	for(i=0;i<m_iplist.GetItemCount();i++)
	{
		memset(szBuf,0,20);
		m_iplist.GetItemText(i,0,szBuf,20);
		if(!strcmp(szBuf,szIp))
		{
			AfxMessageBox("�������Ѿ����б��У������ظ����!");
			return;
		}
	}
	m_iplist.InsertItem(m_iplist.GetItemCount(),szIp);
	//os
	this->GetDlgItemText(IDC_OSLIST,szOs,256);
	//os_icmp
	if(((CButton *)this->GetDlgItem(IDC_ICMPACCEPT))->GetCheck())
		iIcmpAccept=1;
	else
		iIcmpAccept=0;
	
	//д�����ݿ�
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		//�������
		if(pAdoRs->Open("host", CADORecordset::openTable))
		{
			pAdoRs->AddNew();
			//host_ip
			pAdoRs->SetFieldValue("host_ip",_variant_t(szIp));
			//host_os
			pAdoRs->SetFieldValue("host_os",_variant_t(szOs));
			//host_icmp
			pAdoRs->SetFieldValue("host_icmp",iIcmpAccept);
			pAdoRs->Update();
			pAdoRs->Close();
		}

		//��Ӷ˿�
		for(i=0;i<m_portlist.GetItemCount();i++)
		{
			//port_number
			memset(szPortNum,0,20);
			m_portlist.GetItemText(i,0,szPortNum,20);

			csSql.Format("select * from port where port_ip='%s' and port_number=%d ",
				szIp,atoi(szPortNum));
			//�����IP�ı��˿��Ѿ����ڣ�����Ӹö˿�
			if(pAdoRs->Open(csSql, CADORecordset::openQuery))
			{
				if(!pAdoRs->IsEOF())
				{
					pAdoRs->Close();
					continue;
				}
				pAdoRs->AddNew();				
				//port_ip
				pAdoRs->SetFieldValue("port_ip",_variant_t(szIp));
				//port_number
				pAdoRs->SetFieldValue("port_number",atoi(szPortNum));
				//port_protocol
				memset(szBuf,0,20);
				m_portlist.GetItemText(i,1,szBuf,20);
				if(!strcmp(szBuf,"TCP"))
					pAdoRs->SetFieldValue("port_protocol",6);
				else
					pAdoRs->SetFieldValue("port_protocol",17);
				//port_state
				memset(szBuf,0,20);
				m_portlist.GetItemText(i,2,szBuf,20);
				if(!strcmp(szBuf,"open"))
					pAdoRs->SetFieldValue("port_state",1);
				else
					pAdoRs->SetFieldValue("port_state",0);
				//port_script
				memset(szBuf,0,20);
				m_portlist.GetItemText(i,3,szBuf,20);
				pAdoRs->SetFieldValue("port_script",_variant_t(szBuf));
				pAdoRs->Update();
				pAdoRs->Close();
			}
		}		
		delete pAdoRs;
		pAdoDb->Close();
		AfxMessageBox("��������ɹ�!");
		//��λ
		OnNewhost();
	}
	else
	{
		::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
	}
	delete pAdoDb;
}

//�����޸�
void CAdvanceSetting::OnSavehost() 
{
	char szIp[20]={0};
	int i=0;
	char szBuf[20]={0};
	char szPortNum[20]={0};
	char szOs[256]={0};
	CString csSql;
	int iIcmpAccept=0;
	BOOL flag=FALSE;
    this->GetDlgItemText(IDC_IPADDRESS,szIp,20);
	if(!strcmp(szIp,"0.0.0.0"))
		return;
	for(i=0;i<m_iplist.GetItemCount();i++)
	{
		memset(szBuf,0,20);
		m_iplist.GetItemText(i,0,szBuf,20);
		if(!strcmp(szBuf,szIp))
		{
			flag=TRUE;
			break;
		}
	}

	//����б��в����ڸ�����
	if(!flag)
	{
		if(::MessageBox(this->m_hWnd,"���������б����в����ڣ��Ƿ���Ӹ��������б�?","�߼�����",MB_YESNO)==IDYES)
		{
			OnAddhost();
			return;
		}
		else
			return;
	}

	//os
	this->GetDlgItemText(IDC_OSLIST,szOs,256);
	//os_icmp
	if(((CButton *)this->GetDlgItem(IDC_ICMPACCEPT))->GetCheck())
		iIcmpAccept=1;
	else
		iIcmpAccept=0;
	
	//д�����ݿ�
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		csSql.Format("select * from host where host_ip='%s'",szIp);
		//�޸���������
		if(pAdoRs->Open(csSql, CADORecordset::openQuery))
		{
			//host_os
			pAdoRs->SetFieldValue("host_os",_variant_t(szOs));
			//host_icmp
			pAdoRs->SetFieldValue("host_icmp",iIcmpAccept);
			pAdoRs->Update();
			pAdoRs->Close();
		}

		//��Ӷ˿�
		for(i=0;i<m_portlist.GetItemCount();i++)
		{
			//port_number
			memset(szPortNum,0,20);
			m_portlist.GetItemText(i,0,szPortNum,20);

			csSql.Format("select * from port where port_ip='%s' and port_number=%d",
				szIp,atoi(szPortNum));
			//�����IP�ı��˿��Ѿ����ڣ�����Ӹö˿�
			if(pAdoRs->Open(csSql, CADORecordset::openQuery))
			{
				if(!pAdoRs->IsEOF())
				{
					pAdoRs->Close();
					continue;
				}
				pAdoRs->AddNew();				
				//port_ip
				pAdoRs->SetFieldValue("port_ip",_variant_t(szIp));
				//port_number
				pAdoRs->SetFieldValue("port_number",atoi(szPortNum));
				//port_protocol
				memset(szBuf,0,20);
				m_portlist.GetItemText(i,1,szBuf,20);
				if(!strcmp(szBuf,"TCP"))
					pAdoRs->SetFieldValue("port_protocol",6);
				else
					pAdoRs->SetFieldValue("port_protocol",17);
				//port_state
				memset(szBuf,0,20);
				m_portlist.GetItemText(i,2,szBuf,20);
				if(!strcmp(szBuf,"open"))
					pAdoRs->SetFieldValue("port_state",1);
				else
					pAdoRs->SetFieldValue("port_state",0);
				//port_state
				memset(szBuf,0,20);
				m_portlist.GetItemText(i,3,szBuf,20);
				pAdoRs->SetFieldValue("port_script",_variant_t(szBuf));
				pAdoRs->Update();
				pAdoRs->Close();
			}
		}		
		delete pAdoRs;
		pAdoDb->Close();
		AfxMessageBox("������Ϣ�ɹ�!");
	}
	else
	{
		::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
	}
	delete pAdoDb;	
}

//�½�����
void CAdvanceSetting::OnNewhost() 
{
	//IP��ַ���
	this->SetDlgItemText(IDC_IPADDRESS,"");
	//oslist��λ
	m_oslist.SetCurSel(0);
	//portlist���
	m_portlist.DeleteAllItems();
	//port_number���
	this->SetDlgItemText(IDC_PORTNUM,"");
	//statelist��λ
	m_statelist.SetCurSel(0);
	//protocollist��λ
	m_protocollist.SetCurSel(0);
	//scriptlist��λ
	m_scriptlist.SetCurSel(0);
	//icmpaccept
	((CButton *)this->GetDlgItem(IDC_ICMPACCEPT))->SetCheck(1);
}


//IP�б�����Ҽ���Ӧ
void CAdvanceSetting::OnRclickIplist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CRect rect;
	GetWindowRect(&rect);	
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	int x=point.x+rect.left;
	int y=point.y+rect.top;

	if(!m_iplist.GetSelectedCount())
	{
		return;
	}
	else
	{
		CMenu *m_PopMenu=new CMenu;
	    m_PopMenu->LoadMenu(IDR_IPLIST);//���ز˵� 
		TrackPopupMenu(m_PopMenu->GetSubMenu(0)->m_hMenu,0,x,y,0,this->GetSafeHwnd(),&rect);
	}
	*pResult = 0;
}

//ɾ��ָ������
void CAdvanceSetting::OnDelhost() 
{
	//���û��ѡ����Ŀ
	int iIndex=0;
	POSITION pos;
	char szIp[20]={0};
	if(!m_iplist.GetSelectedCount())	 return;
	pos=m_iplist.GetFirstSelectedItemPosition();
	iIndex=m_iplist.GetNextSelectedItem(pos);
	if(IDYES==::MessageBox(0,"ȷ��ɾ��������?","������������ϵͳ",MB_YESNO))
	{
        m_iplist.GetItemText(iIndex,0,szIp,20);
		m_iplist.DeleteItem(iIndex);
		CString csSql;
		CADODatabase* pAdoDb = new CADODatabase();
		CString strConnection = _T("");
		SetCurrentDirectory(szCurrentDirectory);
		strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
		pAdoDb->SetConnectionString(strConnection);
		if(pAdoDb->Open())
		{	
			//ɾ��host���� �����Ϣ
			csSql.Format("delete from host where host_ip='%s'",szIp);
		    pAdoDb->Execute(csSql);
			//ɾ��port���� �����Ϣ
			csSql.Format("delete from port where port_ip='%s'",szIp);
			pAdoDb->Execute(csSql);
			pAdoDb->Close();
		}
		else
		{
			::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
		}
		delete pAdoDb;		
	}	
}

//��ʾ���˫�����б�����ϸ��Ϣ
void CAdvanceSetting::OnDblclkIplist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	char szIp[20]={0};
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    int intSelectedColumn = pNMLV->iItem;
	m_iplist.GetItemText(intSelectedColumn,0,szIp,20);
	
	CString csSql,csText;
	int iIcmpAccept=0,iPortNum=0,iProtocol=0,iState=0;
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	SetCurrentDirectory(szCurrentDirectory);
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=CheateSetting.mdb");
	pAdoDb->SetConnectionString(strConnection);
	if(pAdoDb->Open())
	{	
		//����IP�б�
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		csSql.Format("select * from host where host_ip='%s'",szIp);
		if(pAdoRs->Open(csSql, CADORecordset::openQuery))
		{
			//����������Ϣ
			if(!pAdoRs->IsEOF())
			{
				//host_ip
				this->SetDlgItemText(IDC_IPADDRESS,szIp);
				//host_os
				pAdoRs->GetFieldValue("host_os",csText);
				this->SetDlgItemText(IDC_OSLIST,csText);
				//host_icmp
				pAdoRs->GetFieldValue("host_icmp",iIcmpAccept);
				if(iIcmpAccept)
				   ((CButton *)this->GetDlgItem(IDC_ICMPACCEPT))->SetCheck(1);
				else
			       ((CButton *)this->GetDlgItem(IDC_ICMPACCEPT))->SetCheck(0);
			}
			else
			{
				AfxMessageBox("û�в�ѯ���������������Ϣ!");
				pAdoRs->Close();
				delete pAdoRs;
				pAdoDb->Close();
				delete pAdoDb;
				return;
			}
			pAdoRs->Close();
		}
		csSql.Format("select * from port where port_ip='%s'",szIp);
		//���ض˿���Ϣ
		if(pAdoRs->Open(csSql, CADORecordset::openQuery))
		{
			while(!pAdoRs->IsEOF())
			{
				m_portlist.DeleteAllItems();
				//port_number
				pAdoRs->GetFieldValue("port_number",iPortNum);
				csText.Format("%d",iPortNum);
				m_portlist.InsertItem(m_portlist.GetItemCount(),csText);
				//port_protocol
				pAdoRs->GetFieldValue("Port_protocol",iProtocol);
				if(iProtocol==17)
					csText="UDP";
				else
				    csText="TCP";
				m_portlist.SetItemText(m_portlist.GetItemCount()-1,1,csText);
				//port_state
				pAdoRs->GetFieldValue("port_state",iState);
				if(iState)
					csText="open";
				else
					csText="close";
				m_portlist.SetItemText(m_portlist.GetItemCount()-1,2,csText);
				//port_script
				pAdoRs->GetFieldValue("port_script",csText);
				m_portlist.SetItemText(m_portlist.GetItemCount()-1,3,csText);
				pAdoRs->MoveNext();
			}
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		::MessageBox(0,"�����ݿ�ʧ��!","�߼�����",MB_OK);
	}
	delete pAdoDb;
	return;
}
