!define VERSION "0.2"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

SetCompressor /solid lzma
SetOverwrite on
AllowSkipFiles off

AutoCloseWindow false
CRCCheck on
InstallDir "$APPDATA\nvda\synthDrivers\"
Name "RHVoice v${VERSION} for NVDA"
OutFile "../../build/RHVoice-v${VERSION}-synthDriver-setup.exe"
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
  File ..\..\data\voice\mgc.pdf
  File ..\..\data\voice\mgc.win1
  File ..\..\data\voice\mgc.win2
  File ..\..\data\voice\mgc.win3
  File ..\..\data\voice\tree-dur.inf
  File ..\..\data\voice\tree-lf0.inf
  File ..\..\data\voice\tree-mgc.inf
  SetOutPath $INSTDIR
  File ..\..\build\lib\RHVoice.dll
  File RHVoice.py
  File /oname=RHVoice-README.txt ..\..\README
  File /oname=RHVoice-COPYING.txt ..\..\COPYING
  File /oname=hts_engine_API-COPYING.txt ..\hts_engine_api\COPYING
SectionEnd
