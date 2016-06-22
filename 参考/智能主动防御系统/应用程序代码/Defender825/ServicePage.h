#if !defined(AFX_SERVICEPAGE_H__34BF9E81_4AE9_4628_B56A_2AB7A934027B__INCLUDED_)
#define AFX_SERVICEPAGE_H__34BF9E81_4AE9_4628_B56A_2AB7A934027B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServicePage.h : header file
//

#include "SystemImageList.h" //系统图标类

/////////////////////////////////////////////////////////////////////////////
// CServicePage dialog

class CServicePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CServicePage)

// Construction
public:
	CServicePage();
	~CServicePage();

// Dialog Data
	//{{AFX_DATA(CServicePage)
	enum { IDD = IDD_PROTECT_SERVICE };
	CSystemImageList systemImage;
	CImageList* imageList;
	CListCtrl	m_listService;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CServicePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CServicePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnServiceRefresh();
	afx_msg void OnServiceStart();
	afx_msg void OnServiceStop();
	afx_msg void OnPaint();
	afx_msg void OnRclickServiceList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnServiceAuto();
	afx_msg void OnServiceDisable();
	afx_msg void OnServiceManual();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVICEPAGE_H__34BF9E81_4AE9_4628_B56A_2AB7A934027B__INCLUDED_)
