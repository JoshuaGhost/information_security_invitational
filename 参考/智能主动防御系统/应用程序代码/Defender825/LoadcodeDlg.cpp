// LoadcodeDlg.cpp : implementation file

#include "stdafx.h"
#include "defender.h"

#include   "afxole.h" 
#include   <ole2.h>   
#include "LoadcodeDlg.h"
#include "basic/Ado.h"
#include "net/winpcap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//�豸�������
extern HANDLE driverHandle;	
extern LPSTR driverDosName;

//���˿���
extern UCHAR ICMP_FILTER; //�Ƿ�����Ping��
extern UCHAR PACKET_FILTER; //�Ƿ�������ݰ�����
extern UCHAR ANTIARP;       //�Ƿ�����ARP����ǽ
extern UCHAR PLAYALLROLE;   //�Ƿ�αװ�����в���������

//��������ʱ�б�
CodeNode codelist[100]={0};
CLoadcodeDlg *p_loadcode;

extern char szCurrentDirectory[256];
//��ȡ���ݿ��е�������,���ݸ���������
//ÿ�δ���100��
DWORD WINAPI ReadCode( LPVOID lpParameter);
//��ȡini�����ļ�
int  ReadIni(UCHAR *icmp_filter,UCHAR *packet_filter,UCHAR *antiarp,UCHAR *playallrole);
/////////////////////////////////////////////////////////////////////////////
// LoadCode dialog


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
ON_WM_SHOWWINDOW()
ON_WM_CLOSE()
ON_WM_PAINT()
ON_WM_DESTROY()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//����Enter��
void CLoadcodeDlg::OnOK() 
{
	return;
}
//����"X"
void CLoadcodeDlg::OnClose() 
{
	return;
}
//����ESC��
void CLoadcodeDlg::OnCancel()
{
	//return;

	CDialog::OnCancel();
}

void CLoadcodeDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	//���ر�����
	this->ModifyStyle(WS_CAPTION,0,SWP_FRAMECHANGED);
	//���ô��ڳߴ缰���
	::SetWindowPos(this->m_hWnd, HWND_TOPMOST,0, 0, 400, 260,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);   
	
	HANDLE hThread;
	DWORD  dwThreadid=0;
	CDialog::OnShowWindow(bShow, nStatus);
	p_loadcode=this;
	driverDosName=(char *)malloc(strlen(DRIVERNAME)+5);
	sprintf(driverDosName, "\\\\.\\%s", DRIVERNAME);
	
	
	//���豸
	driverHandle = CreateFile(driverDosName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
    if(driverHandle == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("��NDIS����ʧ��!");
		this->PostMessage(WM_DESTROY,0,0);
	}
	else
	{
		//��ȡ�����ļ�(���˹���)
		ReadIni(&ICMP_FILTER,&PACKET_FILTER,&ANTIARP,&PLAYALLROLE);
		
		//����������		
		//������ȡ�������߳�
		hThread=CreateThread(NULL,
			0,
			ReadCode,
			NULL,
			0,
			&dwThreadid);
		CloseHandle(hThread);
	}
}


//��ȡ���ݿ��е������룬���ݸ���������
DWORD WINAPI ReadCode(LPVOID lpParameter)
{
	UINT i=0;
	UINT finishcount=0;
	UINT totalcount=0;
	UINT percent=0;
	DWORD bytesReturned=0;
	BOOL ret=0;
	CString csPercent;
	CString csShellCode;

	SetCurrentDirectory(szCurrentDirectory);
	CADODatabase* pAdoDb = new CADODatabase();
	CString strConnection = _T("");
	strConnection = _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=code.mdb");
	pAdoDb->SetConnectionString(strConnection);
	
	if(pAdoDb->Open())
	{
		CADORecordset* pAdoRs = new CADORecordset(pAdoDb);
		if(pAdoRs->Open("shellcode", CADORecordset::openTable))
		{
			//ȡ������������
			totalcount=pAdoRs->GetRecordCount();
			while(!pAdoRs->IsEOF())
			{
				if(i==100)
				{
					//���͸���������
					ret=DeviceIoControl(driverHandle,
						IADS_DRIVER_ADDCODE,
						(LPVOID)&codelist,
						100*sizeof(CodeNode),
						NULL,
						0,
						&bytesReturned,
						NULL //ͬ������
						);
					//��ʼ��
					i=0;
					memset((unsigned char *)&codelist,0,sizeof(CodeNode)*100);
				}
				//��ȡ����(codelist[i])
				pAdoRs->GetFieldValue("shellcode",csShellCode);
				strncpy((char *)codelist[i++].code,csShellCode,32);
				Sleep(10);
		    	pAdoRs->MoveNext();
				finishcount++;

				//��ɰٷֱ�
				percent=(UINT)(100*(float)finishcount/(float)totalcount);
				csPercent.Format("%d%%",percent);
				p_loadcode->SetDlgItemText(IDC_PERCENT,csPercent);
			}
			//ʣ���i��������
			if(i)
			{
				//���͸���������
				ret=DeviceIoControl(driverHandle,
					IADS_DRIVER_ADDCODE,
					(LPVOID)&codelist,
					i*sizeof(CodeNode),
					NULL,
					0,
					&bytesReturned,
					NULL //ͬ������
					);
			}
			pAdoRs->Close();	
		}
		delete pAdoRs;
		pAdoDb->Close();
	}
	else
	{
		MessageBox(0,"��������ʧ��!","������������ϵͳ",MB_OK);
	}
	delete pAdoDb;
	p_loadcode->EndDialog(0);
	return 0;
}


//��ȡini�����ļ�
int  ReadIni(UCHAR *icmp_filter,UCHAR *packet_filter,UCHAR *antiarp,UCHAR *playallrole)
{
	char tempbuf[256]={0};//ini�ļ�·��
	char smallbuf[20]={0};
	DWORD nSize=19;
	*icmp_filter=1;
	*packet_filter=1;
	*antiarp=1;
	SetCurrentDirectory(szCurrentDirectory);
	if(!GetCurrentDirectory(250,tempbuf)) return 1;
	if(tempbuf[strlen(tempbuf)-1]!='\\')
		tempbuf[strlen(tempbuf)]='\\';
	strcat(tempbuf,"iads.ini");
	
	memset(smallbuf,0,20);
	if(GetPrivateProfileString("FilterRules",
		"ICMP_FILTER",
		"1",
		smallbuf,
		nSize,
		tempbuf))
	*icmp_filter=atoi(smallbuf);
	
	memset(smallbuf,0,20);
	if(GetPrivateProfileString("FilterRules",
		"PACKET_FILTER",
		"1",
		smallbuf,
		nSize,
		tempbuf))
	*packet_filter=atoi(smallbuf);
	
	memset(smallbuf,0,20);
	if(GetPrivateProfileString("FilterRules",
		"ANTIARP",
		"1",
		smallbuf,
		nSize,
		tempbuf))
	*antiarp=atoi(smallbuf);

	memset(smallbuf,0,20);
	if(GetPrivateProfileString("FilterRules",
		"PLAYALLROLE",
		"1",
		smallbuf,
		nSize,
		tempbuf))
	*playallrole=atoi(smallbuf);
	return 0;
}

void CLoadcodeDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting	
	CRect rect;
	GetClientRect(&rect);	
	int x=rect.TopLeft().x;
	int y=rect.TopLeft().y;
	CPoint pointTop(x,y);
	GetClientRect(&rect);//��ȡ�����С
	CDC  memDC; 
	CBitmap cBitmap;
	CBitmap* pOldMemBmp=NULL;
	cBitmap.LoadBitmap(IDB_LOADCODE);//����λͼ�������Ѿ���ӵ���Դ�ļ��У�
	memDC.CreateCompatibleDC(&dc);//�����봰��DC���ݵ�DC
	pOldMemBmp=memDC.SelectObject(&cBitmap);//���µ�λͼѡ���豸
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&memDC,0,0,400,260,SRCCOPY);//�豸֮��Ŀ���
	memDC.SelectObject(pOldMemBmp);//�ָ��豸
	CDialog::OnPaint();
}

void CLoadcodeDlg::OnDestroy() 
{
	CDialog::OnDestroy();
}


