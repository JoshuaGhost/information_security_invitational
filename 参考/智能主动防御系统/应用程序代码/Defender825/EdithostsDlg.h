#if !defined(AFX_EDITHOSTSDLG_H__88908663_786A_415E_A29F_9BB5719BCC13__INCLUDED_)
#define AFX_EDITHOSTSDLG_H__88908663_786A_415E_A29F_9BB5719BCC13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EdithostsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEdithostsDlg dialog

class CEdithostsDlg : public CDialog
{
// Construction
public:
	CEdithostsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEdithostsDlg)
	enum { IDD = IDD_EDITHOSTS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEdithostsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEdithostsDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnCheck();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITHOSTSDLG_H__88908663_786A_415E_A29F_9BB5719BCC13__INCLUDED_)
