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

!include "${BUILDDIR}\data.nsh"

!define UNINSTALL_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\RHVoice"
!define PROGRAM_REG_KEY "Software\RHVoice"
!define SPTOKENENUMS_REG_KEY "SOFTWARE\Microsoft\Speech\Voices\TokenEnums"
!define CLSID "{d7577808-7ade-4dea-a5b7-ee314d6ef3a1}"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

LicenseLangString readmeFile ${LANG_ENGLISH} "${BUILDDIR}\en.txt"
LicenseLangString readmeFile ${LANG_RUSSIAN} "${BUILDDIR}\ru.txt"
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
InstallDirRegKey HKLM "${UNINSTALL_REG_KEY}" "InstallDir"
Name "RHVoice v${VERSION}"
RequestExecutionLevel admin

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
!insertmacro installLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED build\windows\x86\sapi\RHVoiceSvr.dll "RHVoiceSvr32.dll" $INSTDIR
!ifdef WIN64
!define LIBRARY_X64
${If} ${RunningX64}
!insertmacro installLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED build\windows\x64\sapi\RHVoiceSvr.dll "RHVoiceSvr64.dll" $INSTDIR
${EndIf}
!undef LIBRARY_X64      
!endif

!insertmacro InstallData

SetOutPath "$INSTDIR\doc"
File build\windows\COPYING.txt

SetOutPath "$INSTDIR\config-examples"
File build\windows\RHVoice.ini

WriteRegStr HKLM "${SPTOKENENUMS_REG_KEY}\RHVoice" "CLSID" ${CLSID}
WriteRegStr HKLM ${PROGRAM_REG_KEY} "data_path" "$INSTDIR\data"
!ifdef WIN64
${If} ${RunningX64}
SetRegView 64
WriteRegStr HKLM "${SPTOKENENUMS_REG_KEY}\RHVoice" "CLSID" ${CLSID}
WriteRegStr HKLM ${PROGRAM_REG_KEY} "data_path" "$INSTDIR\data"
SetRegView 32
${EndIf}
!endif

  WriteUninstaller "$INSTDIR\uninstall.exe"
WriteRegStr HKLM ${UNINSTALL_REG_KEY} "INSTDIR" $INSTDIR
WriteRegStr HKLM ${UNINSTALL_REG_KEY} "DisplayName" "RHVoice"
WriteRegStr HKLM ${UNINSTALL_REG_KEY} "UninstallString" '"$INSTDIR\uninstall.exe"'
WriteRegStr HKLM ${UNINSTALL_REG_KEY} "Publisher" "Olga Yakovleva"
WriteRegStr HKLM ${UNINSTALL_REG_KEY} "InstallLocation" $INSTDIR
WriteRegStr HKLM ${UNINSTALL_REG_KEY} "DisplayVersion" ${VERSION}
WriteRegStr HKLM ${UNINSTALL_REG_KEY} "URLUpdateInfo" "http://github.com/Olga-Yakovleva/RHVoice/downloads"
WriteRegDWORD HKLM ${UNINSTALL_REG_KEY} "NoModify" 1
WriteRegDWORD HKLM ${UNINSTALL_REG_KEY} "NoRepair" 1
SectionEnd

Section UnInstall
!ifdef WIN64
${If} ${RunningX64}
SetRegView 64
DeleteRegKey HKLM "${SPTOKENENUMS_REG_KEY}\RHVoice"
DeleteRegKey HKLM ${PROGRAM_REG_KEY}
SetRegView 32
!define LIBRARY_X64
!insertmacro UnInstallLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED "$INSTDIR\lib\RHVoiceSvr64.dll"
!undef LIBRARY_X64      
${EndIf}
!endif
deleteRegKey HKLM "${SPTOKENENUMS_REG_KEY}\RHVoice"
DeleteRegKey HKLM ${PROGRAM_REG_KEY}
!insertmacro UnInstallLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED "$INSTDIR\lib\RHVoiceSvr32.dll"
Rmdir "$INSTDIR\lib"
!insertmacro UninstallData
Delete "$INSTDIR\doc\COPYING.txt"
Rmdir "$INSTDIR\doc"
Delete "$INSTDIR\config-examples\RHVoice.ini"
Rmdir "$INSTDIR\config-examples"
DeleteRegKey HKLM "${UNINSTALL_REG_KEY}"
Delete "$INSTDIR\uninstall.exe"
RMDIR "$INSTDIR"
SectionEnd
