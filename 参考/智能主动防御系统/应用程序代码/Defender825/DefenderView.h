// DefenderView.h : interface of the CDefenderView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEFENDERVIEW_H__F6B0BB90_1D08_4BE2_B368_53A32D7917FB__INCLUDED_)
#define AFX_DEFENDERVIEW_H__F6B0BB90_1D08_4BE2_B368_53A32D7917FB__INCLUDED_

#include "BasicSheet.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "ProtectSheet.h"
#include "LogSheet.h"
#include "SetSheet.h"

class CDefenderView : public CView
{
protected: // create from serialization only
	CDefenderView();
	DECLARE_DYNCREATE(CDefenderView)

// Attributes
public:
	CDefenderDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefenderView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
//	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
//	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
//	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDefenderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDefenderView)
	afx_msg void OnPaint();
	afx_msg void OnProtect();
	afx_msg void OnHelp();
	afx_msg void OnLog();
	afx_msg void OnSet();
	afx_msg void OnUpdateLog(CCmdUI* pCmdUI);
	afx_msg void OnUpdateProtect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSet(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CProtectSheet *protectSheet;
	CSetSheet     *setSheet;
	CLogSheet     *logSheet;
	//CNetlogPage   *logPage;
	

	int m_intCurrent;
	int m_intBefore;
	//CProtectSheet basicSheet("·À»¤");
};

#ifndef _DEBUG  // debug version in DefenderView.cpp
inline CDefenderDoc* CDefenderView::GetDocument()
   { return (CDefenderDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFENDERVIEW_H__F6B0BB90_1D08_4BE2_B368_53A32D7917FB__INCLUDED_)
