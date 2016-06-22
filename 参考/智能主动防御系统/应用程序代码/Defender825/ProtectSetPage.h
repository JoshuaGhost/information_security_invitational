#if !defined(AFX_PROTECTSETPAGE_H__7C63BAF2_E4C1_41B8_8012_2DC5B3E8A375__INCLUDED_)
#define AFX_PROTECTSETPAGE_H__7C63BAF2_E4C1_41B8_8012_2DC5B3E8A375__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProtectSetPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProtectSetPage dialog

class CProtectSetPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CProtectSetPage)

// Construction
public:
	CProtectSetPage();
	~CProtectSetPage();

// Dialog Data
	//{{AFX_DATA(CProtectSetPage)
	enum { IDD = IDD_PROTECT_SET };
	CButton	m_checkplayallrole;
	CButton	m_checkPacketfilter;
	CButton	m_checkIcmpfilter;
	CButton	m_checkAntiarp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CProtectSetPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CProtectSetPage)
	afx_msg void OnPaint();
	afx_msg void OnAntiarpCheck();
	afx_msg void OnIcmpfilterCheck();
	afx_msg void OnPacketfilterCheck();
	afx_msg void OnDestroy();
	afx_msg void OnApplySetting();
	afx_msg void OnAdvancesetting();
	afx_msg void OnPlayallrole();
	afx_msg void OnWhiteButton();
	afx_msg void OnProtectButton();
	afx_msg void OnBeginButton();
	afx_msg void OnStopButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROTECTSETPAGE_H__7C63BAF2_E4C1_41B8_8012_2DC5B3E8A375__INCLUDED_)
