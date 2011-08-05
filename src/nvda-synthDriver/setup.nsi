!ifndef VERSION
!define VERSION "0.3"
!endif

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

SetCompressor /solid lzma
SetOverwrite on
AllowSkipFiles off

AutoCloseWindow false
CRCCheck on
InstallDir "$APPDATA\nvda\synthDrivers\"
Name "RHVoice v${VERSION} for NVDA"
OutFile "..\..\build\win32\RHVoice-v${VERSION}-synthDriver-setup.exe"
RequestExecutionLevel user
ShowInstDetails show
ShowUninstDetails show

Page directory
Page instfiles

!macro InstallVoice name
  SetOutPath "$INSTDIR\RHVoice-data\${name}"
  File ..\..\data\${name}\dur.pdf
  File ..\..\data\${name}\lf0.pdf
  File ..\..\data\${name}\lf0.win1
  File ..\..\data\${name}\lf0.win2
  File ..\..\data\${name}\lf0.win3
  File ..\..\data\${name}\lpf.pdf
  File ..\..\data\${name}\lpf.win1
  File ..\..\data\${name}\mgc.pdf
  File ..\..\data\${name}\mgc.win1
  File ..\..\data\${name}\mgc.win2
  File ..\..\data\${name}\mgc.win3
  File ..\..\data\${name}\tree-dur.inf
  File ..\..\data\${name}\tree-lf0.inf
  File ..\..\data\${name}\tree-lpf.inf
  File ..\..\data\${name}\tree-mgc.inf
!macroend

Section
  !insertmacro InstallVoice "Aleksandr"
  !insertmacro InstallVoice "Elena"
  SetOutPath $INSTDIR
  File ..\..\build\win32\lib\RHVoice.dll
  File RHVoice.py
  SetOutPath "$INSTDIR\RHVoice-doc"
  File /oname=readme.txt ..\..\build\win32\README.txt
  File /oname=license.txt ..\..\build\win32\COPYING.txt
  File /oname=HTS_Engine_license.txt ..\..\build\win32\hts_engine_api\COPYING.txt
SectionEnd
