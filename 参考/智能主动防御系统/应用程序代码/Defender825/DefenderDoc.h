// DefenderDoc.h : interface of the CDefenderDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEFENDERDOC_H__A58E2E71_419F_45F9_97C5_45DE9C7DFB84__INCLUDED_)
#define AFX_DEFENDERDOC_H__A58E2E71_419F_45F9_97C5_45DE9C7DFB84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CDefenderDoc : public CDocument
{
protected: // create from serialization only
	CDefenderDoc();
	DECLARE_DYNCREATE(CDefenderDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefenderDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDefenderDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDefenderDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFENDERDOC_H__A58E2E71_419F_45F9_97C5_45DE9C7DFB84__INCLUDED_)
