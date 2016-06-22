// Defender.h : main header file for the DEFENDER application
//

#if !defined(AFX_DEFENDER_H__D619A1E2_15B8_4151_B62A_AB515F051DB3__INCLUDED_)
#define AFX_DEFENDER_H__D619A1E2_15B8_4151_B62A_AB515F051DB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#define COLOR1 RGB(255,255,255)

/////////////////////////////////////////////////////////////////////////////
// CDefenderApp:
// See Defender.cpp for the implementation of this class
//

class CDefenderApp : public CWinApp
{
public:
	CDefenderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefenderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CDefenderApp)
//	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFENDER_H__D619A1E2_15B8_4151_B62A_AB515F051DB3__INCLUDED_)
