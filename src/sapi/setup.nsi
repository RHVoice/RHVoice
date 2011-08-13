!define PRODUCT RHVoice
!ifndef VERSION
!define VERSION "0.3"
!endif
!define UNINSTALL_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
!define PROGRAM_REG_KEY "Software\RHVoice"
!define SPTOKENS_REG_KEY "SOFTWARE\Microsoft\Speech\Voices\Tokens"
!define CLSID "{9F215C97-3D3B-489D-8419-6B9ABBF31EC2}"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"

LicenseLangString readmeFile ${LANG_ENGLISH} ..\nvda-synthDriver\setup-readme-en.txt
LicenseLangString readmeFile ${LANG_RUSSIAN} ..\nvda-synthDriver\setup-readme-ru.txt
LangString readmeCaption ${LANG_ENGLISH} ": Readme"
LangString readmeCaption ${LANG_RUSSIAN} ": О программе"
LangString readmeText ${LANG_ENGLISH} "General information about RHVoice"
LangString readmeText ${LANG_RUSSIAN} "Общие сведения об RHVoice"

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

PageEx license
Caption $(readmeCaption)
LicenseText $(readmeText) $(^NextBtn)
LicenseData $(readmeFile)
PageExEnd
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

!macro InstallVoice name
  SetOutPath "$INSTDIR\data\${name}"
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

!macro UninstallVoice name
  DeleteRegKey HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-Pseudo-English"
  DeleteRegKey HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-Russian"
  Delete "$INSTDIR\data\${name}\dur.pdf"
  Delete "$INSTDIR\data\${name}\lf0.pdf"
  Delete "$INSTDIR\data\${name}\lf0.win1"
  Delete "$INSTDIR\data\${name}\lf0.win2"
  Delete "$INSTDIR\data\${name}\lf0.win3"
  Delete "$INSTDIR\data\${name}\lpf.pdf"
  Delete "$INSTDIR\data\${name}\lpf.win1"
  Delete "$INSTDIR\data\${name}\mgc.pdf"
  Delete "$INSTDIR\data\${name}\mgc.win1"
  Delete "$INSTDIR\data\${name}\mgc.win2"
  Delete "$INSTDIR\data\${name}\mgc.win3"
  Delete "$INSTDIR\data\${name}\tree-dur.inf"
  Delete "$INSTDIR\data\${name}\tree-lf0.inf"
  Delete "$INSTDIR\data\${name}\tree-lpf.inf"
  Delete "$INSTDIR\data\${name}\tree-mgc.inf"
  RMDir "$INSTDIR\data\${name}"
!macroend

!macro RegisterVoice name variant gender
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}" "" "RHVoice ${name} (${variant})"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}" "CLSID" ${CLSID}
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}" "voice" ${name}
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}" "variant" ${variant}
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}\Attributes" "Age" "Adult"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}\Attributes" "Gender" "${gender}"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}\Attributes" "Language" "419"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}\Attributes" "Name" "RHVoice ${name} (${variant})"
  WriteRegStr HKLM "${SPTOKENS_REG_KEY}\RHVoice-${name}-${variant}\Attributes" "Vendor" "Olga Yakovleva"
!macroend

Section
  !insertmacro InstallVoice "Aleksandr"
  !insertmacro InstallVoice "Elena"
  WriteRegStr HKLM ${PROGRAM_REG_KEY} "data_path" "$INSTDIR\data"
  SetOutPath "$INSTDIR\documentation"
  File /oname=readme.txt ..\..\build\win32\README.txt
  File /oname=license.txt ..\..\build\win32\COPYING.txt
  File /oname=HTS_Engine_license.txt ..\..\build\win32\hts_engine_api\COPYING.txt
  SetOutPath "$INSTDIR\config-examples"
  File ..\..\build\win32\RHVoice.ini
  SetOutPath "$INSTDIR\config-examples\dicts"
  File /oname=example.txt ..\..\build\win32\dict.txt
  SetOutPath "$INSTDIR\config-examples\variants"
  File ..\..\build\win32\Pseudo-Esperanto.txt
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
  !insertmacro RegisterVoice "Aleksandr" "Pseudo-English" "Male"
  !insertmacro RegisterVoice "Aleksandr" "Russian" "Male"
  !insertmacro RegisterVoice "Elena" "Pseudo-English" "Female"
  !insertmacro RegisterVoice "Elena" "Russian" "Female"
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
  UnRegDLL "$INSTDIR\lib\RHVoiceSvr.dll"
  Delete "$INSTDIR\lib\RHVoiceSvr.dll"
  Delete "$INSTDIR\lib\RHVoice.dll"
  RMDIR "$INSTDIR\lib"
  Delete "$INSTDIR\documentation\readme.txt"
  Delete "$INSTDIR\documentation\license.txt"
  Delete "$INSTDIR\documentation\HTS_Engine_license.txt"
  RMDIR "$INSTDIR\documentation"
  Delete "$INSTDIR\config-examples\RHVoice.ini"
  Delete "$INSTDIR\config-examples\dicts\example.txt"
  RMDIR "$INSTDIR\config-examples\dicts"
  Delete "$INSTDIR\config-examples\variants\Pseudo-Esperanto.txt"
  RMDIR "$INSTDIR\config-examples\variants"
  RMDIR "$INSTDIR\config-examples"
  !insertmacro UninstallVoice "Elena"
  !insertmacro UninstallVoice "Aleksandr"
  RMDir "$INSTDIR\data"
  Delete "$INSTDIR\uninstall.exe"
  RMDIR "$INSTDIR"
  DeleteRegKey HKLM ${PROGRAM_REG_KEY}
  DeleteRegKey HKLM ${UNINSTALL_REG_KEY}
SectionEnd
