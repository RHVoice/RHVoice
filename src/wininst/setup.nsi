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

!ifndef VERSION
!define VERSION "0.4-a1"
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
RequestExecutionLevel user
ShowInstDetails show
ShowUninstDetails show

InstallDir "$APPDATA\nvda\synthDrivers\"
Name "RHVoice v${VERSION} for NVDA"
OutFile "..\..\build\win32\RHVoice-v${VERSION}-synthDriver-setup.exe"

PageEx license
Caption $(readmeCaption)
LicenseText $(readmeText) $(^NextBtn)
LicenseData $(readmeFile)
PageExEnd
Page directory
Page instfiles

Section
  SetOutPath $INSTDIR
  File ..\..\build\win32\lib\RHVoice.dll
  File ..\nvda-synthDriver\RHVoice.py
  SetOutPath "$INSTDIR\RHVoice-data"
  File /r /x .git* /x SConscript ..\..\data\*.*
  SetOutPath "$INSTDIR\RHVoice-doc"
  File ..\..\build\win32\COPYING.txt
  SetOverwrite off
  SetOutPath "$INSTDIR\RHVoice-config"
  File ..\..\build\win32\RHVoice.ini
SectionEnd
