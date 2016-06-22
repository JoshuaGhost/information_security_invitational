#if !defined(AFX_LOGSHEET_H__A62AA47C_ED7C_4957_A7D7_D1358B640F33__INCLUDED_)
#define AFX_LOGSHEET_H__A62AA47C_ED7C_4957_A7D7_D1358B640F33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogSheet.h : header file
//

#include "NetlogPage.h"


/////////////////////////////////////////////////////////////////////////////
// CLogSheet

class CLogSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CLogSheet)

// Construction
public:
	CLogSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CLogSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLogSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CLogSheet)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CNetlogPage m_pageLog;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGSHEET_H__A62AA47C_ED7C_4957_A7D7_D1358B640F33__INCLUDED_)
