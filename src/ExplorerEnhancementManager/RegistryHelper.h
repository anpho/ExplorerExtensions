#pragma once

#include <atlbase.h>
#include <atlstr.h>
#include "../Common/Registration.h"

// 扩展的注册范围。
enum class InstallScope
{
    None,    // 未注册
    User,    // HKCU\Software\Classes（当前用户，不需要管理员）
    Machine, // HKLM\Software\Classes（所有用户，需要管理员）
};

class CRegistryHelper
{
public:
    CRegistryHelper();
    ~CRegistryHelper();

    // 当前进程是否以管理员权限运行。
    static bool IsProcessElevated();

    // 已注册的范围；两个范围都存在时返回 Machine。
    InstallScope GetInstallScope();
    bool IsScopeRegistered(InstallScope scope);

    // 直接写/删注册表项。error 返回失败的具体键和错误码，lastError 返回 Win32 错误码。
    bool RegisterExtension(LPCTSTR dllPath, InstallScope scope, CString& error, LONG& lastError);
    bool UnregisterExtension(InstallScope scope, CString& error);

    // 启用状态：Machine 范围读写 HKLM，User 范围读写 HKCU。
    BOOL IsBreakFolderEnabled(bool machineScope);
    BOOL SetBreakFolderEnabled(BOOL bEnable, bool machineScope);
    BOOL IsNewFolderEnabled(bool machineScope);
    BOOL SetNewFolderEnabled(BOOL bEnable, bool machineScope);

private:
    BOOL IsContextMenuEnabled(LPCTSTR valueName, bool machineScope);
    BOOL SetContextMenuEnabled(LPCTSTR valueName, BOOL bEnable, bool machineScope);
};
