// NewFolderExt.cpp: CNewFolderExt 类的实现，用于Windows资源管理器的新建文件夹扩展
// 功能：将选中的文件创建到一个新文件夹中，文件夹名基于文件名的共同前缀

#include "pch.h"
#include "NewFolderExt.h"
#include "common.h"
#include "../Common/LocalizedString.h"

namespace
{
ATL::CString CombineFolderPath(const ATL::CString& parent, const ATL::CString& name)
{
	ATL::CPath path(parent);
	path.Append(name);
	return ATL::CString(path);
}

// 目标名被已存在的文件占用时（无扩展名的文件很容易出现），退到“<名称> folder”并加序号；
// 已存在同名文件夹则直接复用（原有行为）。
ATL::CString MakeUniqueFolderPath(const ATL::CString& parent, const ATL::CString& baseName)
{
	ATL::CString folder = CombineFolderPath(parent, baseName);
	if (!PathFileExistsW(folder))
	{
		return folder;
	}
	if (PathIsDirectoryW(folder))
	{
		return folder;
	}

	ATL::CString candidateName = baseName + L" folder";
	folder = CombineFolderPath(parent, candidateName);
	for (int index = 2; index < 100 && PathFileExistsW(folder) && !PathIsDirectoryW(folder); ++index)
	{
		ATL::CString numbered;
		numbered.Format(L"%s (%d)", candidateName.GetString(), index);
		folder = CombineFolderPath(parent, numbered);
	}
	return folder;
}
} // namespace

// CNewFolderExt
// extern HINSTANCE g_hInstance;

/**
 * @brief 构造函数，初始化上下文菜单扩展
 */
CNewFolderExt::CNewFolderExt()
{
	_pstmShellItemArray = nullptr; // 线程间ShellItemArray流指针
	spsia = nullptr;			   // Shell项目数组指针
	selected_files.clear();		   // 选中文件列表

	// 加载上下文菜单名称（按用户界面语言，中文 Windows 显示中文）
	m_contextmenu_name = LoadLocalizedString(IDS_MENUITEM, L"New Folder +");

	// 加载菜单图标
	if (!m_icon)
	{
		HINSTANCE hShell32 = GetModuleHandle(L"Shell32.dll"); // 获取shell32.dll模块句柄
		// 从shell32.dll加载图标资源
		HICON hIcon = (HICON)LoadImage(hShell32, MAKEINTRESOURCE(20), IMAGE_ICON, GetSystemMetrics(SM_CXMENUCHECK), GetSystemMetrics(SM_CYMENUCHECK), LR_LOADTRANSPARENT | LR_SHARED);
		SIZE t;
		t.cx = GetSystemMetrics(SM_CXMENUCHECK); // 菜单图标宽度
		t.cy = GetSystemMetrics(SM_CYMENUCHECK); // 菜单图标高度
		m_icon = IconToBitmap(hIcon, &t);		 // 将图标转换为位图
	}
}

/**
 * @brief 析构函数，释放资源
 */
CNewFolderExt::~CNewFolderExt()
{
	// 释放图标资源
	if (m_icon)
	{
		DeleteObject(m_icon);
		m_icon = nullptr;
	}

	// 清空选中文件列表
	selected_files.clear();

	// 释放智能指针和COM对象
	m_spdo = nullptr;
	if (spsia)
	{
		spsia->Release();
		spsia = nullptr;
	}
	if (_pstmShellItemArray)
	{
		_pstmShellItemArray->Release();
		_pstmShellItemArray = nullptr;
	}
}

/**
 * @brief 线程处理函数，执行创建文件夹和移动文件的操作
 * @return 线程退出代码
 */
DWORD CNewFolderExt::_ThreadProc()
{
	IShellItemArray* psia = nullptr;
	// 从流中获取ShellItemArray接口并释放流
	HRESULT hr = CoGetInterfaceAndReleaseStream(_pstmShellItemArray, IID_PPV_ARGS(&psia));
	_pstmShellItemArray = NULL;
	if (SUCCEEDED(hr))
	{
		// 执行创建文件夹和移动文件的操作
		DoCreateAndMoveItems(psia);
		psia->Release(); // 释放ShellItemArray接口
	}

	return 0;
}

/**
 * @brief 静态线程处理函数，用于创建线程并调用成员线程处理函数
 * @param pv 线程参数，指向CNewFolderExt实例
 * @return 线程退出代码
 */
DWORD __stdcall CNewFolderExt::s_ThreadProc(void* pv)
{
	// 将参数转换为CNewFolderExt实例指针
	CNewFolderExt* pecv = (CNewFolderExt*)pv;
	// 调用成员线程处理函数
	const DWORD ret = pecv->_ThreadProc();
	// 释放实例引用计数
	pecv->Release();
	return ret;
}

/**
 * @brief 初始化上下文菜单扩展
 * @param pidlFolder 当前文件夹的绝对PIDL
 * @param pdtobj 包含选中项目的数据对象
 * @param hkeyProgID 程序ID的注册表项
 * @return S_OK 初始化成功，否则返回错误码
 */
HRESULT __stdcall CNewFolderExt::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID)
{
	// 管理器的“启用”开关：停用时返回失败，右键菜单不显示
	if (!IsExtensionEnabled(M_REG_ENABLE_NEWFOLDER))
	{
		return E_INVALIDARG;
	}

	// 保存数据对象指针
	m_spdo = pdtobj;
	// 清空选中文件列表
	selected_files.clear();
	// 释放上次初始化留下的ShellItemArray
	if (spsia)
	{
		spsia->Release();
		spsia = nullptr;
	}

	// 检查是否存在可移动的文件，如果只存在一项且该项为文件夹，则不显示
	if (SUCCEEDED(GetShellItemArrayFromDataObject(m_spdo, &spsia)))
	{
		DWORD file_count = 0;
		// 获取选中项目数量
		spsia->GetCount(&file_count);
		// 将选中的文件路径存入vector
		for (DWORD i = 0; i < file_count; i++)
		{
			CComPtr<IShellItem> current_item;
			LPWSTR filename_buffer = nullptr;
			// 获取当前选中项
			if (FAILED(spsia->GetItemAt(i, &current_item)) || !current_item)
			{
				continue;
			}
			// 获取当前项的绝对路径
			if (SUCCEEDED(current_item->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEPARSING, &filename_buffer)) && filename_buffer)
			{
				selected_files.emplace_back(filename_buffer);
			}
			// 释放文件名缓冲区
			CoTaskMemFree(filename_buffer);
		}

		// 没有可处理的项时隐藏菜单项
		if (selected_files.empty())
		{
			return E_FAIL;
		}

		// 如果只选中了一个文件夹，不显示菜单项；只选中一个文件则可以显示
		if (selected_files.size() == 1)
		{
			ATL::CPath onePath(selected_files.at(0));
			if (onePath.IsDirectory())
			{
				return E_INVALIDARG;
			}
		}
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

/**
 * @brief 从DataObject中获取ShellItemArray
 * @param dataSource 数据源对象
 * @param items 返回的ShellItemArray指针
 * @return S_OK 获取成功，否则返回错误码
 */
HRESULT __stdcall CNewFolderExt::GetShellItemArrayFromDataObject(_In_ IUnknown* dataSource, _COM_Outptr_ IShellItemArray** items)
{
	*items = nullptr; // 初始化返回值
	CComPtr<IDataObject> dataObj;
	HRESULT hr;

	// 尝试从数据源获取IDataObject接口
	if (SUCCEEDED(dataSource->QueryInterface(IID_PPV_ARGS(&dataObj))))
	{
		// 从DataObject创建ShellItemArray
		hr = SHCreateShellItemArrayFromDataObject(dataObj, IID_PPV_ARGS(items));
	}
	else
	{
		// 直接从数据源查询ShellItemArray接口
		hr = dataSource->QueryInterface(IID_PPV_ARGS(items));
	}
	return hr;
}

/**
 * @brief 添加上下文菜单项到资源管理器菜单
 * @param hmenu 目标菜单句柄
 * @param indexMenu 菜单项插入位置
 * @param idCmdFirst 可用的第一个命令ID
 * @param idCmdLast 可用的最后一个命令ID
 * @param uFlags 菜单标志
 * @return 成功返回包含菜单项数量的HRESULT，失败返回错误码
 */
HRESULT __stdcall CNewFolderExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	// 如果是默认菜单项，返回失败
	if (uFlags & CMF_DEFAULTONLY)
	{
		return E_FAIL;
	}

	// 检查命令ID范围是否有效
	if (idCmdFirst >= idCmdLast)
	{
		return E_INVALIDARG;
	}

	// 构建菜单项信息结构
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID; // 菜单项包含位图、字符串、类型和ID
	mii.fType = MFT_STRING;										  // 字符串类型菜单项
	mii.dwTypeData = m_contextmenu_name.GetBuffer();			  // 菜单项文本
	mii.wID = idCmdFirst;										  // 菜单项命令ID
	mii.hbmpItem = m_icon;										  // 菜单项图标

	// 插入菜单项到指定位置
	InsertMenuItem(hmenu, indexMenu, TRUE, (LPCMENUITEMINFO)&mii);

	// 释放缓冲区
	m_contextmenu_name.ReleaseBuffer();

	// 返回成功，包含添加的菜单项数量
	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 1);
}

/**
 * @brief 处理上下文菜单项的点击事件
 * @param pici 命令调用信息
 * @return S_OK 命令执行成功，否则返回错误码
 */
HRESULT __stdcall CNewFolderExt::InvokeCommand(CMINVOKECOMMANDINFO* pici)
{
	// 检查命令是否有效
	if ((IS_INTRESOURCE(pici->lpVerb)) && (LOWORD(pici->lpVerb) == 0))
	{
		// 将ShellItemArray接口封送到流中，以便在新线程中使用
		HRESULT hr = CoMarshalInterThreadInterfaceInStream(__uuidof(spsia), spsia, &_pstmShellItemArray);
		if (SUCCEEDED(hr))
		{
			// 增加引用计数，防止对象在线程执行前被销毁
			AddRef();
			// 创建新线程执行创建文件夹和移动文件的操作
			if (!SHCreateThread(s_ThreadProc, this, CTF_COINIT_STA | CTF_PROCESS_REF, NULL))
			{
				// 线程创建失败，释放引用计数
				Release();
			}
		}
		return S_OK;
	}
	else
	{
		// 无效命令，返回失败
		return E_FAIL;
	}
}

/// <summary>
/// 执行创建文件夹并将选择项移动到文件夹内的操作
/// </summary>
/// <param name="psiItemArray">选中的Shell项目数组</param>
/// <returns>HRESULT 操作结果</returns>
HRESULT __stdcall CNewFolderExt::DoCreateAndMoveItems(IShellItemArray* psiItemArray)
{
	HRESULT hr = E_FAIL;

	// 检查输入参数
	if (!psiItemArray || selected_files.empty()) {
		return E_INVALIDARG; // 没有选中的文件
	}

	ATL::CString target_path;
	ATL::CString common_part = find_common_prefix(selected_files, &target_path);
	{
		// 如果没有共同前导子串，那么以第一个文件的文件名为目录名
		if (common_part.Trim().GetLength() == 0)
		{
			common_part = extract_filename_without_ext(selected_files.at(0));
		}
	}

	CComPtr<IFileOperation> pfo;
	hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
	if (FAILED(hr)) {
		return hr;
	}

	HWND hwnd = GetActiveWindow();
	hr = pfo->SetOwnerWindow(hwnd);
	if (FAILED(hr)) {
		return hr;
	}

	// 设置一下Pfo的标志位，此处不设置也可
	hr = pfo->SetOperationFlags(FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMMKDIR);
	if (FAILED(hr)) {
		return hr;
	}

	// 要生成的文件夹路径；同名文件占用目标名时改成不冲突的名字
	CComPtr<IShellItem> pDstFolder;
	ATL::CString path_of_the_new_folder = MakeUniqueFolderPath(target_path, common_part);
	if (path_of_the_new_folder.IsEmpty())
	{
		return E_INVALIDARG;
	}

	if (!PathIsDirectoryW(path_of_the_new_folder.GetString()))
	{
		// 如果目标文件夹不存在的话，就创建一个
		// 使用CreateDirectoryW支持UNC路径
		DWORD create_error = ERROR_SUCCESS;
		if (!CreateDirectoryW(path_of_the_new_folder.GetString(), NULL))
		{
			create_error = GetLastError();
			// 如果是因为路径不存在（对于UNC路径可能需要递归创建），使用SHCreateDirectoryEx
			if (create_error == ERROR_PATH_NOT_FOUND)
			{
				// 该函数直接返回错误码，不能再用GetLastError
				create_error = static_cast<DWORD>(SHCreateDirectoryExW(NULL, path_of_the_new_folder.GetString(), NULL));
			}
		}
		if (create_error != ERROR_SUCCESS)
		{
			ShowOperationError(hwnd, HRESULT_FROM_WIN32(create_error), L"无法创建目标文件夹");
			return HRESULT_FROM_WIN32(create_error);
		}
		// 通知Shell
		SHChangeNotify(SHCNE_MKDIR, SHCNF_PATHW, path_of_the_new_folder.GetString(), NULL);
	}

	// 把目标文件夹创建为IShellItem，给pfo使用
	hr = SHCreateItemFromParsingName(path_of_the_new_folder.GetString(), NULL, IID_PPV_ARGS(&pDstFolder));
	if (FAILED(hr)) {
		ShowOperationError(hwnd, hr, L"无法打开目标文件夹");
		return hr;
	}

	hr = pfo->MoveItems(psiItemArray, pDstFolder);
	if (FAILED(hr)) {
		ShowOperationError(hwnd, hr, L"无法移动选中的项目");
		return hr;
	}

	// 执行pfo操作
	hr = pfo->PerformOperations();
	// 通知所有事件
	SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_FLUSHNOWAIT, NULL, NULL);
	if (FAILED(hr)) {
		ShowOperationError(hwnd, hr, L"文件操作失败");
		return hr;
	}

	// 异步调用selectItem，线程负责释放路径字符串
	ATL::CString* folder_to_select = new ATL::CString(path_of_the_new_folder);
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, selectItem, folder_to_select, 0, NULL);
	if (hThread) {
		CloseHandle(hThread); // 不需要等待线程完成，只是启动它
	}
	else {
		delete folder_to_select;
	}

	return S_OK;
}
/// <summary>
/// 以线程方式选中新创建的文件夹
/// </summary>
/// <param name="pArguments">新文件夹路径（CString*，由本函数释放）</param>
/// <returns></returns>
unsigned __stdcall selectItem(void* pArguments)
{
	// 先复制并释放调用方分配的路径，避免线程未及时启动时泄漏
	ATL::CString* path = static_cast<ATL::CString*>(pArguments);
	if (!path) {
		return 0; // 参数为空指针
	}
	ATL::CString folder_to_select = *path;
	delete path;

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		return static_cast<unsigned>(hr);
	}

	CComPtr<IShellWindows> shellwindow;
	hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_PPV_ARGS(&shellwindow));
	if (FAILED(hr)) {
		CoUninitialize();
		return static_cast<unsigned>(hr);
	}

	long count = 0;
	hr = shellwindow->get_Count(&count);
	if (FAILED(hr)) {
		CoUninitialize();
		return static_cast<unsigned>(hr);
	}

	for (long i = 0; i < count; i++)
	{
		CComPtr<IDispatch> pdisp;
		CComVariant va(i, VT_I4);
		hr = shellwindow->Item(va, &pdisp);
		if (FAILED(hr) || !pdisp)
		{
			continue;
		}

		CComPtr<IWebBrowserApp> pwba;
		hr = pdisp->QueryInterface(IID_PPV_ARGS(&pwba));
		if (FAILED(hr))
		{
			continue;
		}

		HWND hwba;
		hr = pwba->get_HWND((LONG_PTR*)&hwba);
		if (FAILED(hr))
		{
			continue;
		}

		CComPtr<IServiceProvider> psp;
		hr = pwba->QueryInterface(IID_PPV_ARGS(&psp));
		if (FAILED(hr))
		{
			continue;
		}

		CComPtr<IShellBrowser> psb;
		hr = psp->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&psb));
		if (FAILED(hr))
		{
			continue;
		}

		CComPtr<IShellView> psv;
		hr = psb->QueryActiveShellView(&psv);
		if (FAILED(hr))
		{
			continue;
		}

		psv->Refresh();

		CComPtr<IFolderView2> pfv;
		hr = psv->QueryInterface(IID_PPV_ARGS(&pfv));
		if (FAILED(hr))
		{
			continue;
		}

		CComPtr<IShellItemArray> items;
		hr = pfv->Items(SVGIO_ALLVIEW, IID_PPV_ARGS(&items));
		if (FAILED(hr) || !items)
		{
			continue;
		}

		DWORD itemCount = 0;
		hr = items->GetCount(&itemCount);
		if (FAILED(hr))
		{
			continue;
		}

		for (DWORD j = 0; j < itemCount; j++)
		{
			CComPtr<IShellItem> item;
			hr = items->GetItemAt(j, &item);
			if (FAILED(hr) || !item)
			{
				continue;
			}

			LPWSTR filename = nullptr;
			hr = item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &filename);
			if (FAILED(hr) || !filename)
			{
				// 释放获取的文件名
				CoTaskMemFree(filename);
				continue;
			}

			ATL::CPath fpath(filename);
			fpath.Canonicalize();
			const bool matched = fpath.m_strPath.CompareNoCase(folder_to_select) == 0;
			// 释放获取的文件名
			CoTaskMemFree(filename);
			if (!matched)
			{
				continue;
			}

			hr = pfv->SelectItem(j, SVSI_EDIT | SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE | SVSI_POSITIONITEM);
			CoUninitialize();
			return SUCCEEDED(hr) ? 0u : static_cast<unsigned>(hr);
		}
	}

	CoUninitialize();
	return static_cast<unsigned>(E_UNEXPECTED);
}

/// <summary>
/// 查找共同前导子串的长度（共同前缀）
/// </summary>
/// <param name="str1"></param>
/// <param name="str2"></param>
/// <returns></returns>
int CNewFolderExt::first_common_substring_length(const ATL::CString& str1, const ATL::CString& str2, int length_to_compare)
{
	/*
	 * 2022/3/18 重构，增加length_to_compare参数，用于提高处理速度。
	 */
	if (length_to_compare < 0) {
		return 0; // 边界条件检查
	}

	int l = min(str1.GetLength(), str2.GetLength());
	l = min(l, length_to_compare);
	int result = l;
	for (int i = 0; i < l; i++)
	{
		if (str1[i] != str2[i])
		{
			result = i;
			break;
		}
	}
	return result;
}

/// <summary>
/// 求一组字符串的最长公共前导子串
/// </summary>
/// <param name="strs"></param>
/// <returns></returns>
ATL::CString CNewFolderExt::longestCommonPrefix(const std::vector<ATL::CString>& strs)
{
	/*
	 * 2022/3/18 重构，增加length_to_compare参数，用于提高处理速度。
	 */
	if (strs.empty())
	{
		return ATL::CString("");
	}

	ATL::CString base = strs[0];
	int common_prefix_length = base.GetLength();
	for (size_t i = 1; i < strs.size(); i++)
	{
		common_prefix_length = first_common_substring_length(base, strs[i], common_prefix_length);
	}
	if (common_prefix_length == 0)
	{
		return ATL::CString("");
	}
	else
	{
		return base.Left(common_prefix_length);
	}
}

/// <summary>
/// 查找一系列文件的共同前导子串
/// </summary>
/// <param name="filepaths"></param>
/// <param name="default_path">新文件夹创建时的基目录</param>
/// <returns></returns>
ATL::CString CNewFolderExt::find_common_prefix(const std::vector<ATL::CString> filepaths, OUT ATL::CString* default_path)
{
	if (filepaths.empty())
	{
		return ATL::CString("");
	}

	ATL::CPath base_folder(filepaths.at(0));
	base_folder.RemoveFileSpec();
	*default_path = ATL::CString(base_folder);

	std::vector<ATL::CString> filenames;

	for (auto iter = filepaths.begin(); iter != filepaths.end(); iter++)
	{
		ATL::CString _filename = extract_filename_without_ext(*iter);
		filenames.push_back(_filename);
	}

	ATL::CString common_part = longestCommonPrefix(filenames);

	return common_part.Trim();
}

/// <summary>
/// 提取文件名（不含扩展名）
/// </summary>
/// <param name="filepath"></param>
/// <returns></returns>
ATL::CString CNewFolderExt::extract_filename_without_ext(const ATL::CString filepath)
{
	ATL::CPath path(filepath);
	path.StripPath();
	path.RemoveExtension();
	return path;
}

HRESULT __stdcall CNewFolderExt::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, CHAR* pszName, UINT cchMax)
{
	// Only handle command help text (verb help)
	if (uType == GCS_HELPTEXTA)
	{
		// Provide help text for the command
		const char* helpText = "Create a new folder with selected items";
		if (pszName && cchMax > 0)
		{
			strncpy_s(pszName, cchMax, helpText, _TRUNCATE);
			return S_OK;
		}
	}
	else if (uType == GCS_HELPTEXTW)
	{
		// Provide help text for the command (wide character version)
		const wchar_t* helpText = L"Create a new folder with selected items";
		if (pszName && cchMax > 0)
		{
			wcscpy_s(reinterpret_cast<LPWSTR>(pszName), cchMax, helpText);
			return S_OK;
		}
	}
	else if (uType == GCS_VERBA)
	{
		// Provide the verb string (command name)
		const char* verb = "NewFolderPlus";
		if (pszName && cchMax > 0)
		{
			strncpy_s(pszName, cchMax, verb, _TRUNCATE);
			return S_OK;
		}
	}
	else if (uType == GCS_VERBW)
	{
		// Provide the verb string (wide character version)
		const wchar_t* verb = L"NewFolderPlus";
		if (pszName && cchMax > 0)
		{
			wcscpy_s(reinterpret_cast<LPWSTR>(pszName), cchMax, verb);
			return S_OK;
		}
	}
	else
	{
		// 不支持的类型，返回错误
		return E_INVALIDARG;
	}

	return S_OK; // Return S_OK when not handling specific types
}
