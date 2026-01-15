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

; Function to restart Windows Explorer for installation
Function RestartExplorer
    ; Terminate Explorer
    System::Call 'shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
    ExecWait 'taskkill /f /im explorer.exe'
    ; Wait a moment
    Sleep 1000
    ; Restart Explorer
    ExecShell "open" "explorer.exe"
    ; Wait for Explorer to fully start
    Sleep 1000
FunctionEnd

; Function to restart Windows Explorer for uninstallation
Function un.RestartExplorer
    ; Terminate Explorer
    System::Call 'shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
    ExecWait 'taskkill /f /im explorer.exe'
    ; Wait a moment
    Sleep 1000
    ; Restart Explorer
    ExecShell "open" "explorer.exe"
    ; Wait for Explorer to fully start
    Sleep 1000
FunctionEnd

; Main installation section
Section "Main Section" SEC01
    ; Set output path
    SetOutPath $INSTDIR

    ; Install 64-bit version by default
    IfFileExists "$INSTDIR\ExplorerEnhancement_x64.dll" 0 skip_unreg_x64
        ; Try to unregister and delete old DLL
        ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x64.dll'
        Delete "$INSTDIR\ExplorerEnhancement_x64.dll"
        ; Check if DLL still exists
        IfFileExists "$INSTDIR\ExplorerEnhancement_x64.dll" 0 skip_restart_install
            ; Prompt user to restart Explorer
            MessageBox MB_ICONEXCLAMATION "The old DLL file cannot be deleted. Windows Explorer needs to be restarted to complete the installation.\r\nClick OK to restart Explorer and continue."
            Call RestartExplorer
            ; Try again to delete old DLL
            Delete "$INSTDIR\ExplorerEnhancement_x64.dll"
        skip_restart_install:
    skip_unreg_x64:
    
    ; Try to install new DLL
    File "src\build\ExplorerEnhancement_x64.dll"
    ; Check if DLL was installed successfully
    IfFileExists "$INSTDIR\ExplorerEnhancement_x64.dll" 0 dll_install_failed
        Goto dll_install_success
    dll_install_failed:
        ; Prompt user to restart Explorer
        MessageBox MB_ICONEXCLAMATION "The new DLL file cannot be installed. Windows Explorer needs to be restarted to complete the installation.\r\nClick OK to restart Explorer and continue."
        Call RestartExplorer
        ; Try again to install new DLL
        File "src\build\ExplorerEnhancement_x64.dll"
    dll_install_success:
    
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
    ; Try to delete DLL
    Delete "$INSTDIR\ExplorerEnhancement_x64.dll"
    ; Check if DLL still exists
    IfFileExists "$INSTDIR\ExplorerEnhancement_x64.dll" 0 dll_deleted
        ; Prompt user to restart Explorer
        MessageBox MB_ICONEXCLAMATION "The DLL file cannot be deleted. Windows Explorer needs to be restarted to complete the uninstallation.\r\nClick OK to restart Explorer and continue."
        Call un.RestartExplorer
        ; Try again to delete DLL
        Delete "$INSTDIR\ExplorerEnhancement_x64.dll"
    dll_deleted:
    
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