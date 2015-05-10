# Microsoft Developer Studio Generated NMAKE File, Based on master.dsp
!IF "$(CFG)" == ""
CFG=master - Win32 Debug
!MESSAGE No configuration specified. Defaulting to master - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "master - Win32 Release" && "$(CFG)" != "master - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "master.mak" CFG="master - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "master - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "master - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "master - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\master.dll"


CLEAN :
	-@erase "$(INTDIR)\ARCH_X86.OBJ"
	-@erase "$(INTDIR)\BUFFMGR.OBJ"
	-@erase "$(INTDIR)\COMQUEUE.OBJ"
	-@erase "$(INTDIR)\DEVMGR.OBJ"
	-@erase "$(INTDIR)\dim.obj"
	-@erase "$(INTDIR)\EXTCMD.OBJ"
	-@erase "$(INTDIR)\FIBONACCI.OBJ"
	-@erase "$(INTDIR)\HCNAPI.OBJ"
	-@erase "$(INTDIR)\HEAP.OBJ"
	-@erase "$(INTDIR)\HELLOCN.OBJ"
	-@erase "$(INTDIR)\IDEHDDRV.OBJ"
	-@erase "$(INTDIR)\IOCTRL_S.OBJ"
	-@erase "$(INTDIR)\iomgr.obj"
	-@erase "$(INTDIR)\IPV4_IMP.OBJ"
	-@erase "$(INTDIR)\KEYHDLR.OBJ"
	-@erase "$(INTDIR)\KMEMMGR.OBJ"
	-@erase "$(INTDIR)\KTHREAD.OBJ"
	-@erase "$(INTDIR)\KTMGR.OBJ"
	-@erase "$(INTDIR)\LOW_API.OBJ"
	-@erase "$(INTDIR)\MAILBOX.OBJ"
	-@erase "$(INTDIR)\memmgr.obj"
	-@erase "$(INTDIR)\NET_COMM.OBJ"
	-@erase "$(INTDIR)\NETBUFF.OBJ"
	-@erase "$(INTDIR)\OBJMGR.OBJ"
	-@erase "$(INTDIR)\OBJQUEUE.OBJ"
	-@erase "$(INTDIR)\OS_ENTRY.OBJ"
	-@erase "$(INTDIR)\PAGEIDX.OBJ"
	-@erase "$(INTDIR)\PCI_DRV.OBJ"
	-@erase "$(INTDIR)\PERF.OBJ"
	-@erase "$(INTDIR)\RT8139.OBJ"
	-@erase "$(INTDIR)\SHELL.OBJ"
	-@erase "$(INTDIR)\STRING.OBJ"
	-@erase "$(INTDIR)\SYN_MECH.OBJ"
	-@erase "$(INTDIR)\SYNOBJ.OBJ"
	-@erase "$(INTDIR)\SYSD_S.OBJ"
	-@erase "$(INTDIR)\SYSTEM.OBJ"
	-@erase "$(INTDIR)\TASKCTRL.OBJ"
	-@erase "$(INTDIR)\TIMER.OBJ"
	-@erase "$(INTDIR)\TYPES.OBJ"
	-@erase "$(INTDIR)\UDP_IMP.OBJ"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VMM.OBJ"
	-@erase "$(OUTDIR)\master.dll"
	-@erase "$(OUTDIR)\master.exp"
	-@erase "$(OUTDIR)\master.lib"
	-@erase "$(OUTDIR)\master.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MASTER_EXPORTS" /Fp"$(INTDIR)\master.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\master.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x110000" /entry:"?__init@@YAXXZ" /dll /incremental:no /pdb:"$(OUTDIR)\master.pdb" /map:"$(INTDIR)\master.map" /machine:I386 /out:"$(OUTDIR)\master.dll" /implib:"$(OUTDIR)\master.lib" /ALIGN:16 
LINK32_OBJS= \
	"$(INTDIR)\IPV4_IMP.OBJ" \
	"$(INTDIR)\LOW_API.OBJ" \
	"$(INTDIR)\NET_COMM.OBJ" \
	"$(INTDIR)\NETBUFF.OBJ" \
	"$(INTDIR)\UDP_IMP.OBJ" \
	"$(INTDIR)\PCI_DRV.OBJ" \
	"$(INTDIR)\RT8139.OBJ" \
	"$(INTDIR)\BUFFMGR.OBJ" \
	"$(INTDIR)\COMQUEUE.OBJ" \
	"$(INTDIR)\DEVMGR.OBJ" \
	"$(INTDIR)\dim.obj" \
	"$(INTDIR)\HEAP.OBJ" \
	"$(INTDIR)\HELLOCN.OBJ" \
	"$(INTDIR)\IDEHDDRV.OBJ" \
	"$(INTDIR)\iomgr.obj" \
	"$(INTDIR)\KEYHDLR.OBJ" \
	"$(INTDIR)\KMEMMGR.OBJ" \
	"$(INTDIR)\KTHREAD.OBJ" \
	"$(INTDIR)\KTMGR.OBJ" \
	"$(INTDIR)\MAILBOX.OBJ" \
	"$(INTDIR)\memmgr.obj" \
	"$(INTDIR)\OBJMGR.OBJ" \
	"$(INTDIR)\OBJQUEUE.OBJ" \
	"$(INTDIR)\PAGEIDX.OBJ" \
	"$(INTDIR)\PERF.OBJ" \
	"$(INTDIR)\SHELL.OBJ" \
	"$(INTDIR)\SYN_MECH.OBJ" \
	"$(INTDIR)\SYNOBJ.OBJ" \
	"$(INTDIR)\SYSTEM.OBJ" \
	"$(INTDIR)\TASKCTRL.OBJ" \
	"$(INTDIR)\TIMER.OBJ" \
	"$(INTDIR)\TYPES.OBJ" \
	"$(INTDIR)\VMM.OBJ" \
	"$(INTDIR)\STRING.OBJ" \
	"$(INTDIR)\EXTCMD.OBJ" \
	"$(INTDIR)\FIBONACCI.OBJ" \
	"$(INTDIR)\HCNAPI.OBJ" \
	"$(INTDIR)\OS_ENTRY.OBJ" \
	"$(INTDIR)\IOCTRL_S.OBJ" \
	"$(INTDIR)\SYSD_S.OBJ" \
	"$(INTDIR)\ARCH_X86.OBJ"

"$(OUTDIR)\master.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "master - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\master.dll"


CLEAN :
	-@erase "$(INTDIR)\ARCH_X86.OBJ"
	-@erase "$(INTDIR)\BUFFMGR.OBJ"
	-@erase "$(INTDIR)\COMQUEUE.OBJ"
	-@erase "$(INTDIR)\DEVMGR.OBJ"
	-@erase "$(INTDIR)\dim.obj"
	-@erase "$(INTDIR)\EXTCMD.OBJ"
	-@erase "$(INTDIR)\FIBONACCI.OBJ"
	-@erase "$(INTDIR)\HCNAPI.OBJ"
	-@erase "$(INTDIR)\HEAP.OBJ"
	-@erase "$(INTDIR)\HELLOCN.OBJ"
	-@erase "$(INTDIR)\IDEHDDRV.OBJ"
	-@erase "$(INTDIR)\IOCTRL_S.OBJ"
	-@erase "$(INTDIR)\iomgr.obj"
	-@erase "$(INTDIR)\IPV4_IMP.OBJ"
	-@erase "$(INTDIR)\KEYHDLR.OBJ"
	-@erase "$(INTDIR)\KMEMMGR.OBJ"
	-@erase "$(INTDIR)\KTHREAD.OBJ"
	-@erase "$(INTDIR)\KTMGR.OBJ"
	-@erase "$(INTDIR)\LOW_API.OBJ"
	-@erase "$(INTDIR)\MAILBOX.OBJ"
	-@erase "$(INTDIR)\memmgr.obj"
	-@erase "$(INTDIR)\NET_COMM.OBJ"
	-@erase "$(INTDIR)\NETBUFF.OBJ"
	-@erase "$(INTDIR)\OBJMGR.OBJ"
	-@erase "$(INTDIR)\OBJQUEUE.OBJ"
	-@erase "$(INTDIR)\OS_ENTRY.OBJ"
	-@erase "$(INTDIR)\PAGEIDX.OBJ"
	-@erase "$(INTDIR)\PCI_DRV.OBJ"
	-@erase "$(INTDIR)\PERF.OBJ"
	-@erase "$(INTDIR)\RT8139.OBJ"
	-@erase "$(INTDIR)\SHELL.OBJ"
	-@erase "$(INTDIR)\STRING.OBJ"
	-@erase "$(INTDIR)\SYN_MECH.OBJ"
	-@erase "$(INTDIR)\SYNOBJ.OBJ"
	-@erase "$(INTDIR)\SYSD_S.OBJ"
	-@erase "$(INTDIR)\SYSTEM.OBJ"
	-@erase "$(INTDIR)\TASKCTRL.OBJ"
	-@erase "$(INTDIR)\TIMER.OBJ"
	-@erase "$(INTDIR)\TYPES.OBJ"
	-@erase "$(INTDIR)\UDP_IMP.OBJ"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VMM.OBJ"
	-@erase "$(OUTDIR)\master.dll"
	-@erase "$(OUTDIR)\master.exp"
	-@erase "$(OUTDIR)\master.ilk"
	-@erase "$(OUTDIR)\master.lib"
	-@erase "$(OUTDIR)\master.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MASTER_EXPORTS" /Fp"$(INTDIR)\master.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\master.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x110000" /entry:"?__init@@YAXXZ" /dll /incremental:yes /pdb:"$(OUTDIR)\master.pdb" /debug /machine:I386 /out:"$(OUTDIR)\master.dll" /implib:"$(OUTDIR)\master.lib" /pdbtype:sept /ALIGN:16 
LINK32_OBJS= \
	"$(INTDIR)\IPV4_IMP.OBJ" \
	"$(INTDIR)\LOW_API.OBJ" \
	"$(INTDIR)\NET_COMM.OBJ" \
	"$(INTDIR)\NETBUFF.OBJ" \
	"$(INTDIR)\UDP_IMP.OBJ" \
	"$(INTDIR)\PCI_DRV.OBJ" \
	"$(INTDIR)\RT8139.OBJ" \
	"$(INTDIR)\BUFFMGR.OBJ" \
	"$(INTDIR)\COMQUEUE.OBJ" \
	"$(INTDIR)\DEVMGR.OBJ" \
	"$(INTDIR)\dim.obj" \
	"$(INTDIR)\HEAP.OBJ" \
	"$(INTDIR)\HELLOCN.OBJ" \
	"$(INTDIR)\IDEHDDRV.OBJ" \
	"$(INTDIR)\iomgr.obj" \
	"$(INTDIR)\KEYHDLR.OBJ" \
	"$(INTDIR)\KMEMMGR.OBJ" \
	"$(INTDIR)\KTHREAD.OBJ" \
	"$(INTDIR)\KTMGR.OBJ" \
	"$(INTDIR)\MAILBOX.OBJ" \
	"$(INTDIR)\memmgr.obj" \
	"$(INTDIR)\OBJMGR.OBJ" \
	"$(INTDIR)\OBJQUEUE.OBJ" \
	"$(INTDIR)\PAGEIDX.OBJ" \
	"$(INTDIR)\PERF.OBJ" \
	"$(INTDIR)\SHELL.OBJ" \
	"$(INTDIR)\SYN_MECH.OBJ" \
	"$(INTDIR)\SYNOBJ.OBJ" \
	"$(INTDIR)\SYSTEM.OBJ" \
	"$(INTDIR)\TASKCTRL.OBJ" \
	"$(INTDIR)\TIMER.OBJ" \
	"$(INTDIR)\TYPES.OBJ" \
	"$(INTDIR)\VMM.OBJ" \
	"$(INTDIR)\STRING.OBJ" \
	"$(INTDIR)\EXTCMD.OBJ" \
	"$(INTDIR)\FIBONACCI.OBJ" \
	"$(INTDIR)\HCNAPI.OBJ" \
	"$(INTDIR)\OS_ENTRY.OBJ" \
	"$(INTDIR)\IOCTRL_S.OBJ" \
	"$(INTDIR)\SYSD_S.OBJ" \
	"$(INTDIR)\ARCH_X86.OBJ"

"$(OUTDIR)\master.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("master.dep")
!INCLUDE "master.dep"
!ELSE 
!MESSAGE Warning: cannot find "master.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "master - Win32 Release" || "$(CFG)" == "master - Win32 Debug"
SOURCE=.\NetCore\IPV4_IMP.CPP

"$(INTDIR)\IPV4_IMP.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\NetCore\LOW_API.CPP

"$(INTDIR)\LOW_API.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\NetCore\NET_COMM.CPP

"$(INTDIR)\NET_COMM.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\NetCore\NETBUFF.CPP

"$(INTDIR)\NETBUFF.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\NetCore\UDP_IMP.CPP

"$(INTDIR)\UDP_IMP.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Drivers\PCI_DRV.CPP

"$(INTDIR)\PCI_DRV.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Drivers\RT8139.CPP

"$(INTDIR)\RT8139.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\BUFFMGR.CPP

"$(INTDIR)\BUFFMGR.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\COMQUEUE.CPP

"$(INTDIR)\COMQUEUE.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\DEVMGR.CPP

"$(INTDIR)\DEVMGR.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\dim.cpp

"$(INTDIR)\dim.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\HEAP.CPP

"$(INTDIR)\HEAP.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\HELLOCN.CPP

"$(INTDIR)\HELLOCN.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\IDEHDDRV.CPP

"$(INTDIR)\IDEHDDRV.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\iomgr.cpp

"$(INTDIR)\iomgr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\KEYHDLR.CPP

"$(INTDIR)\KEYHDLR.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\KMEMMGR.CPP

"$(INTDIR)\KMEMMGR.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\KTHREAD.CPP

"$(INTDIR)\KTHREAD.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\KTMGR.CPP

"$(INTDIR)\KTMGR.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\MAILBOX.CPP

"$(INTDIR)\MAILBOX.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\memmgr.cpp

"$(INTDIR)\memmgr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\OBJMGR.CPP

"$(INTDIR)\OBJMGR.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\OBJQUEUE.CPP

"$(INTDIR)\OBJQUEUE.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\PAGEIDX.CPP

"$(INTDIR)\PAGEIDX.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\PERF.CPP

"$(INTDIR)\PERF.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\SHELL.CPP

"$(INTDIR)\SHELL.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\SYN_MECH.CPP

"$(INTDIR)\SYN_MECH.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\SYNOBJ.CPP

"$(INTDIR)\SYNOBJ.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\SYSTEM.CPP

"$(INTDIR)\SYSTEM.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\TASKCTRL.CPP

"$(INTDIR)\TASKCTRL.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\TIMER.CPP

"$(INTDIR)\TIMER.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\TYPES.CPP

"$(INTDIR)\TYPES.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Kernel\VMM.CPP

"$(INTDIR)\VMM.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\LIB\STRING.CPP

"$(INTDIR)\STRING.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\KTHREAD\IOCTRL_S.CPP

"$(INTDIR)\IOCTRL_S.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\KTHREAD\SYSD_S.CPP

"$(INTDIR)\SYSD_S.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ARCH\ARCH_X86.CPP

"$(INTDIR)\ARCH_X86.OBJ" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\EXTCMD.CPP

"$(INTDIR)\EXTCMD.OBJ" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FIBONACCI.CPP

"$(INTDIR)\FIBONACCI.OBJ" : $(SOURCE) "$(INTDIR)"


SOURCE=.\HCNAPI.CPP

"$(INTDIR)\HCNAPI.OBJ" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OS_ENTRY.CPP

"$(INTDIR)\OS_ENTRY.OBJ" : $(SOURCE) "$(INTDIR)"



!ENDIF 

