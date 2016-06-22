#if !defined(AFX_PROTECTSHEET_H__B221256D_F712_4F67_9470_A178C6A5A7D4__INCLUDED_)
#define AFX_PROTECTSHEET_H__B221256D_F712_4F67_9470_A178C6A5A7D4__INCLUDED_

#include "ProtectSetPage.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProtectSheet.h : header file
//


#include "LocalPage.h"
#include "ProcessPage.h"
#include "ConnectPage.h"
#include "ServicePage.h"
#include "AutorunPage.h"
#include "HostsPage.h"

/////////////////////////////////////////////////////////////////////////////
// CProtectSheet

class CProtectSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CProtectSheet)

// Construction
public:
	CProtectSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CProtectSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProtectSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	CLocalPage        m_pageLocal;
    CProcessPage      m_pageProcess;
	CConnectPage      m_pageConnect;
	CServicePage      m_pageService;
	CAutorunPage      m_pageAutorun;
	CHostsPage        m_pageHosts;
	

	virtual ~CProtectSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProtectSheet)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROTECTSHEET_H__B221256D_F712_4F67_9470_A178C6A5A7D4__INCLUDED_)
