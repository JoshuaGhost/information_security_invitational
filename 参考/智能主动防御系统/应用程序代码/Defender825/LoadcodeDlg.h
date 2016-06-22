#if !defined(AFX_LOADCODE_H__84590942_E3DA_4DEA_8BFA_952A96CB04DC__INCLUDED_)
#define AFX_LOADCODE_H__84590942_E3DA_4DEA_8BFA_952A96CB04DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadcodeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// LoadCode dialog

class CLoadcodeDlg : public CDialog
{
// Construction
public:
	CLoadcodeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoadcodeDlg)
	enum { IDD = IDD_LOADCODE};
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadcodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnCancel();

	// Generated message map functions
	//{{AFX_MSG(CLoadcodeDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADCODEDLG_H__84590942_E3DA_4DEA_8BFA_952A96CB04DC__INCLUDED_)
