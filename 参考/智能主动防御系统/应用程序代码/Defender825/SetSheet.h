#if !defined(AFX_SETSHEET_H__D9B4DDDB_A478_4AFC_9D9E_225B8AD86F0A__INCLUDED_)
#define AFX_SETSHEET_H__D9B4DDDB_A478_4AFC_9D9E_225B8AD86F0A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetSheet.h : header file
//


#include "ProtectSetPage.h"

/////////////////////////////////////////////////////////////////////////////
// CSetSheet

class CSetSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CSetSheet)

// Construction
public:
	CSetSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CSetSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	CProtectSetPage m_pageSet;
	virtual ~CSetSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSetSheet)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETSHEET_H__D9B4DDDB_A478_4AFC_9D9E_225B8AD86F0A__INCLUDED_)
