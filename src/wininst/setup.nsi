# Copyright (C) 2010, 2011, 2012  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

!ifndef NVDA
!include "LogicLib.nsh"
!include "Library.nsh"
!endif

!ifndef VERSION
!define VERSION "0.4-a3"
!endif

!ifndef NVDA
!define INSTDIR_REG_ROOT "HKLM"
!define INSTDIR_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\RHVoice"
!include AdvUninstLog.nsh
!define PROGRAM_REG_KEY "Software\RHVoice"
!define SPTOKENENUMS_REG_KEY "SOFTWARE\Microsoft\Speech\Voices\TokenEnums"
!define CLSID "{d7577808-7ade-4dea-a5b7-ee314d6ef3a1}"
!endif

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

!ifdef NVDA
InstallDir "$APPDATA\nvda\synthDrivers\"
Name "RHVoice v${VERSION} for NVDA"
OutFile "..\..\build\win32\RHVoice-v${VERSION}-synthDriver-setup.exe"
RequestExecutionLevel user
!else
InstallDir "$PROGRAMFILES\RHVoice"
InstallDirRegKey ${INSTDIR_REG_ROOT} "${INSTDIR_REG_KEY}" "InstallDir"
Name "RHVoice v${VERSION}"
OutFile "..\..\build\win32\RHVoice-v${VERSION}-setup.exe"
RequestExecutionLevel admin
!insertmacro UNATTENDED_UNINSTALL
!endif

PageEx license
Caption $(readmeCaption)
LicenseText $(readmeText) $(^NextBtn)
LicenseData $(readmeFile)
PageExEnd
Page directory
Page instfiles
!ifndef NVDA
UninstPage uninstConfirm
UninstPage instfiles
!endif

Section
!ifdef NVDA
SetOutPath $INSTDIR
File  ..\..\build\win32\lib\RHVoice.dll
File  ..\nvda-synthDriver\RHVoice.py
!else
SetOutPath "$INSTDIR\lib"
!insertmacro UNINSTALL.LOG_OPEN_INSTALL
!insertmacro installLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED ..\..\build\win32\sapi\RHVoiceSvr.dll "$INSTDIR\lib\RHVoiceSvr.dll" $INSTDIR
!insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
!ifdef NVDA
SetOutPath "$INSTDIR\RHVoice-data"
!else
SetOutPath "$INSTDIR\data"
!insertmacro UNINSTALL.LOG_OPEN_INSTALL
!endif
File /r /x .git* /x SConscript ..\..\data\*.*
!ifdef NVDA
SetOutPath "$INSTDIR\RHVoice-doc"
!else
!insertmacro UNINSTALL.LOG_CLOSE_INSTALL
SetOutPath "$INSTDIR\doc"
!insertmacro UNINSTALL.LOG_OPEN_INSTALL
!endif
File ..\..\build\win32\COPYING.txt
!ifdef NVDA
SetOverwrite off
SetOutPath "$INSTDIR\RHVoice-config"
!else
!insertmacro UNINSTALL.LOG_CLOSE_INSTALL
SetOutPath "$INSTDIR\config-examples"
!insertmacro UNINSTALL.LOG_OPEN_INSTALL
!endif
File ..\..\build\win32\RHVoice.ini
!ifndef NVDA
!insertmacro UNINSTALL.LOG_CLOSE_INSTALL
WriteRegStr ${INSTDIR_REG_ROOT} "${SPTOKENENUMS_REG_KEY}\RHVoice" "CLSID" ${CLSID}
WriteRegStr ${INSTDIR_REG_ROOT} ${PROGRAM_REG_KEY} "data_path" "$INSTDIR\data"
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "INSTDIR" $INSTDIR
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "DisplayName" "RHVoice"
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "UninstallString" "${UNINST_EXE}"
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "Publisher" "Olga Yakovleva"
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "InstallLocation" $INSTDIR
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "DisplayVersion" ${VERSION}
WriteRegStr ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "URLUpdateInfo" "http://github.com/Olga-Yakovleva/RHVoice/downloads"
WriteRegDWORD ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "NoModify" 1
WriteRegDWORD ${INSTDIR_REG_ROOT} ${INSTDIR_REG_KEY} "NoRepair" 1
!endif
SectionEnd

!ifndef NVDA
Function .onInit
!insertmacro UNINSTALL.LOG_PREPARE_INSTALL
FunctionEnd

Function .onInstSuccess
!insertmacro UNINSTALL.LOG_UPDATE_INSTALL
FunctionEnd

Section UnInstall
DeleteRegKey ${INSTDIR_REG_ROOT} "${SPTOKENENUMS_REG_KEY}\RHVoice"
!insertmacro UnInstallLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED "$INSTDIR\lib\RHVoiceSvr.dll"
DeleteRegKey ${INSTDIR_REG_ROOT} ${PROGRAM_REG_KEY}
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
!endif
