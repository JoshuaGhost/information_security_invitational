#if !defined(AFX_MODULEDLG_H__745C632E_4D6B_43B3_8488_08A956E59D34__INCLUDED_)
#define AFX_MODULEDLG_H__745C632E_4D6B_43B3_8488_08A956E59D34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModuleDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CModuleDlg dialog

class CModuleDlg : public CDialog
{
// Construction
public:
	CModuleDlg(CWnd* pParent = NULL);   // standard constructor
    CModuleDlg::CModuleDlg(DWORD pid);
// Dialog Data
	//{{AFX_DATA(CModuleDlg)
	enum { IDD = IDD_BASIC_MODULE };
	CListCtrl	m_listModule;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModuleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModuleDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnRefreshButton();
	afx_msg void OnSoftButton();
	afx_msg void OnForceButton();
	afx_msg void OnForceallButton();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DWORD m_pid;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODULEDLG_H__745C632E_4D6B_43B3_8488_08A956E59D34__INCLUDED_)
