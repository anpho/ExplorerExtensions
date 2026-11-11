#pragma once

#include <atlbase.h>
#include <atlstr.h>

class CRegistryHelper
{
public:
    CRegistryHelper();
    ~CRegistryHelper();

    // DLL 是否已按当前用户注册（HKCU\Software\Classes\CLSID\...\InprocServer32）。
    BOOL IsExplorerEnhancementRegistered();

    // 检查并启用/禁用 BreakFolder 上下文菜单
    BOOL IsBreakFolderEnabled();
    BOOL SetBreakFolderEnabled(BOOL bEnable);

    // 检查并启用/禁用 NewFolder 上下文菜单
    BOOL IsNewFolderEnabled();
    BOOL SetNewFolderEnabled(BOOL bEnable);

    // 注册/卸载 ExplorerEnhancement DLL
    BOOL RegisterExplorerEnhancementDLL(LPCTSTR lpDllPath);
    BOOL UnregisterExplorerEnhancementDLL(LPCTSTR lpDllPath);

private:
    // 检查并启用/禁用指定的上下文菜单
    BOOL IsContextMenuEnabled(LPCTSTR lpMenuName);
    BOOL SetContextMenuEnabled(LPCTSTR lpMenuName, BOOL bEnable);

    // 上下文菜单的CLSID
    static const LPCTSTR m_szBreakFolderCLSID;
    static const LPCTSTR m_szNewFolderCLSID;
};
