!define PRODUCT RHVoice
!ifndef VERSION
!define VERSION "0.3"
!endif
!define UNINSTALL_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
!define SPTOKENS_REG_KEY "SOFTWARE\Microsoft\Speech\Voices\Tokens"
!define CLSID "{9F215C97-3D3B-489D-8419-6B9ABBF31EC2}"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

SetCompressor /solid lzma
SetOverwrite on
AllowSkipFiles off

AutoCloseWindow false
CRCCheck on
InstallDir "$PROGRAMFILES\${PRODUCT}"
InstallDirRegKey HKLM UNINSTALL_REG_KEY "INSTDIR"
Name "${PRODUCT} v${VERSION}"
OutFile "..\..\build\win32\${PRODUCT}-v${VERSION}-setup.exe"
RequestExecutionLevel admin
ShowInstDetails show
ShowUninstDetails show

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

!macro RegisterVoice token_name voice_name voice_path voice_variant
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\${token_name}" "" "${voice_name}"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\${token_name}" "CLSID" ${CLSID}
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\${token_name}" "VoicePath" "$INSTDIR\data\${voice_path}"
  WriteRegDWORD HKLM "${SPTOKENS_REG_KEY}\${token_name}" "Variant" ${voice_variant}
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\${token_name}\Attributes" "Age" "Adult"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\${token_name}\Attributes" "Gender" "Male"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\${token_name}\Attributes" "Language" "419"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\${token_name}\Attributes" "Name" "${voice_name}"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\${token_name}\Attributes" "Vendor" "Olga Yakovleva"
!macroend

Section
  SetOutPath "$INSTDIR\data\RHVoice-M1"
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
  SetOutPath "$INSTDIR\documentation"
  File ..\..\build\win32\README.txt
  File ..\..\build\win32\COPYING.txt
  File /oname=hts_engine_API-COPYING.txt ..\..\build\win32\hts_engine_api\COPYING.txt
  SetOutPath "$INSTDIR\lib"
  File ..\..\build\win32\lib\RHVoice.dll
  File ..\..\build\win32\sapi\RHVoiceSvr.dll
  try_RegDLL:
  ClearErrors
  RegDLL "$INSTDIR\lib\RHVoiceSvr.dll"
  IfErrors 0 RegDLL_success
  MessageBox MB_RETRYCANCEL|MB_ICONSTOP $(^ErrorRegistering) /SD IDCANCEL IDRETRY try_RegDLL IDCANCEL 0
  Abort
  RegDLL_success:
  # Register the voices with SAPI
!insertmacro RegisterVoice "RHVoice-M1-V1" "RHVoice-M1 (Pseudo-English Variant)" "RHVoice-M1" 1
!insertmacro RegisterVoice "RHVoice-M1-V2" "RHVoice-M1 (Russian Variant)" "RHVoice-M1" 2
  # Uninstallation information
  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM ${UNINSTALL_REG_KEY} "INSTDIR" $INSTDIR
  WriteRegStr HKLM ${UNINSTALL_REG_KEY} "DisplayName" ${PRODUCT}
  WriteRegStr HKLM ${UNINSTALL_REG_KEY} "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM ${UNINSTALL_REG_KEY} "Publisher" "Olga Yakovleva"
  WriteRegStr HKLM ${UNINSTALL_REG_KEY} "InstallLocation" $INSTDIR
  WriteRegStr HKLM ${UNINSTALL_REG_KEY} "DisplayVersion" ${VERSION}
  WriteRegStr HKLM ${UNINSTALL_REG_KEY} "URLUpdateInfo" "http://github.com/Olga-Yakovleva/RHVoice/downloads"
  WriteRegDWORD HKLM ${UNINSTALL_REG_KEY} "NoModify" 1
  WriteRegDWORD HKLM ${UNINSTALL_REG_KEY} "NoRepair" 1
SectionEnd

Section Uninstall
  DeleteRegKey HKLM "${SPTOKENS_REG_KEY}\RHVoice-M1-V1"
  DeleteRegKey HKLM "${SPTOKENS_REG_KEY}\RHVoice-M1-V2"
  UnRegDLL "$INSTDIR\lib\RHVoiceSvr.dll"
  Delete "$INSTDIR\lib\RHVoiceSvr.dll"
  Delete "$INSTDIR\lib\RHVoice.dll"
  RMDIR "$INSTDIR\lib"
  Delete "$INSTDIR\documentation\README.txt"
  Delete "$INSTDIR\documentation\COPYING.txt"
  Delete "$INSTDIR\documentation\hts_engine_API-COPYING.txt"
  RMDIR "$INSTDIR\documentation"
  Delete "$INSTDIR\data\RHVoice-M1\dur.pdf"
  Delete "$INSTDIR\data\RHVoice-M1\lf0.pdf"
  Delete "$INSTDIR\data\RHVoice-M1\lf0.win1"
  Delete "$INSTDIR\data\RHVoice-M1\lf0.win2"
  Delete "$INSTDIR\data\RHVoice-M1\lf0.win3"
  Delete "$INSTDIR\data\RHVoice-M1\mgc.pdf"
  Delete "$INSTDIR\data\RHVoice-M1\mgc.win1"
  Delete "$INSTDIR\data\RHVoice-M1\mgc.win2"
  Delete "$INSTDIR\data\RHVoice-M1\mgc.win3"
  Delete "$INSTDIR\data\RHVoice-M1\tree-dur.inf"
  Delete "$INSTDIR\data\RHVoice-M1\tree-lf0.inf"
  Delete "$INSTDIR\data\RHVoice-M1\tree-mgc.inf"
  RMDir "$INSTDIR\data\RHVoice-M1"
  RMDir "$INSTDIR\data"
  Delete "$INSTDIR\uninstall.exe"
  RMDIR "$INSTDIR"
  DeleteRegKey HKLM ${UNINSTALL_REG_KEY}
SectionEnd
