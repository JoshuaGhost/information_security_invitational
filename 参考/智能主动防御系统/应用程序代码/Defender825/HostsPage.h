#if !defined(AFX_HOSTSPAGE_H__1A9D8482_44C2_4F8B_9BE0_F2C1975A3746__INCLUDED_)
#define AFX_HOSTSPAGE_H__1A9D8482_44C2_4F8B_9BE0_F2C1975A3746__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HostsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHostsPage dialog

class CHostsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CHostsPage)

// Construction
public:
	CHostsPage();
	~CHostsPage();

// Dialog Data
	//{{AFX_DATA(CHostsPage)
	enum { IDD = IDD_PROTECT_HOSTS };
	CListCtrl	m_listHosts;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CHostsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CHostsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnHostsRefresh();
	afx_msg void OnHostsNew();
	afx_msg void OnHostsDelete();
	afx_msg void OnHostsChange();
	afx_msg void OnHostsSave();
	afx_msg void OnRclickHostsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkHostsList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOSTSPAGE_H__1A9D8482_44C2_4F8B_9BE0_F2C1975A3746__INCLUDED_)
