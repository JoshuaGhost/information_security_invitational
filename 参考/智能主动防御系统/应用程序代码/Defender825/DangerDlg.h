#if !defined(AFX_DANGERDLG1_H__43E525C7_17DE_4C3F_BF74_759ABF5683DA__INCLUDED_)
#define AFX_DANGERDLG1_H__43E525C7_17DE_4C3F_BF74_759ABF5683DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DangerDlg1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDangerDlg dialog

class CDangerDlg : public CDialog
{
// Construction
public:
	int m_choice;
	CDangerDlg(CWnd* pParent = NULL);   // standard constructor


// Dialog Data
	//{{AFX_DATA(CDangerDlg)
	enum { IDD = IDD_DANGER };
	CStatic	m_infoStatic;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDangerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDangerDlg)
	afx_msg void OnEndButton();
	afx_msg void OnRefuseButton();
	afx_msg void OnIgnoreButton();
	afx_msg void OnAddButton();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_infoEdit;//显示提示信息
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DANGERDLG1_H__43E525C7_17DE_4C3F_BF74_759ABF5683DA__INCLUDED_)
