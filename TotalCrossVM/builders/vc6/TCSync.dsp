# Microsoft Developer Studio Project File - Name="TCSync" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TCSync - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "TCSync.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "TCSync.mak" CFG="TCSync - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "TCSync - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TCSync - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TCSync - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../temp/vc6/Conduit/Release"
# PROP Intermediate_Dir "../../../../temp/vc6/Conduit/Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TCSync_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Zp4 /W3 /O2 /I "..\..\src\util" /I "..\..\src\tcvm" /I "p:\extlibs\win32\msinttypes" /I "P:\extlibs\cdk601\C++\Common\include" /I "P:\extlibs\cdk601\C++\Win\include" /I "P:\extlibs\wceplatsdk\desktop\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "SYNC_EXPORTS" /D "LITTLE_ENDIAN" /D "_RAPI_" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x416 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib ole32.lib ../../../../temp/vc6/TCVM/release/TCVM.lib /nologo /dll /pdb:none /machine:I386 /out:"../../../../output/release/TotalCrossSDK/dist/vm/win32/TCSync.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=mkdir ..\..\..\..\output\release\TotalCrossVMS\dist\vm\win32	mkdir ..\..\..\..\output\release\TotalCrossVMS_NORAS\dist\vm\win32	copy ..\..\..\..\output\release\TotalCrossSDK\dist\vm\win32\TCSync.dll ..\..\..\..\output\release\TotalCrossVMS\dist\vm\win32	copy ..\..\..\..\output\release\TotalCrossSDK\dist\vm\win32\TCSync.dll ..\..\..\..\output\release\TotalCrossVMS_NORAS\dist\vm\win32
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TCSync - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../temp/vc6/Conduit/Debug"
# PROP Intermediate_Dir "../../../../temp/vc6/Conduit/Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TCSync_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp4 /W3 /Gm /GX /Zi /Od /I "..\..\src\util" /I "..\..\src\tcvm" /I "p:\extlibs\win32\msinttypes" /I "P:\extlibs\cdk601\C++\Common\include" /I "P:\extlibs\cdk601\C++\Win\include" /I "P:\extlibs\wceplatsdk\desktop\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SYNC_EXPORTS" /D "LITTLE_ENDIAN" /D "_RAPI_" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x416 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../../../temp/vc6/TCVM/Debug/TCVM.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../../output/debug/TotalCrossSDK/dist/vm/win32/TCSync.dll" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=mkdir ..\..\..\..\output\debug\TotalCrossVMS\dist\vm\win32	mkdir ..\..\..\..\output\debug\TotalCrossVMS_NORAS\dist\vm\win32	copy ..\..\..\..\output\debug\TotalCrossSDK\dist\vm\win32\TCSync.dll ..\..\..\..\output\debug\TotalCrossVMS\dist\vm\win32	copy ..\..\..\..\output\debug\TotalCrossSDK\dist\vm\win32\TCSync.dll ..\..\..\..\output\debug\TotalCrossVMS_NORAS\dist\vm\win32
# End Special Build Tool

!ENDIF

# Begin Target

# Name "TCSync - Win32 Release"
# Name "TCSync - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\sync\Conduit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\palmdb\palmdb.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project