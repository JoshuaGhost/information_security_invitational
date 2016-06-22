# Microsoft Developer Studio Project File - Name="Defender" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Defender - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Defender.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Defender.mak" CFG="Defender - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Defender - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Defender - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Defender - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Defender - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib Psapi.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Defender - Win32 Release"
# Name "Defender - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\basic\ado.cpp
# End Source File
# Begin Source File

SOURCE=.\AdvanceSetting.cpp
# End Source File
# Begin Source File

SOURCE=.\AutorunPage.cpp
# End Source File
# Begin Source File

SOURCE=.\BasicSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\net\collection.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectPage.cpp
# End Source File
# Begin Source File

SOURCE=.\DangerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Defender.cpp
# End Source File
# Begin Source File

SOURCE=.\DefenderDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\DefenderView.cpp
# End Source File
# Begin Source File

SOURCE=.\EdithostsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HostsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadcodeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalPage.cpp
# End Source File
# Begin Source File

SOURCE=.\LogSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\net\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\ModuleDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetlogPage.cpp
# End Source File
# Begin Source File

SOURCE=.\net\Packet.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessPage.cpp
# End Source File
# Begin Source File

SOURCE=.\ProtectFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ProtectSetPage.cpp
# End Source File
# Begin Source File

SOURCE=.\ProtectSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\ServicePage.cpp
# End Source File
# Begin Source File

SOURCE=.\SetSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SystemImageList.cpp
# End Source File
# Begin Source File

SOURCE=.\WhiteFile.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\basic\ado.h
# End Source File
# Begin Source File

SOURCE=.\AdvanceSetting.h
# End Source File
# Begin Source File

SOURCE=.\AutorunPage.h
# End Source File
# Begin Source File

SOURCE=.\BasicSheet.h
# End Source File
# Begin Source File

SOURCE=.\net\collection.h
# End Source File
# Begin Source File

SOURCE=.\ConnectPage.h
# End Source File
# Begin Source File

SOURCE=.\DangerDlg.h
# End Source File
# Begin Source File

SOURCE=.\Defender.h
# End Source File
# Begin Source File

SOURCE=.\DefenderDoc.h
# End Source File
# Begin Source File

SOURCE=.\DefenderView.h
# End Source File
# Begin Source File

SOURCE=.\EdithostsDlg.h
# End Source File
# Begin Source File

SOURCE=.\HostsPage.h
# End Source File
# Begin Source File

SOURCE=.\net\IOCommon.h
# End Source File
# Begin Source File

SOURCE=.\ioctl.h
# End Source File
# Begin Source File

SOURCE="..\..\Program Files\Microsoft Platform SDK for Windows XP SP2\Include\IPHlpApi.h"
# End Source File
# Begin Source File

SOURCE=.\LoadcodeDlg.h
# End Source File
# Begin Source File

SOURCE=.\LocalPage.h
# End Source File
# Begin Source File

SOURCE=.\LogSheet.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\net\md5.h
# End Source File
# Begin Source File

SOURCE=.\ModuleDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetlogPage.h
# End Source File
# Begin Source File

SOURCE=.\net\Packet.h
# End Source File
# Begin Source File

SOURCE=.\ProcessPage.h
# End Source File
# Begin Source File

SOURCE=.\ProtectFile.h
# End Source File
# Begin Source File

SOURCE=.\ProtectSetPage.h
# End Source File
# Begin Source File

SOURCE=.\ProtectSheet.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ServicePage.h
# End Source File
# Begin Source File

SOURCE=.\SetSheet.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SystemImageList.h
# End Source File
# Begin Source File

SOURCE=.\WhiteFile.h
# End Source File
# Begin Source File

SOURCE=.\net\Winpcap.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\back.bmp
# End Source File
# Begin Source File

SOURCE=.\res\backimg_all.bmp
# End Source File
# Begin Source File

SOURCE=.\res\backimg_loadcode.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Defender.ico
# End Source File
# Begin Source File

SOURCE=.\Defender.rc
# End Source File
# Begin Source File

SOURCE=.\res\Defender.rc2
# End Source File
# Begin Source File

SOURCE=.\res\DefenderDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\iads.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\TOOLBAR_COLD.BMP
# End Source File
# Begin Source File

SOURCE=.\res\TOOLBAR_HOT.BMP
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
