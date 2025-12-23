#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "MainDialog.h"
#include "RegistryHelper.h"

// 处理命令行参数
BOOL ProcessCommandLine(LPWSTR lpCmdLine, CRegistryHelper& registryHelper)
{
    // 检查是否有 /install 参数
    if (_wcsicmp(lpCmdLine, L"/install") == 0)
    {
        // 找到 ExplorerEnhancement DLL 的路径
        WCHAR szModulePath[MAX_PATH];
        if (GetModuleFileName(NULL, szModulePath, _countof(szModulePath)) == 0)
        {
            MessageBox(NULL, _T("无法获取模块路径"), _T("错误"), MB_OK | MB_ICONERROR);
            return FALSE;
        }

        // 替换文件名
        WCHAR* pLastBackslash = wcsrchr(szModulePath, L'\\');
        if (pLastBackslash != NULL)
        {
            *pLastBackslash = L'\0';
        }

        // 构建 DLL 路径
        WCHAR szDllPath[MAX_PATH];
        _snwprintf_s(szDllPath, _countof(szDllPath), _TRUNCATE, L"%s\\ExplorerEnhancement_%s.dll", 
            szModulePath, (sizeof(void*) == 8) ? L"x64" : L"x86");

        // 注册 DLL
        if (registryHelper.RegisterExplorerEnhancementDLL(szDllPath))
        {
            MessageBox(NULL, _T("ExplorerEnhancement DLL 注册成功"), _T("成功"), MB_OK | MB_ICONINFORMATION);
            return TRUE;
        }
        else
        {
            MessageBox(NULL, _T("ExplorerEnhancement DLL 注册失败"), _T("错误"), MB_OK | MB_ICONERROR);
            return FALSE;
        }
    }

    return FALSE;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, 
                   _In_opt_ HINSTANCE hPrevInstance, 
                   _In_ LPWSTR    lpCmdLine, 
                   _In_ int       nCmdShow)
{
    HRESULT hRes = ::CoInitialize(NULL);
    ATLASSERT(SUCCEEDED(hRes));

    // ATL模块会自动初始化，无需显式调用Init方法

    CRegistryHelper registryHelper;

    // 检查命令行参数
    if (ProcessCommandLine(lpCmdLine, registryHelper))
    {
        ::CoUninitialize();
        return 0;
    }

    // 显示主对话框
    CMainDialog dlg;
    int nRet = static_cast<int>(dlg.DoModal());

    // 清理
    ::CoUninitialize();

    return nRet;
}