#include "pch.h"
#include "RegistryHelper.h"

// 定义CLSID常量
const LPCTSTR CRegistryHelper::m_szBreakFolderCLSID = _T("{f2a0aaf9-2961-4465-a843-884383fcd13b}");
const LPCTSTR CRegistryHelper::m_szNewFolderCLSID = _T("{c36a65af-0a1e-4e09-9729-6da3e4e4461a}");

CRegistryHelper::CRegistryHelper()
{
}

CRegistryHelper::~CRegistryHelper()
{
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
    const TCHAR szBlockedKeyPath[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked");

    // 获取对应的CLSID
    LPCTSTR lpCLSID = (_tcscmp(lpMenuName, _T("BreakFolder")) == 0) ? 
        m_szBreakFolderCLSID : m_szNewFolderCLSID;

    // 尝试打开Blocked键
    lResult = key.Open(HKEY_CURRENT_USER, szBlockedKeyPath, KEY_READ);
    if (lResult != ERROR_SUCCESS)
    {
        // Blocked键不存在，说明功能已启用
        return TRUE;
    }

    // 检查CLSID是否存在于Blocked键中
    DWORD dwType = 0;
    DWORD dwSize = 0;
    lResult = key.QueryValue(lpCLSID, &dwType, NULL, &dwSize);
    if (lResult == ERROR_SUCCESS)
    {
        // CLSID存在于Blocked键中，说明功能已禁用
        return FALSE;
    }

    // CLSID不存在于Blocked键中，说明功能已启用
    return TRUE;
}

BOOL CRegistryHelper::SetContextMenuEnabled(LPCTSTR lpMenuName, BOOL bEnable)
{
    CRegKey key;
    LONG lResult;
    const TCHAR szBlockedKeyPath[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked");

    // 获取对应的CLSID
    LPCTSTR lpCLSID = (_tcscmp(lpMenuName, _T("BreakFolder")) == 0) ? 
        m_szBreakFolderCLSID : m_szNewFolderCLSID;

    if (bEnable)
    {
        // 启用功能：从Blocked键中删除CLSID
        lResult = key.Open(HKEY_CURRENT_USER, szBlockedKeyPath, KEY_WRITE);
        if (lResult == ERROR_SUCCESS)
        {
            // 尝试删除CLSID值
            lResult = key.DeleteValue(lpCLSID);
            if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
            {
                return FALSE;
            }
        }
        else if (lResult != ERROR_FILE_NOT_FOUND)
        {
            // 如果打开失败且不是因为键不存在，返回失败
            return FALSE;
        }
        // 如果Blocked键不存在，说明功能已经是启用状态
    }
    else
    {
        // 禁用功能：将CLSID添加到Blocked键中
        lResult = key.Create(HKEY_CURRENT_USER, szBlockedKeyPath);
        if (lResult != ERROR_SUCCESS)
        {
            return FALSE;
        }

        // 设置CLSID值（空字符串即可）
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
    // 这里应该实现DLL的注册逻辑
    // 通常使用DllRegisterServer函数
    HMODULE hModule = LoadLibrary(lpDllPath);
    if (hModule == NULL)
    {
        return FALSE;
    }

    typedef HRESULT (WINAPI* LPDLLREGISTERSERVER)(void);
    LPDLLREGISTERSERVER lpDllRegisterServer = (LPDLLREGISTERSERVER)GetProcAddress(hModule, "DllRegisterServer");
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
    // 这里应该实现DLL的卸载逻辑
    // 通常使用DllUnregisterServer函数
    HMODULE hModule = LoadLibrary(lpDllPath);
    if (hModule == NULL)
    {
        return FALSE;
    }

    typedef HRESULT (WINAPI* LPDLLUNREGISTERSERVER)(void);
    LPDLLUNREGISTERSERVER lpDllUnregisterServer = (LPDLLUNREGISTERSERVER)GetProcAddress(hModule, "DllUnregisterServer");
    if (lpDllUnregisterServer == NULL)
    {
        FreeLibrary(hModule);
        return FALSE;
    }

    HRESULT hr = lpDllUnregisterServer();
    FreeLibrary(hModule);

    return SUCCEEDED(hr);
}