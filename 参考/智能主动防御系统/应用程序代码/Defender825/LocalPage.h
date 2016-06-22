#if !defined(AFX_LOCALPAGE_H__8ACF43EB_F1E8_47EB_BB36_67B371170AE7__INCLUDED_)
#define AFX_LOCALPAGE_H__8ACF43EB_F1E8_47EB_BB36_67B371170AE7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LocalPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLocalPage dialog

class CLocalPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CLocalPage)

// Construction
public:
	void UpdateTitle();
	CLocalPage();
	~CLocalPage();

// Dialog Data
	//{{AFX_DATA(CLocalPage)
	enum { IDD = IDD_BASIC_LOCAL };
	CComboBox	m_listAdapter;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLocalPage)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	LRESULT OnMessagePcount(WPARAM wParam, LPARAM lParam);
	void GetStatistics();
	// Generated message map functions
	//{{AFX_MSG(CLocalPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeAdapterList();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnRclickMeslist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelete();
	afx_msg void OnDeleteall();
	afx_msg void OnNetshow();
	afx_msg void OnHostshow();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	LRESULT CLocalPage::OnMessageListMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCALPAGE_H__8ACF43EB_F1E8_47EB_BB36_67B371170AE7__INCLUDED_)
