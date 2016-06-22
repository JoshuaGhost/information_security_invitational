// DefenderDoc.cpp : implementation of the CDefenderDoc class
//

#include "stdafx.h"
#include "Defender.h"

#include "DefenderDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDefenderDoc

IMPLEMENT_DYNCREATE(CDefenderDoc, CDocument)

BEGIN_MESSAGE_MAP(CDefenderDoc, CDocument)
	//{{AFX_MSG_MAP(CDefenderDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefenderDoc construction/destruction

CDefenderDoc::CDefenderDoc()
{
	// TODO: add one-time construction code here

}

CDefenderDoc::~CDefenderDoc()
{
}

BOOL CDefenderDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDefenderDoc serialization

void CDefenderDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDefenderDoc diagnostics

#ifdef _DEBUG
void CDefenderDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDefenderDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDefenderDoc commands
