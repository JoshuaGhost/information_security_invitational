#if !defined(AFX_PROCESSPAGE_H__6A5E61F7_8DE0_402A_B84E_341AA15E139A__INCLUDED_)
#define AFX_PROCESSPAGE_H__6A5E61F7_8DE0_402A_B84E_341AA15E139A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessPage.h : header file
//

#include "SystemImageList.h" //系统图标类

/////////////////////////////////////////////////////////////////////////////
// CProcessPage dialog

class CProcessPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CProcessPage)

// Construction
public:
	void SetItemColor(int nItem, int nSubItem, COLORREF clrText, COLORREF clrBkgnd);
	void UpdateSubItem(int nItem, int nSubItem);
	void OnCustomdrawMyList ( NMHDR* pNMHDR, LRESULT* pResult );
	CProcessPage();
	~CProcessPage();

// Dialog Data
	//{{AFX_DATA(CProcessPage)
	enum { IDD = IDD_BASIC_PROCESS };
	CListCtrl	m_listProcess;
	CImageList*  imageList;
	CSystemImageList systemImage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CProcessPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CProcessPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnProcessRefresh();
	afx_msg void OnProcessEnd();
	afx_msg void OnRclickProcessList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProcessModule();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSPAGE_H__6A5E61F7_8DE0_402A_B84E_341AA15E139A__INCLUDED_)
