// ExplorerEnhancementManager.cpp: 管理器程序入口
// 图形界面用于启用/停用右键菜单；/install 和 /uninstall 供安装程序调用，不显示窗口。

#include "pch.h"
#include "resource.h"
#include "MainDialog.h"
#include "RegistryHelper.h"

#include <string>

WTL::CAppModule _Module;

namespace
{
// 管理器 exe 旁边的 DLL：ExplorerEnhancement_<arch>.dll
bool BuildDllPath(std::wstring& dllPath)
{
    // 安装路径可能很长，按需扩容，不使用 MAX_PATH 缓冲。
    std::wstring modulePath(MAX_PATH, L'\0');
    for (;;)
    {
        const DWORD length = ::GetModuleFileNameW(nullptr, modulePath.data(),
                                                  static_cast<DWORD>(modulePath.size()));
        if (length == 0)
        {
            return false;
        }
        if (length < modulePath.size())
        {
            modulePath.resize(length);
            break;
        }
        if (modulePath.size() >= 32768)
        {
            return false;
        }
        modulePath.resize(modulePath.size() * 2);
    }

    const size_t slash = modulePath.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
    {
        return false;
    }
    modulePath.resize(slash + 1);
    dllPath = modulePath + L"ExplorerEnhancement_" + (sizeof(void*) == 8 ? L"x64" : L"x86") + L".dll";
    return true;
}

enum class CommandMode
{
    NotRequested,
    Success,
    Failure,
};

// lpCmdLine 可能带前导空格/引号（不同启动方式），先规整再比较。
bool IsSwitch(LPCWSTR commandLine, LPCWSTR name)
{
    std::wstring argument(commandLine);
    const size_t first = argument.find_first_not_of(L" \t\"");
    if (first == std::wstring::npos)
    {
        return false;
    }
    const size_t last = argument.find_last_not_of(L" \t\"");
    argument = argument.substr(first, last - first + 1);
    return _wcsicmp(argument.c_str(), name) == 0;
}

// /install、/uninstall：无窗口，进程退出码即结果（安装程序检查）。
CommandMode RunCommandLine(LPCWSTR commandLine)
{
    const bool install = IsSwitch(commandLine, L"/install");
    const bool uninstall = IsSwitch(commandLine, L"/uninstall");
    if (!install && !uninstall)
    {
        return CommandMode::NotRequested;
    }

    std::wstring dllPath;
    if (!BuildDllPath(dllPath))
    {
        return CommandMode::Failure;
    }

    CRegistryHelper registryHelper;
    const bool ok = install ? registryHelper.RegisterExplorerEnhancementDLL(dllPath.c_str())
                            : registryHelper.UnregisterExplorerEnhancementDLL(dllPath.c_str());
    return ok ? CommandMode::Success : CommandMode::Failure;
}
} // namespace

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // COM 由本进程负责初始化/反初始化（之前 Apply 里多调用一次 CoUninitialize 的 bug 已去掉）。
    const HRESULT comResult = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(comResult))
    {
        return 1;
    }

    if (FAILED(_Module.Init(nullptr, hInstance)))
    {
        ::CoUninitialize();
        return 1;
    }

    const CommandMode mode = RunCommandLine(lpCmdLine);
    if (mode != CommandMode::NotRequested)
    {
        _Module.Term();
        ::CoUninitialize();
        return mode == CommandMode::Success ? 0 : 1;
    }

    INITCOMMONCONTROLSEX iccex = {};
    iccex.dwSize = sizeof(iccex);
    iccex.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    ::InitCommonControlsEx(&iccex);

    WTL::CMessageLoop messageLoop;
    _Module.AddMessageLoop(&messageLoop);

    CMainDialog dialog;
    int result = 1;
    if (dialog.Create(nullptr, 0) != nullptr)
    {
        dialog.ShowWindow(nCmdShow);
        dialog.UpdateWindow();
        result = messageLoop.Run();
    }

    _Module.RemoveMessageLoop();
    _Module.Term();
    ::CoUninitialize();
    return result;
}
