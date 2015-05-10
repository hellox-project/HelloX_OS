// hcxbuild.h : main header file for the HCXBUILD application
//

#if !defined(AFX_HCXBUILD_H__4E5A55ED_BF7A_4E55_8A37_95CCD3DE200F__INCLUDED_)
#define AFX_HCXBUILD_H__4E5A55ED_BF7A_4E55_8A37_95CCD3DE200F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHcxbuildApp:
// See hcxbuild.cpp for the implementation of this class
//

class CHcxbuildApp : public CWinApp
{
public:
	CHcxbuildApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHcxbuildApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHcxbuildApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HCXBUILD_H__4E5A55ED_BF7A_4E55_8A37_95CCD3DE200F__INCLUDED_)
