# Microsoft Developer Studio Project File - Name="HCNGUI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=HCNGUI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HCNGUI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HCNGUI.mak" CFG="HCNGUI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HCNGUI - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "HCNGUI - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HCNGUI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HCNGUI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HCNGUI_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x160000" /entry:"__init" /dll /machine:I386 /ALIGN:16
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "HCNGUI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HCNGUI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HCNGUI_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "HCNGUI - Win32 Release"
# Name "HCNGUI - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "WINDOW"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\WINDOW\CLIPZONE.CPP
# End Source File
# Begin Source File

SOURCE=.\WINDOW\DEFWPROC.CPP
# End Source File
# Begin Source File

SOURCE=.\WINDOW\GDI.CPP
# End Source File
# Begin Source File

SOURCE=.\WINDOW\WNDMGR.CPP
# End Source File
# End Group
# Begin Group "CTRL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CTRL\bmpbtn.cpp
# End Source File
# Begin Source File

SOURCE=.\CTRL\BUTTON.CPP
# End Source File
# Begin Source File

SOURCE=.\CTRL\MSGBOX.CPP
# End Source File
# End Group
# Begin Group "KAPI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\KAPI\KAPI.CPP
# End Source File
# Begin Source File

SOURCE=.\KAPI\math.cpp
# End Source File
# Begin Source File

SOURCE=.\KAPI\stdio.CPP
# End Source File
# Begin Source File

SOURCE=.\KAPI\STRING.CPP
# End Source File
# End Group
# Begin Group "KTHREAD"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\KTHREAD\APPBAND.CPP
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\clend.cpp
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\clock.cpp
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\GUISHELL.CPP
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\GUIWPROC.CPP
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\launch.cpp
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\MOUSEMGR.CPP
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\RAWIT.CPP
# End Source File
# End Group
# Begin Group "WORD"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\WORD\WordLib.CPP
# End Source File
# End Group
# Begin Group "PICTURE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PICTURE\BMPAPI.cpp
# End Source File
# End Group
# Begin Group "RES"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RES\data.cpp
# End Source File
# End Group
# Begin Group "VIDEO"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VIDEO\GLOBAL.CPP
# End Source File
# Begin Source File

SOURCE=.\VIDEO\VIDEO.CPP
# End Source File
# End Group
# Begin Group "DRAW"

# PROP Default_Filter ""
# End Group
# Begin Group "APP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\APP\CLENDAR.CPP
# End Source File
# Begin Source File

SOURCE=.\APP\CLENDAR.H
# End Source File
# Begin Source File

SOURCE=.\APP\HELLOW.CPP
# End Source File
# Begin Source File

SOURCE=.\APP\HELLOW.H
# End Source File
# End Group
# Begin Group "syscall"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\syscall\syscall.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\GUIENTRY.CPP
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\INCLUDE\BMPAPI.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\bmpbtn.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\BUTTON.H
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\clend.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CLIPZONE.H
# End Source File
# Begin Source File

SOURCE=.\KTHREAD\clock.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\GDI.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\GLOBAL.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\GUISHELL.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\KAPI.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\launch.h
# End Source File
# Begin Source File

SOURCE=.\KAPI\math.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\MSGBOX.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\RAWIT.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\stdio.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\string.h
# End Source File
# Begin Source File

SOURCE=.\syscall\syscall.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\VESA.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\VIDEO.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\WNDMGR.H
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\WordLib.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
