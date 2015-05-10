// hcxbuildDlg.h : header file
//

#if !defined(AFX_HCXBUILDDLG_H__4BCAD94C_7635_4F31_A809_0C2D94B64C59__INCLUDED_)
#define AFX_HCXBUILDDLG_H__4BCAD94C_7635_4F31_A809_0C2D94B64C59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CHcxbuildDlg dialog

//HCX header,will be put in each HCX file's begining by hcxbuild application.
struct __HCX_HEADER{
         DWORD          dwHcxSignature;   //Signature,HCX_SIGNATURE.
         DWORD          dwEntryOffset;    //Entry point's offset in HCX file.
         DWORD          dwBmpOffset;      //HCX application bitmap's offset in HCX file.
         DWORD          dwBmpWidth;       //Bitmap's width.
         DWORD          dwBmpHeight;      //Bitmap's height.
         DWORD          dwColorBits;      //How many bits per color.
         CHAR           AppName[16];      //Application's display name.
         CHAR           MajorVersion;     //Major version of this app.
         CHAR           MinorVersion;     //Minor version of this app.
         CHAR           OsMajorVersion;   //OS's major version this app require.
         CHAR           OsMinorVersion;   //OS's minor version this app require.
};
 
#define HCX_SIGNATURE 0xE9909090  //HCX file's signature,it's a sign of a valid HCX file.
 
//Round a number to 16's boundary.
#define ROUND_TO_16(x) ((x) % 16 == 0 ? (x) : ((x) + 16 - ((x) % 16)))

class CHcxbuildDlg : public CDialog
{
// Construction
public:
	BOOL ShowDllInfo(char* pDllFile);
	CHcxbuildDlg(CWnd* pParent = NULL);	// standard constructor
	~CHcxbuildDlg();
	DWORD GetHCXFileSize(char* pBinFile);
	char* InflateSections(char *pBinFile, char *pHCXFile);
	BMPIMAGE* m_pBitmap;

// Dialog Data
	//{{AFX_DATA(CHcxbuildDlg)
	enum { IDD = IDD_HCXBUILD_DIALOG };
	CEdit	m_DllInfo;
	CStatic	m_Bitmap;
	CEdit	m_Version;
	CEdit	m_Target;
	CEdit	m_MinorVer;
	CEdit	m_AppName;
	CEdit	m_BmpName;
	CEdit	m_DllName;
	CString	m_strDllName;
	CString	m_strBmpName;
	CString	m_strAppName;
	CString	m_strMinorVer;
	CString	m_strTarget;
	CString	m_strVersion;
	CString	m_strDllInfo;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHcxbuildDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CHcxbuildDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtondll();
	afx_msg void OnButtonbmp();
	afx_msg void OnBuild();
	afx_msg void OnButtonbuild();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	     char m_MinorVersion;
         char m_MajorVersion;
         BOOL CheckParam();
         BOOL BuildHCX();
         BOOL InitializeHCXHdr(__HCX_HEADER* pHeader,char* pBinFile,DWORD dwBinSize,BMPIMAGE* pBitmap);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HCXBUILDDLG_H__4BCAD94C_7635_4F31_A809_0C2D94B64C59__INCLUDED_)
