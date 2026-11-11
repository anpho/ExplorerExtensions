// ExplorerEnhancement.cpp: DLL 导出的实现。

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "ExplorerEnhancement_i.h"
#include "dllmain.h"
#include "../Common/Registration.h"

extern HINSTANCE g_hInstance;
using namespace ATL;

namespace
{
// 当前 DLL 的完整路径（注册表 InprocServer32 要写它）。
bool GetModulePath(ATL::CString& modulePath)
{
    ATL::CString path;
    for (int size = MAX_PATH; size <= 32768; size *= 2)
    {
        if (path.GetBuffer(size) == nullptr)
        {
            return false;
        }
        const DWORD length = ::GetModuleFileNameW(g_hInstance, path.GetBuffer(), static_cast<DWORD>(size));
        path.ReleaseBuffer();
        if (length == 0)
        {
            return false;
        }
        if (length < static_cast<DWORD>(size))
        {
            modulePath = path;
            return true;
        }
    }
    return false;
}

// 选中注册表范围：HKLM = 所有用户（需要管理员），HKCU = 当前用户。
HKEY RegistrationRoot(bool machineScope)
{
    return machineScope ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
}

HRESULT RegisterAt(HKEY root)
{
    ATL::CString modulePath;
    if (!GetModulePath(modulePath))
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }
    ATL::CString error;
    return Registration::Register(root, modulePath.GetString(), error) ? S_OK : HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
}

HRESULT UnregisterAt(HKEY root)
{
    ATL::CString error;
    return Registration::Unregister(root, error) ? S_OK : E_FAIL;
}
} // namespace

// 用于确定 DLL 是否可由 OLE 卸载。
_Use_decl_annotations_
	STDAPI
	DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

// 返回一个类工厂以创建所请求类型的对象。
_Use_decl_annotations_
	STDAPI
	DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID *ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// 直接用 regsvr32 注册时默认按用户注册（不需要管理员权限）。
_Use_decl_annotations_
	STDAPI
	DllRegisterServer(void)
{
	return RegisterAt(HKEY_CURRENT_USER);
}

_Use_decl_annotations_
	STDAPI
	DllUnregisterServer(void)
{
	return UnregisterAt(HKEY_CURRENT_USER);
}

// DllInstall - 选择注册表范围：
//   regsvr32 /i:user    或默认      -> HKCU\Software\Classes（当前用户）
//   regsvr32 /i:machine            -> HKLM\Software\Classes（所有用户，需要管理员权限）
STDAPI DllInstall(BOOL bInstall, _In_opt_ LPCWSTR pszCmdLine)
{
	const bool machineScope = (pszCmdLine != nullptr) && (wcsstr(pszCmdLine, L"machine") != nullptr);
	const HKEY root = RegistrationRoot(machineScope);

	if (bInstall)
	{
		const HRESULT hr = RegisterAt(root);
		if (FAILED(hr))
		{
			UnregisterAt(root);
		}
		return hr;
	}

	return UnregisterAt(root);
}
