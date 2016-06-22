#if !defined(AFX_BASICSHEET_H__88548978_DA73_45EE_B147_70ABDBC8E1CD__INCLUDED_)
#define AFX_BASICSHEET_H__88548978_DA73_45EE_B147_70ABDBC8E1CD__INCLUDED_

#include "ProcessPage.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BasicSheet.h : header file
//

#include "LocalPage.h"


/////////////////////////////////////////////////////////////////////////////
// CBasicSheet

class CBasicSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CBasicSheet)

// Construction
public:
	CBasicSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CBasicSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBasicSheet)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	CLocalPage m_pageLocal;
	virtual ~CBasicSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBasicSheet)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASICSHEET_H__88548978_DA73_45EE_B147_70ABDBC8E1CD__INCLUDED_)
