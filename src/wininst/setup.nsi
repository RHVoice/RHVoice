# Copyright (C) 2010, 2011, 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

!include "LogicLib.nsh"
!include "Library.nsh"
!include "x64.nsh"

!ifndef VERSION
!define VERSION "0.4-a3"
!endif

!define INSTDIR_REG_ROOT "HKLM"
!define INSTDIR_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\RHVoice"
!include AdvUninstLog.nsh
!define PROGRAM_REG_KEY "Software\RHVoice"
!define SPTOKENENUMS_REG_KEY "SOFTWARE\Microsoft\Speech\Voices\TokenEnums"
!define CLSID "{d7577808-7ade-4dea-a5b7-ee314d6ef3a1}"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

LicenseLangString readmeFile ${LANG_ENGLISH} en.txt
LicenseLangString readmeFile ${LANG_RUSSIAN} ru.txt
LangString readmeCaption ${LANG_ENGLISH} ": Readme"
LangString readmeCaption ${LANG_RUSSIAN} ": О программе"
LangString readmeText ${LANG_ENGLISH} "General information about RHVoice"
LangString readmeText ${LANG_RUSSIAN} "Общие сведения об RHVoice"

SetCompressor /solid lzma
SetOverwrite on
AllowSkipFiles off
AutoCloseWindow false
CRCCheck on
ShowInstDetails show
ShowUninstDetails show

InstallDir "$PROGRAMFILES\RHVoice"
InstallDirRegKey ${INSTDIR_REG_ROOT} "${INSTDIR_REG_KEY}" "InstallDir"
Name "RHVoice v${VERSION}"
OutFile "..\..\build\windows_x86\RHVoice-v${VERSION}-setup.exe"
RequestExecutionLevel admin
!insertmacro UNATTENDED_UNINSTALL

PageEx license
Caption $(readmeCaption)
LicenseText $(readmeText) $(^NextBtn)
LicenseData $(readmeFile)
PageExEnd
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section
SetOutPath "$INSTDIR\lib"
!insertmacro UNINSTALL.LOG_OPEN_INSTALL
!insertmacro installLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED ..\..\build\windows_x86\sapi\RHVoiceSvr.dll "$INSTDIR\lib\RHVoiceSvr32.dll" $INSTDIR
!define LIBRARY_X64
${If} ${RunningX64}
!insertmacro installLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED ..\..\build\windows_x64\sapi\RHVoiceSvr.dll "$INSTDIR\lib\RHVoiceSvr64.dll" $INSTDIR
${EndIf}
!undef LIBRARY_X64      
!insertmacro UNINSTALL.LOG_CLOSE_INSTALL
SetOutPath "$INSTDIR\data"
!insertmacro UNINSTALL.LOG_OPEN_INSTALL
File /r /x .git* /x SConscript ..\..\data\*.*
!insertmacro UNINSTALL.LOG_CLOSE_INSTALL
SetOutPath "$INSTDIR\doc"
!insertmacro UNINSTALL.LOG_OPEN_INSTALL
File ..\..\build\windows_x86\COPYING.txt
!insertmacro UNINSTALL.LOG_CLOSE_INSTALL
SetOutPath "$INSTDIR\config-examples"
!insertmacro UNINSTALL.LOG_OPEN_INSTALL
File ..\..\build\windows_x86\RHVoice.ini
!insertmacro UNINSTALL.LOG_CLOSE_INSTALL
WriteRegStr ${INSTDIR_REG_ROOT} "${SPTOKENENUMS_REG_KEY}\RHVoice" "CLSID" ${CLSID}
WriteRegStr ${INSTDIR_REG_ROOT} ${PROGRAM_REG_KEY} "data_path" "$INSTDIR\data"
${If} ${RunningX64}
SetRegView 64
WriteRegStr ${INSTDIR_REG_ROOT} "${SPTOKENENUMS_REG_KEY}\RHVoice" "CLSID" ${CLSID}
WriteRegStr ${INSTDIR_REG_ROOT} ${PROGRAM_REG_KEY} "data_path" "$INSTDIR\data"
SetRegView 32
${EndIf}
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "INSTDIR" $INSTDIR
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "DisplayName" "RHVoice"
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "UninstallString" "${UNINST_EXE}"
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "Publisher" "Olga Yakovleva"
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "InstallLocation" $INSTDIR
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "DisplayVersion" ${VERSION}
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "URLUpdateInfo" "http://github.com/Olga-Yakovleva/RHVoice/downloads"
WriteRegDWORD ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "NoModify" 1
WriteRegDWORD ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "NoRepair" 1

SectionEnd

Function .onInit
!insertmacro UNINSTALL.LOG_PREPARE_INSTALL
FunctionEnd

Function .onInstSuccess
!insertmacro UNINSTALL.LOG_UPDATE_INSTALL
FunctionEnd

Section UnInstall
${If} ${RunningX64}
SetRegView 64
DeleteRegKey ${INSTDIR_REG_ROOT} "${SPTOKENENUMS_REG_KEY}\RHVoice"
DeleteRegKey ${INSTDIR_REG_ROOT} ${PROGRAM_REG_KEY}
SetRegView 32
!define LIBRARY_X64
!insertmacro UnInstallLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED "$INSTDIR\lib\RHVoiceSvr64.dll"
!undef LIBRARY_X64      
${EndIf}
DeleteRegKey ${INSTDIR_REG_ROOT} ${PROGRAM_REG_KEY}
deleteRegKey ${INSTDIR_REG_ROOT} "${SPTOKENENUMS_REG_KEY}\RHVoice"
!insertmacro UnInstallLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED "$INSTDIR\lib\RHVoiceSvr32.dll"
!insertmacro UNINSTALL.LOG_BEGIN_UNINSTALL
!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\config-examples"
!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\doc"
!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\data"
!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\lib"
!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR"
!insertmacro UNINSTALL.LOG_END_UNINSTALL
DeleteRegKey ${INSTDIR_REG_ROOT} "${INSTDIR_REG_KEY}"
RMDIR "$INSTDIR"
SectionEnd
