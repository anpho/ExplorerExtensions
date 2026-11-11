#include "pch.h"
#include "RegistryHelper.h"

namespace
{
// 管理器自己的启用状态（每个菜单一个 DWORD，1=启用，0=停用）。
// 注册在哪个范围就写哪个范围：HKLM 给所有用户，HKCU 给当前用户。
const TCHAR kSettingsKey[] = _T("Software\\anpho\\ExplorerEnhancement");

HKEY RootKeyFor(bool machineScope)
{
    return machineScope ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
}
} // namespace

CRegistryHelper::CRegistryHelper()
{
}

CRegistryHelper::~CRegistryHelper()
{
}

bool CRegistryHelper::IsProcessElevated()
{
    HANDLE token = nullptr;
    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &token))
    {
        return false;
    }

    TOKEN_ELEVATION elevation = {};
    DWORD returned = sizeof(elevation);
    const bool ok = ::GetTokenInformation(token, TokenElevation, &elevation,
                                          sizeof(elevation), &returned) != FALSE;
    ::CloseHandle(token);
    return ok && (elevation.TokenIsElevated != 0);
}

bool CRegistryHelper::IsScopeRegistered(InstallScope scope)
{
    if (scope == InstallScope::None)
    {
        return false;
    }
    return Registration::IsRegistered(RootKeyFor(scope == InstallScope::Machine));
}

InstallScope CRegistryHelper::GetInstallScope()
{
    if (IsScopeRegistered(InstallScope::Machine))
    {
        return InstallScope::Machine;
    }
    if (IsScopeRegistered(InstallScope::User))
    {
        return InstallScope::User;
    }
    return InstallScope::None;
}

bool CRegistryHelper::RegisterExtension(LPCTSTR dllPath, InstallScope scope, CString& error, LONG& lastError)
{
    lastError = ERROR_SUCCESS;
    if (scope == InstallScope::None)
    {
        lastError = ERROR_INVALID_PARAMETER;
        error = L"Invalid registration scope.";
        return false;
    }
    if (dllPath == nullptr || ::GetFileAttributesW(dllPath) == INVALID_FILE_ATTRIBUTES)
    {
        lastError = ERROR_FILE_NOT_FOUND;
        error.Format(L"DLL not found: %s", dllPath != nullptr ? dllPath : L"(null)");
        return false;
    }
    return Registration::Register(RootKeyFor(scope == InstallScope::Machine), dllPath, error, &lastError);
}

bool CRegistryHelper::UnregisterExtension(InstallScope scope, CString& error)
{
    if (scope == InstallScope::None)
    {
        return true;
    }
    return Registration::Unregister(RootKeyFor(scope == InstallScope::Machine), error);
}

BOOL CRegistryHelper::IsBreakFolderEnabled(bool machineScope)
{
    return IsContextMenuEnabled(_T("EnableBreakFolderPlus"), machineScope);
}

BOOL CRegistryHelper::SetBreakFolderEnabled(BOOL bEnable, bool machineScope)
{
    return SetContextMenuEnabled(_T("EnableBreakFolderPlus"), bEnable, machineScope);
}

BOOL CRegistryHelper::IsNewFolderEnabled(bool machineScope)
{
    return IsContextMenuEnabled(_T("EnableNewFolderPlus"), machineScope);
}

BOOL CRegistryHelper::SetNewFolderEnabled(BOOL bEnable, bool machineScope)
{
    return SetContextMenuEnabled(_T("EnableNewFolderPlus"), bEnable, machineScope);
}

BOOL CRegistryHelper::IsContextMenuEnabled(LPCTSTR valueName, bool machineScope)
{
    DWORD enabled = 1;
    DWORD size = sizeof(enabled);
    if (::RegGetValue(RootKeyFor(machineScope), kSettingsKey, valueName, RRF_RT_REG_DWORD,
                      nullptr, &enabled, &size) != ERROR_SUCCESS)
    {
        // 没有设置过时默认启用
        return TRUE;
    }
    return enabled != 0;
}

BOOL CRegistryHelper::SetContextMenuEnabled(LPCTSTR valueName, BOOL bEnable, bool machineScope)
{
    CRegKey key;
    if (key.Create(RootKeyFor(machineScope), kSettingsKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }
    return key.SetDWORDValue(valueName, bEnable ? 1u : 0u) == ERROR_SUCCESS;
}
