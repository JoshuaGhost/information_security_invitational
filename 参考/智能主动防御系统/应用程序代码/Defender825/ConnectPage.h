#if !defined(AFX_CONNECTPAGE_H__4430B872_ED65_4C90_A1F3_A782E419E7EF__INCLUDED_)
#define AFX_CONNECTPAGE_H__4430B872_ED65_4C90_A1F3_A782E419E7EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectPage.h : header file
//

#include "SystemImageList.h"

/////////////////////////////////////////////////////////////////////////////
// CConnectPage dialog

class CConnectPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CConnectPage)

// Construction
public:
	CConnectPage();
	~CConnectPage();

// Dialog Data
	//{{AFX_DATA(CConnectPage)
	enum { IDD = IDD_BASIC_CONNECT };
    CSystemImageList systemImage;
	CImageList* imageList;
	CListCtrl	m_listConnect;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CConnectPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CConnectPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnRclickConnectList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnConnectEnd();
	afx_msg void OnConnectRefresh();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTPAGE_H__4430B872_ED65_4C90_A1F3_A782E419E7EF__INCLUDED_)
