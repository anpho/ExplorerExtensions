; Windows Explorer Extension Installer
; NSIS Script

; Set compression
SetCompressor /SOLID lzma

; Set output file
OutFile "ExplorerExtensionSetup.exe"

; Set installation icon
; Icon "icon.ico"

; Set request execution level
RequestExecutionLevel user

; Define version information
!define VERSION "26.1.13"
!define PRODUCT_NAME "anpho's Extensions"
!define COPYRIGHT "x.com/anpho"

VIProductVersion "${VERSION}.0"
VIFileVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" "anpho"
VIAddVersionKey "LegalCopyright" "${COPYRIGHT}"
VIAddVersionKey "FileDescription" "Windows Explorer Context Menu Extension"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "FileVersion" "${VERSION}.0"

; Set installation directory
InstallDir $LOCALAPPDATA\ExplorerEnhancement

; Set installation pages
Page Directory
Page InstFiles

; Set uninstallation pages
UninstPage UninstConfirm
UninstPage InstFiles

; Define registry constants for Add/Remove Programs
!define UNINSTALL_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\anpho's Extensions"

; Main installation section
Section "Main Section" SEC01
    ; Set output path
    SetOutPath $INSTDIR

    ; Install 64-bit version by default
    IfFileExists "$INSTDIR\ExplorerEnhancement_x64.dll" 0 skip_unreg_x64
        ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x64.dll'
    skip_unreg_x64:
    File "src\build\ExplorerEnhancement_x64.dll"
    File "src\build\ExplorerEnhancementManager_x64.exe"

    ; Register DLL
    ExecWait '$SYSDIR\regsvr32.exe /s $INSTDIR\ExplorerEnhancement_x64.dll'

    ; Create start menu shortcuts
    CreateDirectory "$SMPROGRAMS\anpho's Extensions"
    CreateShortcut "$SMPROGRAMS\anpho's Extensions\anpho's Extensions Manager.lnk" "$INSTDIR\ExplorerEnhancementManager_x64.exe"
    CreateShortcut "$SMPROGRAMS\anpho's Extensions\Uninstall.lnk" "$INSTDIR\uninstall.exe"

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Set Add/Remove Programs information
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "DisplayIcon" "$INSTDIR\ExplorerEnhancementManager_x64.exe,0"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "Publisher" "anpho"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "InstallDate" "$(^YYYY)-$(^MM)-$(^DD)"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "QuietUninstallString" "$INSTDIR\uninstall.exe /S"
    WriteRegDWORD HKCU "${UNINSTALL_REG_KEY}" "NoModify" 1
    WriteRegDWORD HKCU "${UNINSTALL_REG_KEY}" "NoRepair" 1
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "URLInfoAbout" "https://x.com/anpho"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "HelpLink" "mailto:support@anpho.cn"
SectionEnd

; Uninstallation section
Section "Uninstall"
    ; Unregister DLL
    ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x64.dll'
    Delete "$INSTDIR\ExplorerEnhancement_x64.dll"
    Delete "$INSTDIR\ExplorerEnhancementManager_x64.exe"

    ; Delete shortcuts
    Delete "$SMPROGRAMS\anpho's Extensions\anpho's Extensions Manager.lnk"
    Delete "$SMPROGRAMS\anpho's Extensions\Uninstall.lnk"
    RMDir "$SMPROGRAMS\anpho's Extensions"

    ; Delete uninstaller
    Delete "$INSTDIR\uninstall.exe"

    ; Delete Add/Remove Programs information
    DeleteRegKey HKCU "${UNINSTALL_REG_KEY}"

    ; Delete installation directory
    RMDir "$INSTDIR"

    ; Prompt user to restart Windows Explorer
    MessageBox MB_ICONINFORMATION "Windows Explorer Extension has been successfully uninstalled.\r\nPlease restart Windows Explorer or your computer for the changes to take effect."
SectionEnd

; Function executed after successful installation
Function .onInstSuccess
    ; Start ExplorerEnhancementManager program
    ExecShell "open" "$INSTDIR\ExplorerEnhancementManager_x64.exe"
FunctionEnd