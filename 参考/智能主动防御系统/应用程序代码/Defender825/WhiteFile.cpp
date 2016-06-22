// WhiteFile.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "WhiteFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWhiteFile dialog


CWhiteFile::CWhiteFile(CWnd* pParent /*=NULL*/)
	: CDialog(CWhiteFile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWhiteFile)
	//}}AFX_DATA_INIT
}


void CWhiteFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CWhiteFile)
	DDX_Control(pDX, IDC_INFO_LIST, m_listInfo);
	DDX_Control(pDX, IDC_ACT_EDIT, m_actEdit);
	DDX_Control(pDX, IDC_PROC_EDIT, m_procEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWhiteFile, CDialog)
    
	
	//{{AFX_MSG_MAP(CWhiteFile)
	ON_BN_CLICKED(IDC_ADD_BUTTON, OnAddButton)
	ON_BN_CLICKED(IDC_DELETE_BUTTON, OnDeleteButton)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhiteFile message handlers


BOOL CWhiteFile::OnInitDialog()
{
	CDialog::OnInitDialog();

	DWORD dwStyle= GetWindowLong(m_listInfo.m_hWnd, GWL_STYLE); 
	SetWindowLong( m_listInfo.m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT);


	DWORD styles =m_listInfo.GetExtendedStyle();
	m_listInfo.SetExtendedStyle(styles|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_GRIDLINES);
	m_listInfo.SetBkColor(RGB(0xFF, 0xFF, 0xE0));
	m_listInfo.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));

//	m_listInfo.InsertColumn(0,"���",LVCFMT_LEFT,50);
	m_listInfo.InsertColumn(0,"������",LVCFMT_LEFT,200);
	m_listInfo.InsertColumn(1,"����",LVCFMT_LEFT,200);
	return true;
}

void  CWhiteFile::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	int          ipos;
	int          j,m;
	CStdioFile   whiteFile;
	LPTSTR       point;
	char         buffer[1024]={0},key[1024],info[256];
  
	whiteFile.Open("c:\\whitelist.txt",CFile::modeRead);
	whiteFile.SeekToBegin();

	point=whiteFile.ReadString(buffer,1024);//
	
	while (point!=NULL)
	{
		memset(key,0,1024);
		memset(info,0,256);
		ipos =m_listInfo.GetItemCount();//�ȵõ���ǰ�е�λ��
//		str.Format("%d",ipos+1);
//		ipos=m_listInfo.InsertItem(ipos,str);
		j=0;
		m=0;
		while (buffer[j]!='\t')
		{
			key[m++]=buffer[j++];
		}
		j++;
		m=0;
		while (buffer[j]!='\n'&&buffer[j]!='\0')//����Ļ��з��Ǹ�ʲô10��
		{
			info[m++]=buffer[j++];
		}
		m_listInfo.InsertItem(ipos,key);
		m_listInfo.SetItemText(ipos,1,info);
		memset(buffer,0,1024);
		point=whiteFile.ReadString(buffer,1024);//
	}
	whiteFile.Close();
	
    
}



void CWhiteFile::OnAddButton() 
{
	CStdioFile      whiteFile;
	CString    pstr,info_str;
	int        ipos;
	m_procEdit.SetLimitText(20);
	m_actEdit.SetLimitText(64);
	m_procEdit.GetWindowText(pstr);//ȡ��һ����Ϣ
	pstr.MakeUpper();
	if (strlen(pstr)==0)
	{
		MessageBox("����������");
		return ;
	}
	m_actEdit.GetWindowText(info_str);//ȡ�ڶ�����Ϣ
//	info_str.MakeUpper();
if (strlen(info_str)==0)
	{
		MessageBox("����������");
		return ;
	}
	ipos = m_listInfo.GetItemCount();

	ipos=m_listInfo.InsertItem(ipos,pstr);
	m_listInfo.SetItemText(ipos,1,info_str);
   
	CWnd *pWnd; 
    pWnd = GetDlgItem(IDC_PROC_EDIT);
    pWnd->SetWindowText("");
	pWnd = GetDlgItem(IDC_ACT_EDIT);
    pWnd->SetWindowText("");

	whiteFile.Open("c:\\whitelist.txt",CFile::modeReadWrite);
	whiteFile.SeekToEnd();
    whiteFile.Write("\n",1);
	whiteFile.Write(pstr,strlen(pstr));
	whiteFile.Write("\t",1);
	whiteFile.Write(info_str,strlen(info_str));
	whiteFile.Close();
	
}

void CWhiteFile::OnDeleteButton() 
{
	CStdioFile   whiteFile;
	CString      temp[1024];
	POSITION     pos;
	int   index,num=0,i=0,point,j;
	pos= m_listInfo.GetFirstSelectedItemPosition();
	index=m_listInfo.GetNextSelectedItem(pos);

	m_listInfo.DeleteItem(index);
	whiteFile.Open("c:\\whitelist.txt",CFile::modeReadWrite);
	whiteFile.SeekToBegin();

	point= whiteFile.ReadString(temp[0]);
	while ((point!=NULL)&&(i<1024))
	{
		i++;
		point=whiteFile.ReadString(temp[i]);//����ÿһ��
	}
	j=0;
	whiteFile.SetLength(0);
	while (j!=i)
	{
		if (j!=index)
		{
			whiteFile.WriteString(temp[j]);
			if(j!=i-1)
				whiteFile.Write("\n",1);
		}
		j++;
	}
	whiteFile.Close();
	
}
