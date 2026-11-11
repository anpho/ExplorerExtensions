// ExplorerEnhancementManager.cpp: 管理器程序入口
// 图形界面用于启用/停用右键菜单；/install 和 /uninstall 供安装程序调用，不显示窗口。
// 注册范围跟随权限：普通用户 -> HKCU，管理员 -> HKLM（所有用户）。

#include "pch.h"
#include "resource.h"
#include "MainDialog.h"
#include "RegistryHelper.h"
#include "../Common/LocalizedString.h"

#include <string>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

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

// 注册失败时把细节写到 %LOCALAPPDATA%\ExplorerEnhancement\manager.log，方便反馈。
void WriteManagerLog(const CString& message)
{
    wchar_t localAppData[MAX_PATH] = {};
    if (::GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH) == 0)
    {
        return;
    }

    CString directory;
    directory.Format(L"%s\\ExplorerEnhancement", localAppData);
    ::CreateDirectoryW(directory, nullptr);

    CString logPath;
    logPath.Format(L"%s\\manager.log", directory.GetString());
    HANDLE file = ::CreateFileW(logPath, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr,
                                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return;
    }

    SYSTEMTIME now = {};
    ::GetLocalTime(&now);
    CString line;
    line.Format(L"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
                now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond,
                message.GetString());

    const int byteCount = ::WideCharToMultiByte(CP_UTF8, 0, line, -1, nullptr, 0, nullptr, nullptr);
    if (byteCount > 1)
    {
        std::string utf8(static_cast<size_t>(byteCount - 1), '\0');
        ::WideCharToMultiByte(CP_UTF8, 0, line, -1, utf8.data(), byteCount, nullptr, nullptr);
        DWORD written = 0;
        ::WriteFile(file, utf8.data(), static_cast<DWORD>(utf8.size()), &written, nullptr);
    }
    ::CloseHandle(file);
}

void ShowRegistrationFailure(const CString& detail)
{
    CString message = LoadLocalizedString(IDS_REGISTER_FAILED);
    if (!detail.IsEmpty())
    {
        message += L"\n\n";
        message += detail;
    }
    ::MessageBoxW(nullptr, message, LoadLocalizedString(IDS_INFO_TITLE),
                  MB_ICONERROR | MB_OK);
}

// 普通权限无法写 HKCU\Software\Classes\CLSID 时（Windows 的用户级 COM 加固），
// 弹 UAC 让同一 exe 以管理员身份跑 /install，注册到所有用户。
bool TryElevatedInstall()
{
    wchar_t modulePath[32768] = {};
    if (::GetModuleFileNameW(nullptr, modulePath, ARRAYSIZE(modulePath)) == 0)
    {
        return false;
    }

    SHELLEXECUTEINFOW info = {};
    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.lpVerb = L"runas";
    info.lpFile = modulePath;
    info.lpParameters = L"/install";
    info.nShow = SW_HIDE;
    if (!::ShellExecuteExW(&info))
    {
        return false;
    }

    ::WaitForSingleObject(info.hProcess, 120000);
    DWORD exitCode = 1;
    ::GetExitCodeProcess(info.hProcess, &exitCode);
    ::CloseHandle(info.hProcess);
    return exitCode == 0;
}

// 全局注册只能由管理员修改设置；用管理员权限重新启动自己，避免停在灰掉的只读窗口。
bool RelaunchElevated()
{
    wchar_t modulePath[32768] = {};
    if (::GetModuleFileNameW(nullptr, modulePath, ARRAYSIZE(modulePath)) == 0)
    {
        return false;
    }

    SHELLEXECUTEINFOW info = {};
    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.lpVerb = L"runas";
    info.lpFile = modulePath;
    info.nShow = SW_SHOWNORMAL;
    if (!::ShellExecuteExW(&info))
    {
        return false;
    }
    ::CloseHandle(info.hProcess);
    return true;
}

// 首次注册时把两个菜单设为启用（之前可能留有 0 的禁用状态）。
void ResetEnabledDefaults(CRegistryHelper& registryHelper, InstallScope scope)
{
    const bool machineScope = (scope == InstallScope::Machine);
    registryHelper.SetBreakFolderEnabled(TRUE, machineScope);
    registryHelper.SetNewFolderEnabled(TRUE, machineScope);
}

enum class CommandMode
{
    NotRequested,
    Success,
    Failure,
};

// /install、/uninstall：无窗口，进程退出码即结果（安装程序检查）。
CommandMode RunCommandLine(LPCWSTR commandLine, bool elevated)
{
    const bool install = IsSwitch(commandLine, L"/install");
    const bool uninstall = IsSwitch(commandLine, L"/uninstall");
    if (!install && !uninstall)
    {
        return CommandMode::NotRequested;
    }

    CRegistryHelper registryHelper;
    std::wstring dllPath;
    CString error;
    LONG errorCode = ERROR_SUCCESS;

    if (install)
    {
        // 已经从“所有用户”范围安装过：普通权限下无需（也无法）再写一份用户注册。
        if (registryHelper.GetInstallScope() == InstallScope::Machine && !elevated)
        {
            return CommandMode::Success;
        }

        const InstallScope scope = elevated ? InstallScope::Machine : InstallScope::User;
        const bool wasRegistered = registryHelper.GetInstallScope() != InstallScope::None;
        if (!BuildDllPath(dllPath) || !registryHelper.RegisterExtension(dllPath.c_str(), scope, error, errorCode))
        {
            error += L"\nDLL: ";
            error += dllPath.c_str();
            WriteManagerLog(L"/install failed: " + error);
            return CommandMode::Failure;
        }
        if (scope == InstallScope::Machine)
        {
            // 移除旧的当前用户注册，避免两份注册并存
            CString unused;
            registryHelper.UnregisterExtension(InstallScope::User, unused);
        }
        if (!wasRegistered)
        {
            ResetEnabledDefaults(registryHelper, scope);
        }
        return CommandMode::Success;
    }

    // 卸载：全局注册需要管理员权限才能删除
    const InstallScope scope = registryHelper.GetInstallScope();
    if (scope == InstallScope::None)
    {
        return CommandMode::Success;
    }
    if (scope == InstallScope::Machine && !elevated)
    {
        WriteManagerLog(L"/uninstall failed: machine-wide registration needs administrator rights.");
        return CommandMode::Failure;
    }

    bool ok = registryHelper.UnregisterExtension(scope, error);
    if (scope == InstallScope::Machine)
    {
        CString unused;
        ok = registryHelper.UnregisterExtension(InstallScope::User, unused) && ok;
    }
    if (!ok)
    {
        WriteManagerLog(L"/uninstall failed: " + error);
    }
    return ok ? CommandMode::Success : CommandMode::Failure;
}
} // namespace

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    const bool elevated = CRegistryHelper::IsProcessElevated();

    // COM 由本进程负责初始化/反初始化。
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

    const CommandMode mode = RunCommandLine(lpCmdLine, elevated);
    if (mode != CommandMode::NotRequested)
    {
        _Module.Term();
        ::CoUninitialize();
        return mode == CommandMode::Success ? 0 : 1;
    }

    // 未安装时直接打开管理器：按当前权限自动注册，而不是让用户先跑安装程序。
    CRegistryHelper registryHelper;
    InstallScope scope = registryHelper.GetInstallScope();
    std::wstring dllPath;
    bool installedElevated = false;
    if (scope == InstallScope::None)
    {
        const InstallScope target = elevated ? InstallScope::Machine : InstallScope::User;
        CString error;
        LONG errorCode = ERROR_SUCCESS;
        bool registered = BuildDllPath(dllPath)
            && registryHelper.RegisterExtension(dllPath.c_str(), target, error, errorCode);
        if (!registered && errorCode == ERROR_ACCESS_DENIED && !elevated)
        {
            // Windows 的用户级 COM 加固禁止普通用户创建 HKCU\Software\Classes\CLSID 子键。
            // 询问后弹 UAC，以管理员身份装到“所有用户”。
            const CString question = LoadLocalizedString(IDS_USER_REG_DENIED_ASK);
            if (::MessageBoxW(nullptr, question, LoadLocalizedString(IDS_INFO_TITLE),
                              MB_YESNO | MB_ICONQUESTION) == IDYES && TryElevatedInstall())
            {
                registered = (registryHelper.GetInstallScope() == InstallScope::Machine);
                installedElevated = registered;
            }
        }
        if (registered)
        {
            if (registryHelper.GetInstallScope() == InstallScope::Machine)
            {
                CString unused;
                registryHelper.UnregisterExtension(InstallScope::User, unused);
            }
            scope = registryHelper.GetInstallScope();
            ResetEnabledDefaults(registryHelper, scope);
        }
        else
        {
            if (error.IsEmpty())
            {
                error = L"ExplorerEnhancement DLL not found next to the manager.";
            }
            error += L"\nDLL: ";
            error += dllPath.c_str();
            WriteManagerLog(L"Auto-registration failed: " + error);
            ShowRegistrationFailure(error);
        }
    }
    else if (elevated && scope == InstallScope::User)
    {
        // 以管理员身份打开时，升级为“所有用户”范围。
        CString error;
        LONG errorCode = ERROR_SUCCESS;
        if (BuildDllPath(dllPath) && registryHelper.RegisterExtension(dllPath.c_str(), InstallScope::Machine, error, errorCode))
        {
            CString unused;
            registryHelper.UnregisterExtension(InstallScope::User, unused);
            scope = registryHelper.GetInstallScope();
        }
        else
        {
            WriteManagerLog(L"Upgrade to machine scope failed: " + error);
        }
    }

    // 全局注册只有管理员能改设置：换成管理员权限的实例，避免出现全灰的只读窗口。
    if (scope == InstallScope::Machine && !elevated)
    {
        const bool shouldRelaunch = installedElevated
            || ::MessageBoxW(nullptr, LoadLocalizedString(IDS_RESTART_ELEVATED_ASK),
                             LoadLocalizedString(IDS_INFO_TITLE),
                             MB_YESNO | MB_ICONQUESTION) == IDYES;
        if (shouldRelaunch && RelaunchElevated())
        {
            _Module.Term();
            ::CoUninitialize();
            return 0;
        }
    }

    const bool canModify = (scope == InstallScope::User)
        || (scope == InstallScope::Machine && elevated);

    INITCOMMONCONTROLSEX iccex = {};
    iccex.dwSize = sizeof(iccex);
    iccex.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    ::InitCommonControlsEx(&iccex);

    WTL::CMessageLoop messageLoop;
    _Module.AddMessageLoop(&messageLoop);

    CMainDialog dialog;
    dialog.Configure(scope, canModify);
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
