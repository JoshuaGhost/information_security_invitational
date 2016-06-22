#if !defined(AFX_WHITEFILE_H__7390127F_82C1_41DF_9628_D01F58FC4ABE__INCLUDED_)
#define AFX_WHITEFILE_H__7390127F_82C1_41DF_9628_D01F58FC4ABE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WhiteFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWhiteFile dialog

class CWhiteFile : public CDialog
{
// Construction
public:
	CWhiteFile(CWnd* pParent = NULL);   // standard constructor
	BOOL OnInitDialog();

// Dialog Data
	//{{AFX_DATA(CWhiteFile)
	enum { IDD = IDD_WHITE_DIALOG };
	CListCtrl	m_listInfo;
	CEdit	m_actEdit;
	CEdit	m_procEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhiteFile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWhiteFile)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnAddButton();
	afx_msg void OnDeleteButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHITEFILE_H__7390127F_82C1_41DF_9628_D01F58FC4ABE__INCLUDED_)
