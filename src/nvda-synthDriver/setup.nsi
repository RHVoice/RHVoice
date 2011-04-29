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

Section
  SetOutPath "$INSTDIR\RHVoice-data\voice"
  File ..\..\data\voice\dur.pdf
  File ..\..\data\voice\lf0.pdf
  File ..\..\data\voice\lf0.win1
  File ..\..\data\voice\lf0.win2
  File ..\..\data\voice\lf0.win3
  File ..\..\data\voice\lpf.pdf
  File ..\..\data\voice\lpf.win1
  File ..\..\data\voice\mgc.pdf
  File ..\..\data\voice\mgc.win1
  File ..\..\data\voice\mgc.win2
  File ..\..\data\voice\mgc.win3
  File ..\..\data\voice\tree-dur.inf
  File ..\..\data\voice\tree-lf0.inf
  File ..\..\data\voice\tree-lpf.inf
  File ..\..\data\voice\tree-mgc.inf
  SetOutPath $INSTDIR
  File ..\..\build\win32\lib\RHVoice.dll
  File RHVoice.py
  File /oname=RHVoice-README.txt ..\..\build\win32\README.txt
  File /oname=RHVoice-COPYING.txt ..\..\build\win32\COPYING.txt
  File /oname=hts_engine_API-COPYING.txt ..\..\build\win32\hts_engine_api\COPYING.txt
SectionEnd
