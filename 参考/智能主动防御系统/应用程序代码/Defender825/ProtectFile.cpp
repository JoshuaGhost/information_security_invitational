// ProtectFile.cpp : implementation file
//

#include "stdafx.h"
#include "defender.h"
#include "ProtectFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProtectFile dialog


CProtectFile::CProtectFile(CWnd* pParent /*=NULL*/)
	: CDialog(CProtectFile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProtectFile)
	//}}AFX_DATA_INIT
}


void CProtectFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProtectFile)
	DDX_Control(pDX, IDC_PROTECT_EDIT2, m_infoEdit);
	DDX_Control(pDX, IDC_PROTECT_EDIT, m_protectEdit);
	DDX_Control(pDX, IDC_PROTECT_LIST, m_listInfo);
	//}}AFX_DATA_MAP
}


BOOL CProtectFile::OnInitDialog()
{
	CDialog::OnInitDialog();

	DWORD dwStyle= GetWindowLong(m_listInfo.m_hWnd, GWL_STYLE); 
	SetWindowLong( m_listInfo.m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT);


	DWORD styles =m_listInfo.GetExtendedStyle();
	m_listInfo.SetExtendedStyle(styles|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_GRIDLINES);
	m_listInfo.SetBkColor(RGB(0xFF, 0xFF, 0xE0));
	m_listInfo.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));

//	m_listInfo.InsertColumn(0,"序号",LVCFMT_LEFT,50);
	m_listInfo.InsertColumn(0,"注册表项",LVCFMT_LEFT,500);
	m_listInfo.InsertColumn(1,"信息",LVCFMT_LEFT,200);
	return true;
}

//   TODO:   Return   a   different   brush   if   the   default   is   not   desired 



BEGIN_MESSAGE_MAP(CProtectFile, CDialog)
	//{{AFX_MSG_MAP(CProtectFile)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADD_BUTTON, OnAddButton)
	ON_BN_CLICKED(IDC_DELETE_BUTTON, OnDeleteButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProtectFile message handlers

void  CProtectFile::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	int          ipos;
	int          j,m;
	CStdioFile   protectFile;
	LPTSTR       point;
	char         buffer[1024]={0},key[1024],info[256];
  
	protectFile.Open("c:\\protect.txt",CFile::modeRead);
	protectFile.SeekToBegin();

	point=protectFile.ReadString(buffer,1024);//
	
	while (point!=NULL)
	{
		memset(key,0,1024);
		memset(info,0,256);
		ipos =m_listInfo.GetItemCount();//先得到当前行的位置
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
		while (buffer[j]!='\n'&&buffer[j]!='\0')//读入的换行符是个什么10？
		{
			info[m++]=buffer[j++];
		}
		m_listInfo.InsertItem(ipos,key);
		m_listInfo.SetItemText(ipos,1,info);
		memset(buffer,0,1024);
		point=protectFile.ReadString(buffer,1024);//
	}
	protectFile.Close();
	
    
}

void CProtectFile::OnAddButton() 
{
	CStdioFile      protectFile;
	CString    pstr,info_str;
	int        ipos;
	m_protectEdit.SetLimitText(512);
	m_infoEdit.SetLimitText(128);

	m_protectEdit.GetWindowText(pstr);//取第一个信息
	pstr.MakeUpper();
	CWnd *pWnd;
	if (strlen(pstr)==0)
	{
		MessageBox("请输入数据");
		return ;
	}
	m_infoEdit.GetWindowText(info_str);//取第二个信息
	if (strlen(info_str)==0)
	{
		MessageBox("请输入数据");
		return ;
	}
//	info_str.MakeUpper();
	ipos = m_listInfo.GetItemCount();
//	tstr.Format("%d",ipos+1);
//	ipos = m_listInfo.InsertItem(ipos,tstr);
	ipos=m_listInfo.InsertItem(ipos,pstr);
	m_listInfo.SetItemText(ipos,1,info_str);
    pWnd = GetDlgItem(IDC_PROTECT_EDIT);
    pWnd->SetWindowText("");
    pWnd = GetDlgItem(IDC_PROTECT_EDIT2);
    pWnd->SetWindowText("");

	protectFile.Open("c:\\protect.txt",CFile::modeReadWrite);
	protectFile.SeekToEnd();
    protectFile.Write("\n",1);
	protectFile.Write(pstr,strlen(pstr));
	protectFile.Write("\t",1);
	protectFile.Write(info_str,strlen(info_str));
	protectFile.Close();
	
}

void CProtectFile::OnDeleteButton() 
{
    CStdioFile   protectFile;
	CString      temp[1024];
	POSITION     pos;
	int   index,num=0,i=0,point,j;
	pos= m_listInfo.GetFirstSelectedItemPosition();
	index=m_listInfo.GetNextSelectedItem(pos);

	m_listInfo.DeleteItem(index);
	protectFile.Open("c:\\protect.txt",CFile::modeReadWrite);
	protectFile.SeekToBegin();

	point= protectFile.ReadString(temp[0]);
	while ((point!=NULL)&&(i<1024))
	{
		i++;
		point=protectFile.ReadString(temp[i]);//读入每一行
	}
	j=0;
	protectFile.SetLength(0);
	while (j!=i)
	{
		if (j!=index)
		{
			protectFile.WriteString(temp[j]);
			if(j!=i-1)
				protectFile.Write("\n",1);
		}
		j++;
	}
	protectFile.Close();
	
}
