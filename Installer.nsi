; Windows Explorer Extension Installer
; NSIS Script

; 设置压缩算法
SetCompressor /SOLID lzma

; 定义安装程序名称
OutFile "ExplorerExtensionSetup.exe"

; 定义安装程序图标（可选）
; Icon "icon.ico"

; 明确指定不需要管理员权限
RequestExecutionLevel user

; 定义安装目录
InstallDir $LOCALAPPDATA\ExplorerEnhancement

; 安装程序页面
Page Directory
Page InstFiles

; 卸载程序页面
UninstPage UninstConfirm
UninstPage InstFiles

; 定义安装程序区域
Section "Main Section" SEC01
    ; 设置输出目录
    SetOutPath $INSTDIR
    
    ; 检测系统架构
    StrCmp $PROCESSOR_ARCHITECTURE "x86" arch_x86 arch_x64
    
    arch_x86:
        ; 先取消注册已存在的DLL（如果有）
        IfFileExists "$INSTDIR\ExplorerEnhancement_x86.dll" 0 skip_unreg_x86
            ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x86.dll'
        skip_unreg_x86:
        File "src\build\ExplorerEnhancement_x86.dll"
        File "src\build\ExplorerEnhancementManager_x86.exe"
        Goto arch_end
    
    arch_x64:
        ; 先取消注册已存在的DLL（如果有）
        IfFileExists "$INSTDIR\ExplorerEnhancement_x64.dll" 0 skip_unreg_x64
            ExecWait '$SYSDIR\regsvr32.exe /s /u $INSTDIR\ExplorerEnhancement_x64.dll'
        skip_unreg_x64:
        File "src\build\ExplorerEnhancement_x64.dll"
        File "src\build\ExplorerEnhancementManager_x64.exe"
    
    arch_end:
    
    ; 注册DLL
    StrCmp $PROCESSOR_ARCHITECTURE "x86" reg_x86 reg_x64
    
    reg_x86:
        ExecWait '$SYSDIR\regsvr32.exe /s $INSTDIR\ExplorerEnhancement_x86.dll'
        Goto reg_end
    
    reg_x64:
        ExecWait '$SYSDIR\regsvr32.exe /s $INSTDIR\ExplorerEnhancement_x64.dll'
    
    reg_end:
    
    ; 创建开始菜单快捷方式
    CreateDirectory "$SMPROGRAMS\ExplorerEnhancement"
    StrCmp $PROCESSOR_ARCHITECTURE "x86" shortcut_x86 shortcut_x64
    
    shortcut_x86:
        CreateShortcut "$SMPROGRAMS\ExplorerEnhancement\Explorer Enhancement Manager.lnk" "$INSTDIR\ExplorerEnhancementManager_x86.exe"
        Goto shortcut_end
    
    shortcut_x64:
        CreateShortcut "$SMPROGRAMS\ExplorerEnhancement\Explorer Enhancement Manager.lnk" "$INSTDIR\ExplorerEnhancementManager_x64.exe"
    
    shortcut_end:
    CreateShortcut "$SMPROGRAMS\ExplorerEnhancement\Uninstall.lnk" "$INSTDIR\uninstall.exe"
    
    ; 创建卸载程序
    WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

; 卸载程序区域
Section "Uninstall"
    ; 取消注册DLL
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
    
    ; 删除快捷方式
    Delete "$SMPROGRAMS\ExplorerEnhancement\Explorer Enhancement Manager.lnk"
    Delete "$SMPROGRAMS\ExplorerEnhancement\Uninstall.lnk"
    RMDir "$SMPROGRAMS\ExplorerEnhancement"
    
    ; 删除卸载程序
    Delete "$INSTDIR\uninstall.exe"
    
    ; 删除安装目录
    RMDir "$INSTDIR"
    
    ; 提示用户重启Windows Explorer
    MessageBox MB_ICONINFORMATION "Windows Explorer Extension has been successfully uninstalled.\r\nPlease restart Windows Explorer or your computer for the changes to take effect."
SectionEnd

; 安装完成后提示用户重启Windows Explorer
Function .onInstSuccess
    MessageBox MB_ICONINFORMATION "Windows Explorer Extension has been successfully installed.\r\nPlease restart Windows Explorer or your computer for the changes to take effect."
FunctionEnd
