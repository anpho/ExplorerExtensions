; Windows Explorer Extension Installer
; NSIS Script

SetCompressor /SOLID lzma

OutFile "ExplorerExtensionSetup.exe"

RequestExecutionLevel user

!define VERSION "26.1.15"
!define PRODUCT_NAME "anpho's Extensions"
!define COPYRIGHT "x.com/anpho"

VIProductVersion "${VERSION}.0"
VIFileVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" "anpho"
VIAddVersionKey "LegalCopyright" "${COPYRIGHT}"
VIAddVersionKey "FileDescription" "Windows Explorer Context Menu Extension"
VIAddVersionKey "ProductVersion" "${VERSION}"

InstallDir $LOCALAPPDATA\ExplorerEnhancement

Page Directory
Page InstFiles

UninstPage UninstConfirm
UninstPage InstFiles

; Define registry constants for Add/Remove Programs
!define UNINSTALL_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\anpho's Extensions"

Section "Main Section" SEC01

    SetOutPath $INSTDIR
    StrCmp $PROCESSOR_ARCHITECTURE "x86" arch_x86 arch_x64

    arch_x86:
        IfFileExists "$INSTDIR\ExplorerEnhancement_x86.dll" 0 skip_unreg_x86
            ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x86.dll'
        skip_unreg_x86:
        File "src\build\ExplorerEnhancement_x86.dll"
        File "src\build\ExplorerEnhancementManager_x86.exe"
        Goto arch_end

    arch_x64:
        IfFileExists "$INSTDIR\ExplorerEnhancement_x64.dll" 0 skip_unreg_x64
            ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x64.dll'
        skip_unreg_x64:
        File "src\build\ExplorerEnhancement_x64.dll"
        File "src\build\ExplorerEnhancementManager_x64.exe"

    arch_end:

    StrCmp $PROCESSOR_ARCHITECTURE "x86" reg_x86 reg_x64

    reg_x86:
        ExecWait '$SYSDIR\regsvr32.exe /s $INSTDIR\ExplorerEnhancement_x86.dll'
        Goto reg_end

    reg_x64:
        ExecWait '$SYSDIR\regsvr32.exe /s $INSTDIR\ExplorerEnhancement_x64.dll'

    reg_end:


    CreateDirectory "$SMPROGRAMS\anpho's Extensions"
    StrCmp $PROCESSOR_ARCHITECTURE "x86" shortcut_x86 shortcut_x64

    shortcut_x86:
        CreateShortcut "$SMPROGRAMS\anpho's Extensions\anpho's Extensions Manager.lnk" "$INSTDIR\ExplorerEnhancementManager_x86.exe"
        Goto shortcut_end

    shortcut_x64:
        CreateShortcut "$SMPROGRAMS\anpho's Extensions\anpho's Extensions Manager.lnk" "$INSTDIR\ExplorerEnhancementManager_x64.exe"

    shortcut_end:
    CreateShortcut "$SMPROGRAMS\anpho's Extensions\Uninstall.lnk" "$INSTDIR\uninstall.exe"

    WriteUninstaller "$INSTDIR\uninstall.exe"

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

Section "Uninstall"
    StrCmp $PROCESSOR_ARCHITECTURE "x86" unreg_x86 unreg_x64

    unreg_x86:
        ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x86.dll'
        Delete "$INSTDIR\ExplorerEnhancement_x86.dll"
        Delete "$INSTDIR\ExplorerEnhancementManager_x86.exe"
        Goto unreg_end

    unreg_x64:
        ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x64.dll'
        Delete "$INSTDIR\ExplorerEnhancement_x64.dll"
        Delete "$INSTDIR\ExplorerEnhancementManager_x64.exe"

    unreg_end:

    Delete "$SMPROGRAMS\anpho's Extensions\anpho's Extensions Manager.lnk"
    Delete "$SMPROGRAMS\anpho's Extensions\Uninstall.lnk"
    RMDir "$SMPROGRAMS\anpho's Extensions"

    Delete "$INSTDIR\uninstall.exe"

    DeleteRegKey HKCU "${UNINSTALL_REG_KEY}"

    RMDir "$INSTDIR"

    MessageBox MB_ICONINFORMATION "Windows Explorer Extension has been successfully uninstalled.Please restart Windows Explorer or your computer for the changes to take effect."
SectionEnd

Function .onInstSuccess

    StrCmp $PROCESSOR_ARCHITECTURE "x86" start_x86 start_x64
    
    start_x86:
        ExecShell "open" "$INSTDIR\ExplorerEnhancementManager_x86.exe"
        Goto start_end
    
    start_x64:
        ExecShell "open" "$INSTDIR\ExplorerEnhancementManager_x64.exe"
    
    start_end:
FunctionEnd
