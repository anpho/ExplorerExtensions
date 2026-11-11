// Registration.h: 直接写/删 COM 注册表项（不依赖 ATL 注册脚本和 DllRegisterServer）。
//
// 为什么不用 ATL 的按用户注册：AtlSetPerUserRegistration 的重定向在本项目实测
// 没有生效——普通权限注册失败、管理员权限才成功。这里按调用方给的根键显式写，
// 用户范围（HKCU）和全局范围（HKLM）都由调用方决定，行为可预期。
#pragma once

#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>

namespace Registration
{
inline constexpr wchar_t kNewFolderClsid[] = L"{c36a65af-0a1e-4e09-9729-6da3e4e4461a}";
inline constexpr wchar_t kBreakFolderClsid[] = L"{f2a0aaf9-2961-4465-a843-884383fcd13b}";
inline constexpr const wchar_t* kClassIds[] = { kNewFolderClsid, kBreakFolderClsid };

// ContextMenuHandlers 下是“子键 + 默认值 = CLSID”的形式。
struct HandlerEntry
{
    LPCWSTR parentPath; // ContextMenuHandlers 键
    LPCWSTR name;       // 处理器子键名
    LPCWSTR clsid;
};

inline const HandlerEntry kHandlers[] = {
    { L"Software\\Classes\\*\\shellex\\ContextMenuHandlers", L"NewFolderExtension", kNewFolderClsid },
    { L"Software\\Classes\\Directory\\shellex\\ContextMenuHandlers", L"NewFolderExtension", kNewFolderClsid },
    { L"Software\\Classes\\Directory\\shellex\\ContextMenuHandlers", L"BreakFolder", kBreakFolderClsid },
};

inline LPCWSTR RootName(HKEY root)
{
    return (root == HKEY_LOCAL_MACHINE) ? L"HKLM" : L"HKCU";
}

inline bool HasValue(HKEY root, LPCWSTR keyPath, LPCWSTR valueName)
{
    ATL::CRegKey key;
    if (key.Open(root, keyPath, KEY_READ) != ERROR_SUCCESS)
    {
        return false;
    }
    DWORD type = 0;
    DWORD size = 0;
    return key.QueryValue(valueName, &type, nullptr, &size) == ERROR_SUCCESS;
}

inline bool ClassRegistered(HKEY root, LPCWSTR clsid)
{
    wchar_t keyPath[256] = {};
    swprintf_s(keyPath, L"Software\\Classes\\CLSID\\%s\\InprocServer32", clsid);
    return HasValue(root, keyPath, nullptr);
}

inline bool HandlerRegistered(HKEY root, const HandlerEntry& handler)
{
    ATL::CString keyPath;
    keyPath.Format(L"%s\\%s", handler.parentPath, handler.name);
    return HasValue(root, keyPath.GetString(), nullptr);
}

inline bool IsRegistered(HKEY root)
{
    return ClassRegistered(root, kNewFolderClsid)
        && ClassRegistered(root, kBreakFolderClsid)
        && HandlerRegistered(root, kHandlers[0])
        && HandlerRegistered(root, kHandlers[1])
        && HandlerRegistered(root, kHandlers[2]);
}

inline bool WriteValue(HKEY root, LPCWSTR keyPath, LPCWSTR valueName, LPCWSTR value, ATL::CString& error, LONG* lastError = nullptr)
{
    ATL::CRegKey key;
    LONG result = key.Create(root, keyPath);
    if (result != ERROR_SUCCESS)
    {
        if (lastError != nullptr) { *lastError = result; }
        error.Format(L"Create %s\\%s failed: %u", RootName(root), keyPath, result);
        return false;
    }
    result = key.SetStringValue(valueName, value);
    if (result != ERROR_SUCCESS)
    {
        if (lastError != nullptr) { *lastError = result; }
        error.Format(L"Set %s\\%s\\%s failed: %u", RootName(root), keyPath,
                     valueName != nullptr ? valueName : L"(default)", result);
        return false;
    }
    return true;
}

inline bool Register(HKEY root, LPCWSTR dllPath, ATL::CString& error, LONG* lastError = nullptr)
{
    if (lastError != nullptr) { *lastError = ERROR_SUCCESS; }
    wchar_t keyPath[256] = {};
    for (const wchar_t* clsid : kClassIds)
    {
        swprintf_s(keyPath, L"Software\\Classes\\CLSID\\%s\\InprocServer32", clsid);
        if (!WriteValue(root, keyPath, nullptr, dllPath, error, lastError))
        {
            return false;
        }
        if (!WriteValue(root, keyPath, L"ThreadingModel", L"Apartment", error, lastError))
        {
            return false;
        }
    }

    for (const auto& handler : kHandlers)
    {
        ATL::CString handlerKey;
        handlerKey.Format(L"%s\\%s", handler.parentPath, handler.name);
        if (!WriteValue(root, handlerKey.GetString(), nullptr, handler.clsid, error, lastError))
        {
            return false;
        }
    }

    if (!IsRegistered(root))
    {
        if (lastError != nullptr) { *lastError = ERROR_INVALID_DATA; }
        error.Format(L"Registration verification failed under %s.", RootName(root));
        return false;
    }
    return true;
}

inline bool Unregister(HKEY root, ATL::CString& error)
{
    // 删除 CLSID 子树（两个类都在 Software\Classes\CLSID 下）
    ATL::CRegKey clsidRoot;
    if (clsidRoot.Open(root, L"Software\\Classes\\CLSID", KEY_READ | KEY_WRITE) == ERROR_SUCCESS)
    {
        for (const wchar_t* clsid : kClassIds)
        {
            const LONG result = clsidRoot.RecurseDeleteKey(clsid);
            if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND)
            {
                error.Format(L"Delete %s\\Software\\Classes\\CLSID\\%s failed: %u",
                             RootName(root), clsid, result);
                return false;
            }
        }
    }

    // 删除 ContextMenuHandlers 下的处理器子键
    for (const auto& handler : kHandlers)
    {
        ATL::CRegKey parent;
        if (parent.Open(root, handler.parentPath, KEY_READ | KEY_WRITE) != ERROR_SUCCESS)
        {
            continue;
        }
        const LONG result = parent.RecurseDeleteKey(handler.name);
        if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND)
        {
            error.Format(L"Delete %s\\%s\\%s failed: %u", RootName(root),
                         handler.parentPath, handler.name, result);
            return false;
        }
    }

    return !IsRegistered(root);
}
} // namespace Registration
