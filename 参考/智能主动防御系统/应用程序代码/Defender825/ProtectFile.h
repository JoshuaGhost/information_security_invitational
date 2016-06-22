#if !defined(AFX_PROTECTFILE_H__9ADAF120_91D5_445A_9557_F185F3B7E30D__INCLUDED_)
#define AFX_PROTECTFILE_H__9ADAF120_91D5_445A_9557_F185F3B7E30D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProtectFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProtectFile dialog

class CProtectFile : public CDialog
{
// Construction
public:
	CProtectFile(CWnd* pParent = NULL);   // standard constructor
	BOOL OnInitDialog();

// Dialog Data
	//{{AFX_DATA(CProtectFile)
	enum { IDD = IDD_PROTECT_DIALOG };
	CEdit	m_infoEdit;
	CEdit	m_protectEdit;
	CListCtrl	m_listInfo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProtectFile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProtectFile)
	afx_msg void OnAddProtectEdit();
	afx_msg void OnDeleteProtectEdit2();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnAddButton();
	afx_msg void OnDeleteButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROTECTFILE_H__9ADAF120_91D5_445A_9557_F185F3B7E30D__INCLUDED_)
