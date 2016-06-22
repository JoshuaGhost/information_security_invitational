#if !defined(AFX_ADVANCESETTING_H__EF06BC72_2555_4312_B1C5_CE0EAB9992FF__INCLUDED_)
#define AFX_ADVANCESETTING_H__EF06BC72_2555_4312_B1C5_CE0EAB9992FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvanceSetting.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvanceSetting dialog

class CAdvanceSetting : public CDialog
{
// Construction
public:
	CAdvanceSetting(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAdvanceSetting)
	enum { IDD = IDD_ADVANCESETTING_DIALOG };
	CListCtrl	m_iplist;
	CComboBox	m_statelist;
	CComboBox	m_scriptlist;
	CComboBox	m_protocollist;
	CListCtrl	m_portlist;
	CComboBox	m_oslist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvanceSetting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void LoadCheateSetting();

	// Generated message map functions
	//{{AFX_MSG(CAdvanceSetting)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnAddport();
	afx_msg void OnRclickPortlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDel();
	afx_msg void OnDestroy();
	afx_msg void OnAddhost();
	afx_msg void OnSavehost();
	afx_msg void OnNewhost();
	afx_msg void OnRclickIplist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelhost();
	afx_msg void OnDblclkIplist(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVANCESETTING_H__EF06BC72_2555_4312_B1C5_CE0EAB9992FF__INCLUDED_)
