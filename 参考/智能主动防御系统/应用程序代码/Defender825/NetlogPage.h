#if !defined(AFX_NETLOGPAGE_H__1356C638_F27E_465F_8C6B_AE0909DD0D39__INCLUDED_)
#define AFX_NETLOGPAGE_H__1356C638_F27E_465F_8C6B_AE0909DD0D39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NetlogPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNetlogPage dialog

class CNetlogPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CNetlogPage)

// Construction
public:
	CNetlogPage();
	~CNetlogPage();

// Dialog Data
	//{{AFX_DATA(CNetlogPage)
	enum { IDD = IDD_PROTECT_NETLOG };
	CListCtrl	m_listAttacklog;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNetlogPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CNetlogPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnRclickAttacklogList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAttacklogRefresh();
	afx_msg void OnAttacklogClearall();
	afx_msg void OnAttacklogSave();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETLOGPAGE_H__1356C638_F27E_465F_8C6B_AE0909DD0D39__INCLUDED_)
