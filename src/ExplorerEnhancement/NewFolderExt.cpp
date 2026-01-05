// NewFolderExt.cpp: CNewFolderExt 类的实现，用于Windows资源管理器的新建文件夹扩展
// 功能：将选中的文件创建到一个新文件夹中，文件夹名基于文件名的共同前缀

#include "pch.h"
#include "NewFolderExt.h"
#include "common.h"

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

	// 加载上下文菜单名称
	if (!m_contextmenu_name.LoadStringW(IDS_MENUITEM))
	{
		m_contextmenu_name = L"New Folder +";
	}

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
	ibindctx = nullptr;
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
	IShellItemArray* psia;
	// 从流中获取ShellItemArray接口并释放流
	HRESULT hr = CoGetInterfaceAndReleaseStream(_pstmShellItemArray, IID_PPV_ARGS(&psia));
	_pstmShellItemArray = NULL;
	if (SUCCEEDED(hr))
	{
		// 执行创建文件夹和移动文件的操作
		DoCreateAndMoveItems(NULL, psia);
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
	// 保存数据对象指针
	m_spdo = pdtobj;
	// 清空选中文件列表
	selected_files.clear();

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
			LPWSTR filename_buffer;
			// 获取当前选中项
			spsia->GetItemAt(i, &current_item);
			// 获取当前项的绝对路径
			current_item->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEPARSING, &filename_buffer);

			ATL::CString item_name(filename_buffer);
			selected_files.push_back(item_name);
			// 释放文件名缓冲区
			CoTaskMemFree(filename_buffer);
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

	// 从注册表获取菜单项启用状态
	DWORD enabled = RegGetDword(HKEY_CURRENT_USER, M_REG_BASEKEY, M_REG_ENABLE_NEWFOLDER);
	if (!enabled)
	{
		// 菜单项未启用，返回空结果
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
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

	// 插入菜单项到菜单开头
	InsertMenuItem(hmenu, 0, TRUE, (LPCMENUITEMINFO)&mii);

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
/// <param name="pici"></param>
/// <param name="psiItemArray"></param>
/// <returns></returns>
HRESULT __stdcall CNewFolderExt::DoCreateAndMoveItems(CMINVOKECOMMANDINFO* pici, IShellItemArray* psiItemArray)
{
	HRESULT hr = E_FAIL;

	ATL::CString target_path;
	ATL::CString common_part = find_common_prefix(selected_files, &target_path);
	{
		// 如果没有共同前导子串，那么以第一个文件的文件名为目录名
		if (common_part.Trim().GetLength() == 0)
		{
			common_part = extract_filename_without_ext(selected_files.at(0));
			/*srand(timeGetTime());
			common_part.Append(L"-");
			common_part.Append(std::to_wstring(rand() * 7 % 100).c_str());*/
		}
	}
	RETURN_IF_FAILED(CoInitialize(NULL));

	CComPtr<IFileOperation> pfo;
	RETURN_IF_FAILED(CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo)));

	// 此处似乎不应该使用pici->hwnd，待考
	HWND hwnd = pici ? pici->hwnd : GetActiveWindow();
	pfo->SetOwnerWindow(hwnd);

	// 设置一下Pfo的标志位，此处不设置也可
	pfo->SetOperationFlags(FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMMKDIR);

	// 要生成的文件夹路径
	CComPtr<IShellItem> pDstFolder;
	ATL::CString path_of_the_new_folder = combine_path(target_path, common_part);

	if (!PathIsDirectoryW(path_of_the_new_folder.GetString()))
	{
		// 如果目标文件夹不存在的话，就创建一个
		//  使用CreateDirectoryW支持UNC路径
		bool rs = CreateDirectoryW(path_of_the_new_folder.GetString(), NULL);
		if (!rs)
		{
			DWORD error = GetLastError();
			// 如果是因为路径不存在（对于UNC路径可能需要递归创建），使用SHCreateDirectoryEx
			if (error == ERROR_PATH_NOT_FOUND)
			{
				rs = SHCreateDirectoryExW(NULL, path_of_the_new_folder.GetString(), NULL) == ERROR_SUCCESS;
			}
			if (!rs)
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}
		// 通知Shell
		SHChangeNotify(SHCNE_MKDIR, SHCNF_PATHW, path_of_the_new_folder.GetString(), NULL);
	}

	// 把目标文件夹创建为IShellItem，给pfo使用
	RETURN_IF_FAILED(SHCreateItemFromParsingName(path_of_the_new_folder.GetString(), NULL, IID_PPV_ARGS(&pDstFolder)));

	if (!psiItemArray)
	{
		for (auto iter = selected_files.begin(); iter != selected_files.end(); iter++)
		{
			// 避免同名文件夹移动
			if ((*iter).CompareNoCase(path_of_the_new_folder) == 0)
			{
				continue;
			}
			CComPtr<IShellItem> pitem;
			hr = SHCreateItemFromParsingName((*iter).GetString(), NULL, IID_PPV_ARGS(&pitem));
			if (SUCCEEDED(hr))
			{
				pfo->MoveItem(pitem, pDstFolder, extract_filename((*iter)).GetString(), NULL);
				/*
				 * 2022/3/18 对于每一个移动的文件，向shell发送通知
				 */
				SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pitem, NULL);
			}
		}
	}
	else
	{
		pfo->MoveItems(psiItemArray, pDstFolder);
	}
	SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_FLUSHNOWAIT, NULL, NULL);
	// 执行pfo操作
	hr = pfo->PerformOperations();
	CoUninitialize();
	selectItem(&path_of_the_new_folder);
	return hr;
}
/// <summary>
/// 以线程方式执行选中并重命名操作，目前大部分情况下可用
/// </summary>
/// <param name="pArguments">threadParam结构</param>
/// <returns></returns>
unsigned __stdcall selectItem(void* pArguments)
{

	RETURN_IF_FAILED(CoInitialize(NULL));
	ATL::CString* folder_to_select = static_cast<ATL::CString*>(pArguments);

	CComPtr<IShellWindows> shellwindow;
	RETURN_IF_FAILED(CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_PPV_ARGS(&shellwindow)));

	long count;
	shellwindow->get_Count(&count);
	for (long i = 0; i < count; i++)
	{
		CComPtr<IDispatch> pdisp;
		CComVariant va(i, VT_I4);
		if (SUCCEEDED(shellwindow->Item(va, &pdisp)) && (!!pdisp))
		{
			CComPtr<IWebBrowserApp> pwba;
			HRESULT h = pdisp->QueryInterface(IID_PPV_ARGS(&pwba));
			if (FAILED(h))
			{
				continue;
			}
			HWND hwba;

			if (SUCCEEDED(pwba->get_HWND((LONG_PTR*)&hwba)))
			{
				CComPtr<IServiceProvider> psp;
				h = pwba->QueryInterface(IID_PPV_ARGS(&psp));
				if (FAILED(h))
				{
					continue;
				}
				CComPtr<IShellBrowser> psb;
				h = psp->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&psb));
				if (FAILED(h))
				{
					continue;
				}
				CComPtr<IShellView> psv;
				h = psb->QueryActiveShellView(&psv);
				if (FAILED(h))
				{
					continue;
				}
				psv->Refresh();
				CComPtr<IFolderView2> pfv;
				h = psv->QueryInterface(IID_PPV_ARGS(&pfv));
				if (FAILED(h))
				{
					continue;
				}
				CComPtr<IShellItemArray> items;
				if (SUCCEEDED(pfv->Items(SVGIO_ALLVIEW, IID_PPV_ARGS(&items))))
				{
					DWORD count = 0;
					items->GetCount(&count);
					for (DWORD i = 0; i < count; i++)
					{
						IShellItem* _item;
						if (SUCCEEDED(items->GetItemAt(i, &_item)))
						{
							LPWSTR filename = NULL;
							if (SUCCEEDED(_item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &filename)))
							{
								ATL::CPath fpath(filename);
								fpath.Canonicalize();
								if (fpath.m_strPath.CompareNoCase(*folder_to_select) == 0)
								{
									try
									{
										if (SUCCEEDED(pfv->SelectItem(i, SVSI_EDIT | SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE | SVSI_POSITIONITEM)))
										{
											// PostMessage(hwba, WM_KEYDOWN, VK_F2, 0);
											CoUninitialize();
											return S_OK;
										}
									}
									catch (const std::exception&)
									{
										CoUninitialize();
										return E_UNEXPECTED;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	CoUninitialize();
	return E_UNEXPECTED;
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
	// 只比较前10个字符串，避免过多计算
	for (size_t i = 1; i < min(10, strs.size()); i++)
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
/// 提取文件名（含扩展名）
/// </summary>
/// <param name="filepath"></param>
/// <returns></returns>
ATL::CString CNewFolderExt::extract_filename(const ATL::CString filepath)
{
	ATL::CPath path(filepath);
	path.StripPath();
	return ATL::CString(path);
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

/// <summary>
/// 将src_path 调整到 new_base_folder文件夹下
/// </summary>
/// <param name="src_path"></param>
/// <param name="new_base_folder"></param>
/// <returns></returns>
ATL::CString CNewFolderExt::rebase_path(ATL::CString src_path, ATL::CString new_base_folder)
{
	ATL::CString filename = extract_filename(src_path);
	ATL::CPath new_path(new_base_folder);
	new_path.Combine(new_path, filename);
	return ATL::CString(new_path);
}

/// <summary>
/// 拼合路径
/// </summary>
/// <param name="folder_name"></param>
/// <param name="suffix"></param>
/// <returns></returns>
ATL::CString CNewFolderExt::combine_path(ATL::CString folder_name, ATL::CString suffix)
{
	ATL::CPath base(folder_name);
	base.Append(suffix);
	return base;
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
			// Convert wide string to char string for the output buffer
			wcstombs_s(nullptr, pszName, cchMax, helpText, _TRUNCATE);
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
			// Convert wide string to char string for the output buffer
			wcstombs_s(nullptr, pszName, cchMax, verb, _TRUNCATE);
			return S_OK;
		}
	}

	return S_OK; // Return S_OK when not handling specific types
}

HRESULT __stdcall CNewFolderExt::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName)
{
	UNREFERENCED_PARAMETER(psiItemArray);
	*ppszName = nullptr;
	auto title = wil::make_cotaskmem_string_nothrow(m_contextmenu_name.GetString());
	RETURN_IF_NULL_ALLOC(title);
	*ppszName = title.release();
	return S_OK;
}

HRESULT __stdcall CNewFolderExt::GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon)
{
	UNREFERENCED_PARAMETER(psiItemArray);
	*ppszIcon = nullptr;
	auto path = wil::make_cotaskmem_string_nothrow(L"%systemroot%\\system32\\shell32.dll, -20");
	RETURN_IF_NULL_ALLOC(path);
	*ppszIcon = path.release();
	return S_OK;
}

HRESULT __stdcall CNewFolderExt::GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip)
{
	// Provide tooltip information for the command
	*ppszInfotip = nullptr;

	// Create tooltip text
	const wchar_t* tooltipText = L"Create a new folder and move selected items into it";

	// Allocate memory for the tooltip string using CoTaskMemAlloc
	LPWSTR tooltip = static_cast<LPWSTR>(CoTaskMemAlloc((wcslen(tooltipText) + 1) * sizeof(wchar_t)));
	if (!tooltip)
	{
		return E_OUTOFMEMORY;
	}

	// Copy the tooltip text
	wcscpy_s(tooltip, wcslen(tooltipText) + 1, tooltipText);

	// Assign the tooltip to the output parameter
	*ppszInfotip = tooltip;

	return S_OK;
}

HRESULT __stdcall CNewFolderExt::GetCanonicalName(GUID* pguidCommandName)
{
	*pguidCommandName = __uuidof(NewFolderExt);
	return S_OK;
}

HRESULT __stdcall CNewFolderExt::GetState(IShellItemArray* psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState)
{
	// Check registry setting to determine if this command should be enabled
	UNREFERENCED_PARAMETER(psiItemArray);
	DWORD enabled = RegGetDword(HKEY_CURRENT_USER, M_REG_BASEKEY, M_REG_ENABLE_NEWFOLDER);
	if (!enabled)
	{
		*pCmdState = ECS_HIDDEN;
	}
	else
	{
		*pCmdState = ECS_ENABLED;
	}
	return S_OK;
}

HRESULT __stdcall CNewFolderExt::Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc)
{
	ibindctx = pbc;
	return DoCreateAndMoveItems(nullptr, psiItemArray);
}

HRESULT __stdcall CNewFolderExt::GetFlags(EXPCMDFLAGS* pFlags)
{
	*pFlags = ECF_DEFAULT;
	return S_OK;
}

HRESULT __stdcall CNewFolderExt::EnumSubCommands(IEnumExplorerCommand** ppEnum)
{
	// This command doesn't have subcommands, so return a null enumerator
	*ppEnum = nullptr;
	return S_OK;  // S_OK with a null enumerator indicates no subcommands
}
