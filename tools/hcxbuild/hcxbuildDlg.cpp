// hcxbuildDlg.cpp : implementation file
//

#include "stdafx.h"
#include "bitmap.h"
#include "hcxbuild.h"
#include "hcxbuildDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHcxbuildDlg dialog

CHcxbuildDlg::CHcxbuildDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHcxbuildDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHcxbuildDlg)
	m_strDllName = _T("");
	m_strBmpName = _T("");
	m_strAppName = _T("");
	m_strMinorVer = _T("");
	m_strTarget = _T("");
	m_strVersion = _T("");
	m_strDllInfo = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pBitmap = NULL;
}

void CHcxbuildDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHcxbuildDlg)
	DDX_Control(pDX, IDC_DLLINFO, m_DllInfo);
	DDX_Control(pDX, IDC_BMPCONTENT, m_Bitmap);
	DDX_Control(pDX, IDC_EDITVERSION, m_Version);
	DDX_Control(pDX, IDC_EDITTARGET, m_Target);
	DDX_Control(pDX, IDC_EDITMINORVER, m_MinorVer);
	DDX_Control(pDX, IDC_EDITAPPNAME, m_AppName);
	DDX_Control(pDX, IDC_EDITBMP, m_BmpName);
	DDX_Control(pDX, IDC_EDITDLL, m_DllName);
	DDX_Text(pDX, IDC_EDITDLL, m_strDllName);
	DDX_Text(pDX, IDC_EDITBMP, m_strBmpName);
	DDX_Text(pDX, IDC_EDITAPPNAME, m_strAppName);
	DDX_Text(pDX, IDC_EDITMINORVER, m_strMinorVer);
	DDX_Text(pDX, IDC_EDITTARGET, m_strTarget);
	DDX_Text(pDX, IDC_EDITVERSION, m_strVersion);
	DDX_Text(pDX, IDC_DLLINFO, m_strDllInfo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHcxbuildDlg, CDialog)
	//{{AFX_MSG_MAP(CHcxbuildDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTONDLL, OnButtondll)
	ON_BN_CLICKED(IDC_BUTTONBMP, OnButtonbmp)
	//ON_BN_CLICKED(ID_BUILD, OnBuild)
	ON_BN_CLICKED(IDC_BUTTONBUILD, OnButtonbuild)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHcxbuildDlg message handlers

BOOL CHcxbuildDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHcxbuildDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHcxbuildDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHcxbuildDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

CHcxbuildDlg::~CHcxbuildDlg()
{
         //Release all resources.
         ImageDealloc(this->m_pBitmap);
}
 
BOOL CHcxbuildDlg::CheckParam()
{
         const char*  pHCXName = NULL;
         int    surfixLen = 4;
 
         //Obtain the application name and version.
         UpdateData(TRUE);
 
         //Validate all variables.
         if(m_strDllName == "")
         {
                   MessageBox("Please specify the DLL file.","Error",MB_OK);
                   return FALSE;
         }
         if(m_strBmpName == "")
         {
                   MessageBox("Please specify the bitmap file.","Error",MB_OK);
                   return FALSE;
         }
         if(m_strAppName == "")
         {
                   MessageBox("Please specify a application name.","Error",MB_OK);
                   return FALSE;
         }
         if(m_strVersion == "")
         {
                   MessageBox("Please specify your application major version.","Error",MB_OK);
                   return FALSE;
         }
         if(m_strMinorVer == "")
         {
                   MessageBox("Please specify your application minor version.","Error",MB_OK);
                   return FALSE;
         }
         if(m_strAppName.GetLength() >= 32)  //Too long.
         {
                   MessageBox("Application name's length can not exceed 32 bytes,please specify again.","Error",MB_OK);
                   return FALSE;
         }
         if(m_strTarget == "")
         {
                   MessageBox("Please specify the output file name.","Error",MB_OK);
                   return FALSE;
         }
         pHCXName = (LPCTSTR)m_strTarget;
         surfixLen = m_strTarget.GetLength();
         if(surfixLen < 5)  //Target file name's length at least 5 bytes.
         {
                   MessageBox("Please specify a valid output file name.","Error",MB_OK);
                   return FALSE;
         }
         if((0 != strcmp((pHCXName + surfixLen - 4),".HCX")) && (0 != strcmp((pHCXName + surfixLen - 4),".hcx")))
         {
                   MessageBox("Please specify a valid output file name with .hcx as surfix.","Error",MB_OK);
                   return FALSE;
         }
 
         m_MajorVersion = atoi(m_strVersion);
         m_MinorVersion = atoi(m_strMinorVer);
         return TRUE;
}
//A helper routine used to initialize the HCX file's header given the necessary
//parameters.
BOOL CHcxbuildDlg::InitializeHCXHdr(__HCX_HEADER* pHeader,char* pBinFile,DWORD dwBinSize,BMPIMAGE* pBitmap)
{
         IMAGE_DOS_HEADER*       ImageDosHeader = NULL;
         IMAGE_NT_HEADERS*       ImageNtHeader = NULL;
         IMAGE_OPTIONAL_HEADER*  ImageOptionalHeader = NULL;
         DWORD                   dwOffset = 0;
 
         ImageDosHeader = (IMAGE_DOS_HEADER*)pBinFile;
         dwOffset = ImageDosHeader->e_lfanew;
         ImageNtHeader = (IMAGE_NT_HEADERS*)(pBinFile + dwOffset);
         ImageOptionalHeader = &(ImageNtHeader->OptionalHeader);
         //Check if the binary image is valid.
         if(ImageOptionalHeader->SectionAlignment != ImageOptionalHeader->FileAlignment)
         {
                   return FALSE;
         }
         pHeader->dwHcxSignature  = HCX_SIGNATURE;
         pHeader->dwEntryOffset   = ImageOptionalHeader->AddressOfEntryPoint;
         pHeader->dwEntryOffset  -= 8;  //Minus the three nop instructions and one jmp instructions.
 
         //Initialize the bitmap related variables.
         pHeader->dwBmpHeight  = pBitmap->height;
         pHeader->dwBmpWidth   = pBitmap->width;
         pHeader->dwColorBits  = pBitmap->BitCount;
         //pHeader->dwBmpOffset  = ROUND_TO_16(dwBinSize);
         pHeader->dwBmpOffset  = GetHCXFileSize(pBinFile);
         pHeader->dwBmpOffset  -= m_pBitmap->width * m_pBitmap->height * 3;  //Minutes the bitmap's size.
 
         //Initialize version and app name variables.
         strcpy(pHeader->AppName,m_strAppName);
         pHeader->MajorVersion  = this->m_MajorVersion;
         pHeader->MinorVersion  = this->m_MinorVersion;
         pHeader->OsMajorVersion = 1;   //Only comply to Hello China V1.75.
         pHeader->OsMinorVersion = 75;
 
         return TRUE;
}
 
//Build a HCX file.
BOOL CHcxbuildDlg::BuildHCX()
{
         HANDLE hBinFile       = INVALID_HANDLE_VALUE;  //Binary file's handle.
         HANDLE hTarget        = INVALID_HANDLE_VALUE;  //Target file's handle.
         __HCX_HEADER          hcxHdr;  //HCX file's header,will be initialized later.
         char*  pBinBuffer     = NULL;  //Buffer to hold bin file.
         char*  pHCXBuffer     = NULL;  //Buffer to hold target HCX file.
         char*  pBuffer        = NULL;  //Temporary variable used to operate other buffers.
         DWORD  dwBinSize      = 0;     //Bin file's size.
         DWORD  dwHCXSize      = 0;     //HCX file's size.
         BOOL   bResult        = FALSE; //Result indicator of this routine.
         DWORD  dwReadSize     = 0;     //Used to hold bytes number read from file.
 
         //Try to open the binary file.
         hBinFile = CreateFile(
                   this->m_strDllName,
                   GENERIC_READ,
                   FILE_SHARE_DELETE,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);
         if(INVALID_HANDLE_VALUE == hBinFile)
         {
                   goto __TERMINAL;
         }
         //Get bin file's size.
         dwBinSize = GetFileSize(hBinFile,&dwReadSize);
         if(0 == dwBinSize)  //Invalid file's size.
         {
                   goto __TERMINAL;
         }
         //Create temporary buffer and read the whole file.
         pBinBuffer = (char*)malloc(dwBinSize);
         if(NULL == pBinBuffer)
         {
                   goto __TERMINAL;
         }
         if(!ReadFile(hBinFile,pBinBuffer,dwBinSize,&dwReadSize,NULL))
         {
                   goto __TERMINAL;
         }
         if(dwReadSize != dwBinSize)  //Also invalid situation.
         {
                   goto __TERMINAL;
         }
         //Calculate target file(HCX file)'s size.
         dwHCXSize = GetHCXFileSize(pBinBuffer);
         if(0 == dwHCXSize)
         {
                   goto __TERMINAL;
         }
         TRACE("dwHCXSize = %d\r\n",dwHCXSize);
         //dwHCXSize = dwBinSize;
         //dwHCXSize = ROUND_TO_16(dwHCXSize);
         //dwHCXSize += m_pBitmap->width * m_pBitmap->height * 3; //Bitmap's size.
         //Allocate target file's temporary buffer.
         pHCXBuffer = (char*)malloc(dwHCXSize);
         if(NULL == pHCXBuffer)
         {
                   goto __TERMINAL;
         }
         //Open target file to write.
         hTarget = CreateFile(
                   this->m_strTarget,
                   GENERIC_WRITE,
                   FILE_SHARE_DELETE,
                   NULL,
                   OPEN_ALWAYS,
                   0,
                   NULL);
         if(NULL == hTarget)
         {
                   goto __TERMINAL;
         }
         //All right,initialize the HCX file's header.
         if(!InitializeHCXHdr(&hcxHdr,pBinBuffer,dwBinSize,m_pBitmap))
         {
                   goto __TERMINAL;
         }
         //Construct the target file's content.
         memcpy(pHCXBuffer,&hcxHdr,sizeof(__HCX_HEADER));
                    //Inflate sections into target file from source module.
                    pBuffer = InflateSections(pBinBuffer,pHCXBuffer);
                    if(NULL == pBuffer)
                    {
                             goto __TERMINAL;
                    }
         //memcpy((pHCXBuffer + sizeof(__HCX_HEADER)),(pBinBuffer + sizeof(__HCX_HEADER)),
         //          dwBinSize - sizeof(__HCX_HEADER));
         //pBuffer = pHCXBuffer + dwBinSize;
         memcpy(pBuffer,m_pBitmap->DataR,m_pBitmap->width * m_pBitmap->height);
         pBuffer += m_pBitmap->width * m_pBitmap->height;
         memcpy(pBuffer,m_pBitmap->DataB,m_pBitmap->width * m_pBitmap->height);
         pBuffer += m_pBitmap->width * m_pBitmap->height;
         memcpy(pBuffer,m_pBitmap->DataG,m_pBitmap->width * m_pBitmap->height);
         pBuffer += m_pBitmap->width * m_pBitmap->height;
                    TRACE("BuildHCX,pHCXBuffer's length = %d\r\n",pBuffer - pHCXBuffer);
 
         //OK,target buffer is ready,write it to target file.
         if(!WriteFile(hTarget,pHCXBuffer,dwHCXSize,&dwReadSize,NULL))
         {
                   goto __TERMINAL;
         }
                    if(dwReadSize != dwHCXSize)  //Write failed.
                    {
                             goto __TERMINAL;
                    }
         SetEndOfFile(hTarget);
 
         //Set success of the whole operation.
         bResult = TRUE;
 
__TERMINAL:
         //Release all resources.
         if(INVALID_HANDLE_VALUE != hBinFile)
         {
                   CloseHandle(hBinFile);
         }
         if(INVALID_HANDLE_VALUE != hTarget)
         {
                   CloseHandle(hTarget);
         }
         if(pBinBuffer)
         {
                   free(pBinBuffer);
         }
         if(pHCXBuffer)
         {
                   free(pHCXBuffer);
         }
         return bResult;
}
 
void CHcxbuildDlg::OnButtondll() 
{
	HANDLE       hBinFile   = INVALID_HANDLE_VALUE;
	char*        pBinBuff   = NULL;
	DWORD        dwBinSize  = 0;
	DWORD        dwReadSize = 0;

    CFileDialog ofDialog(TRUE,NULL,NULL,0,"DLL files(*.dll)|*.dll|All files(*.*)|*.*|");
    if(IDOK == ofDialog.DoModal())
    {
              m_strDllName = ofDialog.GetPathName();
              m_DllName.SetWindowText(m_strDllName);
    }
	//Now try to open the bin file to display it's general information.
	hBinFile = CreateFile(
		m_strDllName,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if(INVALID_HANDLE_VALUE == hBinFile)
	{
		MessageBox("Can not open the specified source file.","Error");
		goto __TERMINAL;
	}
	//Read the whole content.
	dwBinSize = GetFileSize(hBinFile,&dwReadSize);
	if(0 == dwBinSize)
	{
		MessageBox("The source file contains nothing.","Error");
		goto __TERMINAL;
	}
	pBinBuff = (char*)malloc(dwBinSize);
	if(NULL == pBinBuff)
	{
		goto __TERMINAL;
	}
	if(!ReadFile(hBinFile,
		pBinBuff,
		dwBinSize,
		&dwReadSize,
		NULL))
	{
		goto __TERMINAL;
	}
	if(dwBinSize != dwReadSize)  //Also is an exception case.
	{
		goto __TERMINAL;
	}

	//OK,try to show the general information.
	ShowDllInfo(pBinBuff);

__TERMINAL:
	if(INVALID_HANDLE_VALUE != hBinFile)
	{
		CloseHandle(hBinFile);
	}
	if(NULL != pBinBuff)
	{
		free(pBinBuff);
	}
	return;
}
 
 
void CHcxbuildDlg::OnButtonbmp() 
{
         CFileDialog ofDialog(TRUE,NULL,NULL,0,"Bitmap files(*.bmp)|*.bmp|All files(*.*)|*.*|");
         //CBitmap bitmap;
         //CRect rect;
         HANDLE hBmpFile  = INVALID_HANDLE_VALUE;  //Bitmap file's handle.
         CDC* pDC = m_Bitmap.GetDC();
         BOOL bResult = FALSE;
 
         if(IDOK == ofDialog.DoModal())
         {
                   m_strBmpName = ofDialog.GetPathName();
                   m_BmpName.SetWindowText(m_strBmpName);
                   //Try to open the bitmap file.
                   hBmpFile = CreateFile(m_strBmpName,
                            GENERIC_READ,
                            FILE_SHARE_DELETE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);
                   if(INVALID_HANDLE_VALUE == hBmpFile)
                   {
                            MessageBox("Can not open the bitmap file,please check it.","Error",MB_OK);
                            goto __TERMINAL;
                   }
                   //Now try to load the bitmap,but should release the previous one if exist.
                   if(NULL != m_pBitmap)
                   {
                            ImageDealloc(m_pBitmap);
                   }
                   this->m_pBitmap = LoadBitmap(hBmpFile);
                   if(NULL == m_pBitmap)
                   {
                            MessageBox("Can not load bitmap from file,color bits may incorrect.",MB_OK);
                            goto __TERMINAL;
                   }
                   //Check if the selected bitmap is legal,only 128 * 128 with 24 bits color is valid.
                   if((m_pBitmap->BitCount < 24) || (m_pBitmap->width != 128) || (m_pBitmap->height != 128))
                   {
                            MessageBox("Bitmap's format is illegal,only 128 * 128 with 24 bits color is valid.","Error",MB_OK);
                            goto __TERMINAL;
                   }
                   //Load OK,draw it in picture box.
                   ShowBitmap(pDC,2,2,128,128,m_pBitmap);
                   bResult = TRUE;
         }
 
__TERMINAL:
         //Close bitmap file.
         CloseHandle(hBmpFile);
         //Clear all resources if fail.
         if(!bResult)
         {
                   if(m_pBitmap)
                   {
                            ImageDealloc(m_pBitmap);
                            m_pBitmap = NULL;
                   }
                   m_strBmpName = "";
                   m_BmpName.SetWindowText("");
         }      
}
 
 
void CHcxbuildDlg::OnBuild() 
{
}
 
void CHcxbuildDlg::OnButtonbuild() 
{
         if(!CheckParam())
         {
                   return;
         }
         //Parameters are all OK,try to build the HCX file.
         if(BuildHCX())
         {
                   MessageBox("Build HCX file successfully.","Info",MB_OK);
         }
         else
         {
                   MessageBox("Build HCX file failed,please check the variables you offered.","Info",MB_OK);
         }               
}
 
//A helper routine used to calculate the target HCX file's size,by giving the source
//binary file.
//This routine will analyze each section in source file,and 'inflate' it to be as loaded
//into memory,then accumulate all these sections to get the actual HCX file's size.
DWORD CHcxbuildDlg::GetHCXFileSize(char* pBinFile)
{
         DWORD                         dwHCXSize     = 0;
         IMAGE_DOS_HEADER*             pDOSHdr       = (IMAGE_DOS_HEADER*)pBinFile;
         IMAGE_NT_HEADERS*             pNTHdrs       = NULL;
         IMAGE_FILE_HEADER*            pFileHdr      = NULL;
         IMAGE_OPTIONAL_HEADER*        pOptionalHdr  = NULL;
         IMAGE_SECTION_HEADER*         pSectionHdr   = NULL;
         DWORD                         dwSectNum     = 0;    //Section numbers in source file.
         DWORD                         i;
 
         if(NULL == pBinFile)
         {
                   goto __TERMINAL;
         }
 
         //Check the validate of the source bin file.
         if(0x5A4D != pDOSHdr->e_magic)
         {
                   goto __TERMINAL;
         }
         pNTHdrs = (IMAGE_NT_HEADERS*)(pBinFile + pDOSHdr->e_lfanew);
         pFileHdr = &pNTHdrs->FileHeader;
         pOptionalHdr = &pNTHdrs->OptionalHeader;
         pSectionHdr = (IMAGE_SECTION_HEADER*)(pBinFile + pDOSHdr->e_lfanew + sizeof(IMAGE_NT_HEADERS));
         dwSectNum = pFileHdr->NumberOfSections;
         TRACE("GetHCXFileSize,dwSectNum = %d\r\n",dwSectNum);
 
         //Calculate total size of all sections.
         for(i = 0;i < dwSectNum;i ++,pSectionHdr ++)
         {
                   if(dwHCXSize <= pSectionHdr->VirtualAddress)
                   {
                            dwHCXSize = pSectionHdr->VirtualAddress;
                            dwHCXSize += pSectionHdr->SizeOfRawData;
                   }
                   TRACE("GetHCXFile,dwHCXSize = %d\r\n",dwHCXSize);
                   TRACE("GetHCXFile,secname = %s\r\n",pSectionHdr->Name);
                   TRACE("GetHCXFile,virtual addr = %X\r\n",pSectionHdr->VirtualAddress);
                   TRACE("GetHCXFile,size of rd = %d\r\n",pSectionHdr->SizeOfRawData);
         }
         dwHCXSize = ROUND_TO_16(dwHCXSize);
    dwHCXSize += m_pBitmap->width * m_pBitmap->height * 3; //Bitmap's size.
 
__TERMINAL:
         return dwHCXSize;
}
 
char* CHcxbuildDlg::InflateSections(char *pBinFile, char *pHCXFile)
{
         char*                           psrcBuffer      = pBinFile;
         char*                           pdstBuffer      = pHCXFile;
         IMAGE_DOS_HEADER*               pDOSHdr         = (IMAGE_DOS_HEADER*)pBinFile;
         IMAGE_NT_HEADERS*               pNTHdrs         = NULL;
         IMAGE_FILE_HEADER*              pFileHdr        = NULL;
         IMAGE_OPTIONAL_HEADER*          pOptionalHdr    = NULL;
         IMAGE_SECTION_HEADER*           pSectionHdr     = NULL;
         DWORD                           dwSectNum       = 0;
         DWORD                           dwTotalLength   = 0;      //Total length of all sections.
         BOOL                            bResult         = FALSE;
         DWORD                           i;
 
         if((NULL == pBinFile) || (NULL == pHCXFile))
         {
                   goto __TERMINAL;
         }
         //Check the validate of source file.
         if(0x5A4D != pDOSHdr->e_magic)
         {
                   goto __TERMINAL;
         }
         pNTHdrs = (IMAGE_NT_HEADERS*)(pBinFile + pDOSHdr->e_lfanew);
         pFileHdr = &pNTHdrs->FileHeader;
         pOptionalHdr = &pNTHdrs->OptionalHeader;
         pSectionHdr  = (IMAGE_SECTION_HEADER*)(pBinFile + pDOSHdr->e_lfanew + sizeof(IMAGE_NT_HEADERS));
         dwSectNum = pFileHdr->NumberOfSections;
         if(0 == dwSectNum)  //Invalid value.
         {
                   goto __TERMINAL;
         }
 
         //Now inflate all sections in source file.
         for(i = 0;i < dwSectNum;i ++,pSectionHdr ++)
         {
                   if(pSectionHdr->VirtualAddress <= sizeof(__HCX_HEADER))  //File's format is invalid.
                   {
                            goto __TERMINAL;
                   }
                   psrcBuffer = pBinFile + pSectionHdr->PointerToRawData;
                   pdstBuffer = pHCXFile + pSectionHdr->VirtualAddress;
                   if(0 == pSectionHdr->PointerToRawData)  //Maybe a .bss section.
                   {
                            memset(pdstBuffer,0,pSectionHdr->SizeOfRawData);
                   }
                   else
                   {
                            memcpy(pdstBuffer,psrcBuffer,pSectionHdr->SizeOfRawData);
                   }
                   dwTotalLength = pSectionHdr->VirtualAddress + pSectionHdr->SizeOfRawData;
                   //pdstBuffer += pSectionHdr->SizeOfRawData;  //Adjust dst buffer pointing to current section's ending point.
         }
         //Round to 16.
         dwTotalLength = ROUND_TO_16(dwTotalLength);
         pdstBuffer = pHCXFile + dwTotalLength;
         bResult = TRUE;
         
__TERMINAL:
         if(bResult)
         {
                   return pdstBuffer;
         }
         else
         {
                   return NULL;
         }
}

BOOL CHcxbuildDlg::ShowDllInfo(char *pDllFile)
{
	IMAGE_DOS_HEADER*         pDOSHdr   = (IMAGE_DOS_HEADER*)pDllFile;
	IMAGE_NT_HEADERS*         pNTHdr    = NULL;
	IMAGE_FILE_HEADER*        pFileHdr  = NULL;
	IMAGE_OPTIONAL_HEADER*    pOptionalHdr = NULL;
	IMAGE_SECTION_HEADER*     pSectionHdr  = NULL;
	CString                   strTmp       = "";
	BOOL                      bResult      = FALSE;
	DWORD                     dwSectNum,i;

	if(NULL == pDllFile)
	{
		return FALSE;
	}
	if(0x5A4D != pDOSHdr->e_magic)  //Is not a valid MZ file.
	{
		goto __TERMINAL;
	}
	pNTHdr = (IMAGE_NT_HEADERS*)(pDllFile + pDOSHdr->e_lfanew);
	pFileHdr = &pNTHdr->FileHeader;
	pOptionalHdr = &pNTHdr->OptionalHeader;
	pSectionHdr  = (IMAGE_SECTION_HEADER*)(pDllFile + pDOSHdr->e_lfanew + sizeof(IMAGE_NT_HEADERS));
	dwSectNum = pFileHdr->NumberOfSections;

	//Gather the source bin file's information.
	m_strDllInfo = "";  //Clear info container first.
	strTmp.Format("  Entry point     : 0x%X\r\n",pOptionalHdr->AddressOfEntryPoint);
	m_strDllInfo += strTmp;
	strTmp.Format("  File alignment  : 0x%X\r\n",pOptionalHdr->FileAlignment);
	m_strDllInfo += strTmp;
	strTmp.Format("  Sect alignment  : 0x%X\r\n",pOptionalHdr->SectionAlignment);
	m_strDllInfo += strTmp;
	m_strDllInfo += "\r\n";
	strTmp.Format("  Total %d sections,verbose info as:\r\n",dwSectNum);
	m_strDllInfo += strTmp;
	//strTmp.Format("    name      mem_rav   file_rav  size\r\n");
	strTmp.Format("  %8s    %8s    %8s  %8s\r\n",
		"name",
		"mem_rav",
		"file_rav",
		"size");
	m_strDllInfo += strTmp;
	for(i = 0;i < dwSectNum;i ++)
	{
		strTmp.Format("  %8s  0x%08X  0x%08X  %8d\r\n",
			pSectionHdr->Name,
			pSectionHdr->VirtualAddress,
			pSectionHdr->PointerToRawData,
			pSectionHdr->SizeOfRawData);
		m_strDllInfo += strTmp;
		pSectionHdr ++;  //Process next section.
	}
	//UpdateData(TRUE);
	UpdateData(FALSE);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}
