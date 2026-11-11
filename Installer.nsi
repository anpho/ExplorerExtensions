; Windows Explorer Extension - per-user installer (NSIS 3.x)
;
; Build from the repo root:
;   makensis /DVERSION=1.2.3.0 Installer.nsi
; VERSION must be a 4-part version (it is also the version written to version.h
; by the release workflow). It defaults to 1.0.0.0 for local builds.
;
; Keep this file ASCII-only: makensis reads ANSI scripts, so non-ASCII comments
; can eat the following line on non-Latin code pages.

Unicode true
SetCompressor /SOLID lzma
; Do not offer "Ignore" when a file cannot be written: an old DLL left in place
; must not end up reported as a successful install.
AllowSkipFiles off

OutFile "ExplorerExtensionSetup.exe"

; Per-user install: %LOCALAPPDATA% + HKCU only, no elevation.
RequestExecutionLevel user

!include "LogicLib.nsh"
!include "x64.nsh"

!ifndef VERSION
  !define VERSION "1.0.0.0"
!endif

!define PRODUCT_NAME "anpho's Extensions"
!define COPYRIGHT "https://x.com/anpho"
!define UNINSTALL_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\anpho's Extensions"
!define BLOCKED_KEY "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Blocked"
!define CLSID_NEWFOLDER "{c36a65af-0a1e-4e09-9729-6da3e4e4461a}"
!define CLSID_BREAKFOLDER "{f2a0aaf9-2961-4465-a843-884383fcd13b}"

VIProductVersion "${VERSION}"
VIFileVersion "${VERSION}"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" "anpho"
VIAddVersionKey "LegalCopyright" "${COPYRIGHT}"
VIAddVersionKey "FileDescription" "Windows Explorer context menu extension installer"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductVersion" "${VERSION}"

Name "${PRODUCT_NAME}"
InstallDir "$LOCALAPPDATA\ExplorerEnhancement"
InstallDirRegKey HKCU "${UNINSTALL_REG_KEY}" "InstallLocation"

Page Directory
Page InstFiles

UninstPage UninstConfirm
UninstPage InstFiles

Var DllName
Var ExeName
Var RestartedExplorer

; ---------------------------------------------------------------- install
Section "Install" SEC01

    ; Arch-specific file names. Both architectures are embedded; the matching
    ; one is extracted at run time.
    ${If} ${RunningX64}
        StrCpy $DllName "ExplorerEnhancement_x64.dll"
        StrCpy $ExeName "ExplorerEnhancementManager_x64.exe"
    ${Else}
        StrCpy $DllName "ExplorerEnhancement_x86.dll"
        StrCpy $ExeName "ExplorerEnhancementManager_x86.exe"
    ${EndIf}

    ; Stop a running manager so the exe can be replaced.
    ExecWait '"$SYSDIR\taskkill.exe" /F /IM "$ExeName"' $0

    ; On upgrade Explorer may still map the old DLL: restart it before copying,
    ; otherwise the upgrade keeps running the old code.
    IfFileExists "$INSTDIR\$DllName" 0 no_old_install
        ${IfNot} ${Silent}
            MessageBox MB_YESNO|MB_ICONQUESTION \
                "Windows Explorer will be restarted to replace the loaded extension. Continue?" \
                IDYES +2
            Abort
        ${EndIf}
        ExecWait '"$SYSDIR\taskkill.exe" /F /IM explorer.exe' $0
        StrCpy $RestartedExplorer 1
    no_old_install:

    SetOutPath "$INSTDIR"
    ${If} ${RunningX64}
        File "src\build\ExplorerEnhancement_x64.dll"
        File "src\build\ExplorerEnhancementManager_x64.exe"
    ${Else}
        File "src\build\ExplorerEnhancement_x86.dll"
        File "src\build\ExplorerEnhancementManager_x86.exe"
    ${EndIf}

    ; Unregister the old copy first. Then the new manager registers its own
    ; architecture DLL per user (a 64-bit exe can load the 64-bit DLL).
    IfFileExists "$INSTDIR\$ExeName" 0 skip_unregister
        ExecWait '"$INSTDIR\$ExeName" /uninstall' $0
    skip_unregister:
    ExecWait '"$INSTDIR\$ExeName" /install' $0
    ${If} $0 != 0
        ${IfNot} ${Silent}
            MessageBox MB_ICONSTOP \
                "DLL registration failed. The extension was copied to $INSTDIR but is not registered. Try running $INSTDIR\$ExeName /install."
        ${EndIf}
        SetErrorLevel 2
        Abort
    ${EndIf}

    CreateDirectory "$SMPROGRAMS\anpho's Extensions"
    CreateShortcut "$SMPROGRAMS\anpho's Extensions\anpho's Extensions Manager.lnk" "$INSTDIR\$ExeName"
    CreateShortcut "$SMPROGRAMS\anpho's Extensions\Uninstall.lnk" "$INSTDIR\uninstall.exe"

    WriteUninstaller "$INSTDIR\uninstall.exe"

    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "DisplayIcon" "$INSTDIR\$ExeName,0"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "Publisher" "anpho"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "InstallDate" "$(^YYYY)-$(^MM)-$(^DD)"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
    WriteRegDWORD HKCU "${UNINSTALL_REG_KEY}" "NoModify" 1
    WriteRegDWORD HKCU "${UNINSTALL_REG_KEY}" "NoRepair" 1
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "URLInfoAbout" "https://x.com/anpho"
    WriteRegStr HKCU "${UNINSTALL_REG_KEY}" "HelpLink" "mailto:support@anpho.cn"

SectionEnd

; -------------------------------------------------------------- uninstall
Section "Uninstall"

    ${If} ${RunningX64}
        StrCpy $ExeName "ExplorerEnhancementManager_x64.exe"
    ${Else}
        StrCpy $ExeName "ExplorerEnhancementManager_x86.exe"
    ${EndIf}

    ; Unregister first; on failure warn but still remove the files and report it.
    ExecWait '"$SYSDIR\taskkill.exe" /F /IM "$ExeName"' $0
    IfFileExists "$INSTDIR\$ExeName" 0 skip_unregister
        ExecWait '"$INSTDIR\$ExeName" /uninstall' $0
        ${If} $0 != 0
            ${IfNot} ${Silent}
                MessageBox MB_ICONEXCLAMATION \
                    "Could not remove the DLL registration (exit code $0). You may need to reinstall and uninstall again."
            ${EndIf}
        ${EndIf}
    skip_unregister:

    ClearErrors
    Delete "$INSTDIR\ExplorerEnhancement_x64.dll"
    Delete "$INSTDIR\ExplorerEnhancement_x86.dll"
    Delete "$INSTDIR\ExplorerEnhancementManager_x64.exe"
    Delete "$INSTDIR\ExplorerEnhancementManager_x86.exe"
    Delete "$INSTDIR\uninstall.exe"
    IfErrors 0 files_deleted
        ${IfNot} ${Silent}
            MessageBox MB_ICONEXCLAMATION \
                "Some files could not be deleted. Restart Windows Explorer and run the uninstaller again."
        ${EndIf}
    files_deleted:

    Delete "$SMPROGRAMS\anpho's Extensions\anpho's Extensions Manager.lnk"
    Delete "$SMPROGRAMS\anpho's Extensions\Uninstall.lnk"
    RMDir "$SMPROGRAMS\anpho's Extensions"

    ; Remove the per-user disabled state written by the manager.
    DeleteRegValue HKCU "${BLOCKED_KEY}" "${CLSID_NEWFOLDER}"
    DeleteRegValue HKCU "${BLOCKED_KEY}" "${CLSID_BREAKFOLDER}"
    DeleteRegKey HKCU "${UNINSTALL_REG_KEY}"

    RMDir "$INSTDIR"

    ${IfNot} ${Silent}
        MessageBox MB_ICONINFORMATION \
            "${PRODUCT_NAME} has been uninstalled. Restart Windows Explorer for the change to take effect."
    ${EndIf}
SectionEnd

Function .onInstSuccess
    ${If} $RestartedExplorer == 1
        Exec '"$WINDIR\explorer.exe"'
    ${EndIf}
    ${IfNot} ${Silent}
        ExecShell "open" "$INSTDIR\$ExeName"
    ${EndIf}
FunctionEnd
