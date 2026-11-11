#include "pch.h"
#include "RegistryHelper.h"

// 定义CLSID常量
const LPCTSTR CRegistryHelper::m_szBreakFolderCLSID = _T("{f2a0aaf9-2961-4465-a843-884383fcd13b}");
const LPCTSTR CRegistryHelper::m_szNewFolderCLSID = _T("{c36a65af-0a1e-4e09-9729-6da3e4e4461a}");

namespace
{
// 壳会检查该键：值名是 CLSID，存在即表示扩展被停用。
const TCHAR kBlockedKeyPath[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked");

// DLL 的 DllRegisterServer 使用 AtlSetPerUserRegistration(true)，写入 HKCU\Software\Classes。
bool IsClassRegistered(LPCTSTR clsid)
{
    CRegKey key;
    TCHAR keyPath[256] = {};
    _sntprintf_s(keyPath, _countof(keyPath), _TRUNCATE,
                 _T("Software\\Classes\\CLSID\\%s\\InprocServer32"), clsid);
    return key.Open(HKEY_CURRENT_USER, keyPath, KEY_READ) == ERROR_SUCCESS;
}
} // namespace

CRegistryHelper::CRegistryHelper()
{
}

CRegistryHelper::~CRegistryHelper()
{
}

BOOL CRegistryHelper::IsExplorerEnhancementRegistered()
{
    return IsClassRegistered(m_szNewFolderCLSID) && IsClassRegistered(m_szBreakFolderCLSID);
}

BOOL CRegistryHelper::IsBreakFolderEnabled()
{
    return IsContextMenuEnabled(_T("BreakFolder"));
}

BOOL CRegistryHelper::SetBreakFolderEnabled(BOOL bEnable)
{
    return SetContextMenuEnabled(_T("BreakFolder"), bEnable);
}

BOOL CRegistryHelper::IsNewFolderEnabled()
{
    return IsContextMenuEnabled(_T("NewFolder"));
}

BOOL CRegistryHelper::SetNewFolderEnabled(BOOL bEnable)
{
    return SetContextMenuEnabled(_T("NewFolder"), bEnable);
}

BOOL CRegistryHelper::IsContextMenuEnabled(LPCTSTR lpMenuName)
{
    CRegKey key;
    LONG lResult;

    // 获取对应的CLSID
    LPCTSTR lpCLSID = (_tcscmp(lpMenuName, _T("BreakFolder")) == 0) ?
        m_szBreakFolderCLSID : m_szNewFolderCLSID;

    // 尝试打开Blocked键
    lResult = key.Open(HKEY_CURRENT_USER, kBlockedKeyPath, KEY_READ);
    if (lResult != ERROR_SUCCESS)
    {
        // Blocked键不存在，说明功能已启用
        return TRUE;
    }

    // 检查CLSID是否存在于Blocked键中（只关心是否存在，不读值）
    DWORD dwType = 0;
    DWORD dwSize = 0;
    lResult = key.QueryValue(lpCLSID, &dwType, NULL, &dwSize);
    return lResult == ERROR_SUCCESS ? FALSE : TRUE;
}

BOOL CRegistryHelper::SetContextMenuEnabled(LPCTSTR lpMenuName, BOOL bEnable)
{
    CRegKey key;
    LONG lResult;

    // 获取对应的CLSID
    LPCTSTR lpCLSID = (_tcscmp(lpMenuName, _T("BreakFolder")) == 0) ?
        m_szBreakFolderCLSID : m_szNewFolderCLSID;

    if (bEnable)
    {
        // 启用功能：从Blocked键中删除CLSID
        lResult = key.Open(HKEY_CURRENT_USER, kBlockedKeyPath, KEY_WRITE);
        if (lResult == ERROR_SUCCESS)
        {
            lResult = key.DeleteValue(lpCLSID);
            if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
            {
                return FALSE;
            }
        }
        else if (lResult != ERROR_FILE_NOT_FOUND)
        {
            // 打开失败且不是因为键不存在
            return FALSE;
        }
        // Blocked键不存在时功能本来就是启用的
    }
    else
    {
        // 禁用功能：将CLSID添加到Blocked键中
        lResult = key.Create(HKEY_CURRENT_USER, kBlockedKeyPath);
        if (lResult != ERROR_SUCCESS)
        {
            return FALSE;
        }

        // 值内容无关紧要，存在即停用
        lResult = key.SetStringValue(lpCLSID, _T(""));
        if (lResult != ERROR_SUCCESS)
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CRegistryHelper::RegisterExplorerEnhancementDLL(LPCTSTR lpDllPath)
{
    HMODULE hModule = LoadLibrary(lpDllPath);
    if (hModule == NULL)
    {
        return FALSE;
    }

    typedef HRESULT (WINAPI* LPDLLREGISTERSERVER)(void);
    LPDLLREGISTERSERVER lpDllRegisterServer =
        (LPDLLREGISTERSERVER)GetProcAddress(hModule, "DllRegisterServer");
    if (lpDllRegisterServer == NULL)
    {
        FreeLibrary(hModule);
        return FALSE;
    }

    HRESULT hr = lpDllRegisterServer();
    FreeLibrary(hModule);

    return SUCCEEDED(hr);
}

BOOL CRegistryHelper::UnregisterExplorerEnhancementDLL(LPCTSTR lpDllPath)
{
    HMODULE hModule = LoadLibrary(lpDllPath);
    if (hModule == NULL)
    {
        return FALSE;
    }

    typedef HRESULT (WINAPI* LPDLLUNREGISTERSERVER)(void);
    LPDLLUNREGISTERSERVER lpDllUnregisterServer =
        (LPDLLUNREGISTERSERVER)GetProcAddress(hModule, "DllUnregisterServer");
    if (lpDllUnregisterServer == NULL)
    {
        FreeLibrary(hModule);
        return FALSE;
    }

    HRESULT hr = lpDllUnregisterServer();
    FreeLibrary(hModule);

    return SUCCEEDED(hr);
}
