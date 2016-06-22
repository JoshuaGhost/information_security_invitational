#if !defined(AFX_AUTORUNPAGE_H__2027ABD6_80DF_4672_922E_F603EADA86EB__INCLUDED_)
#define AFX_AUTORUNPAGE_H__2027ABD6_80DF_4672_922E_F603EADA86EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AutorunPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAutorunPage dialog

#include "SystemImageList.h" //系统图标类

class CAutorunPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CAutorunPage)

// Construction
public:
	CAutorunPage();
	~CAutorunPage();

// Dialog Data
	//{{AFX_DATA(CAutorunPage)
	enum { IDD = IDD_PROTECT_AUTORUN };
	CSystemImageList systemImage;
	CImageList* imageList;
	CListCtrl	m_listAutorun;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAutorunPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAutorunPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnRclickAutorunList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAutorunDelete();
	afx_msg void OnAutorunRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTORUNPAGE_H__2027ABD6_80DF_4672_922E_F603EADA86EB__INCLUDED_)
