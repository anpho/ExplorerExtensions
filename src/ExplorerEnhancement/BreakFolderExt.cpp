// BreakFolderExt.cpp: CBreakFolderExt 类的实现，用于Windows资源管理器的文件夹操作扩展
// 功能：将选中文件夹中的所有文件移动到它们的父文件夹中

#include "pch.h"
#include "BreakFolderExt.h"
#include "common.h"

// CBreakFolderExt 类实现

/**
 * @brief 初始化上下文菜单扩展
 * @param pidlFolder 当前文件夹的绝对PIDL
 * @param pdtobj 包含选中项目的数据对象
 * @param hkeyProgID 程序ID的注册表项
 * @return S_OK 初始化成功，否则返回错误码
 */
HRESULT __stdcall CBreakFolderExt::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
	// 管理器的“启用”开关：停用时返回失败，右键菜单不显示
	if (!IsExtensionEnabled(M_REG_ENABLE_BREAKFOLDER))
	{
		return E_INVALIDARG;
	}

	// 输入验证
	if (!pdtobj)
	{
		return E_INVALIDARG;
	}

	m_folders.clear();

	CComPtr<IShellItemArray> items;
	HRESULT hr = SHCreateShellItemArrayFromDataObject(pdtobj, IID_PPV_ARGS(&items));
	if (FAILED(hr) || !items)
	{
		// 某些宿主视图下拿不到数组，兜底直接查询一次
		hr = pdtobj->QueryInterface(IID_PPV_ARGS(&items));
		if (FAILED(hr) || !items)
		{
			return hr;
		}
	}

	// 获取选中文件的数量
	DWORD file_count = 0;
	hr = items->GetCount(&file_count);
	if (FAILED(hr))
	{
		return hr;
	}

	// 只保留文件夹项，文件项跳过
	for (DWORD i = 0; i < file_count; i++)
	{
		CComPtr<IShellItem> cur_item; // 当前Shell项目
		hr = items->GetItemAt(i, &cur_item);
		if (FAILED(hr) || !cur_item)
		{
			continue;
		}

		SFGAOF sfgaoFlags = SFGAO_FOLDER;
		hr = cur_item->GetAttributes(SFGAO_FOLDER, &sfgaoFlags);
		// 取属性失败时也按文件夹处理：本扩展只注册在 Directory 上
		if (FAILED(hr) || (sfgaoFlags & SFGAO_FOLDER))
		{
			m_folders.push_back(cur_item);
		}
	}

	// 没有文件夹项时隐藏菜单
	return m_folders.empty() ? E_INVALIDARG : S_OK;
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
HRESULT __stdcall CBreakFolderExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{

	// 如果是默认菜单项，返回失败
	if (uFlags & CMF_DEFAULTONLY)
		return E_FAIL;

	// 如果图标未加载，加载图标
	if (!m_hBitmap)
	{
		loadIcon();
	}

	// 创建菜单项信息结构
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_BITMAP;
	mii.fType = MFT_STRING;
	mii.hbmpItem = m_hBitmap;
	BSTR menu_name = m_contextMenuName.AllocSysString();
	mii.dwTypeData = menu_name;
	mii.wID = idCmdFirst++;
	InsertMenuItem(hmenu, indexMenu, TRUE, (LPCMENUITEMINFO)&mii);
	SysFreeString(menu_name);

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 1);
}

/**
 * @brief 处理上下文菜单项的点击事件
 * @param pici 命令调用信息
 * @return S_OK 命令执行成功，否则返回错误码
 */
HRESULT __stdcall CBreakFolderExt::InvokeCommand(CMINVOKECOMMANDINFO *pici)
{
	if ((IS_INTRESOURCE(pici->lpVerb)) && (LOWORD(pici->lpVerb) == 0))
	{
		return OnBreakFoldersTriggered();
	}
	else
	{
		return E_FAIL;
	}
}

/**
 * @brief 获取命令的字符串表示
 * @param idCmd 命令ID
 * @param uType 字符串类型
 * @param pReserved 保留参数
 * @param pszName 字符串缓冲区
 * @param cchMax 缓冲区大小
 * @return S_OK 操作成功，否则返回错误码
 */
HRESULT __stdcall CBreakFolderExt::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pReserved, CHAR *pszName, UINT cchMax)
{
	// Only handle command help text (verb help)
	if (uType == GCS_HELPTEXTA)
	{
		// Provide help text for the command
		const char* helpText = "Break folders and move files to parent folder";
		if (pszName && cchMax > 0)
		{
			strncpy_s(pszName, cchMax, helpText, _TRUNCATE);
			return S_OK;
		}
	}
	else if (uType == GCS_HELPTEXTW)
	{
		// Provide help text for the command (wide character version)
		const wchar_t* helpText = L"Break folders and move files to parent folder";
		if (pszName && cchMax > 0)
		{
			wcscpy_s(reinterpret_cast<LPWSTR>(pszName), cchMax, helpText);
			return S_OK;
		}
	}
	else if (uType == GCS_VERBA)
	{
		// Provide the verb string (command name)
		const char* verb = "BreakFoldersPlus";
		if (pszName && cchMax > 0)
		{
			strncpy_s(pszName, cchMax, verb, _TRUNCATE);
			return S_OK;
		}
	}
	else if (uType == GCS_VERBW)
	{
		// Provide the verb string (wide character version)
		const wchar_t* verb = L"BreakFoldersPlus";
		if (pszName && cchMax > 0)
		{
			wcscpy_s(reinterpret_cast<LPWSTR>(pszName), cchMax, verb);
			return S_OK;
		}
	}

	return S_OK; // Return S_OK when not handling specific types
}

/**
 * @brief 执行文件夹打破操作的核心逻辑
 * 将每个选中文件夹中的全部文件（含子文件夹中的文件）移动到该文件夹自己的父文件夹
 * @return S_OK 操作成功，否则返回错误码
 */
HRESULT CBreakFolderExt::OnBreakFoldersTriggered()
{
	// 检查是否有选中的文件夹
	if (m_folders.empty())
	{
		return E_INVALIDARG;
	}

	CComPtr<IFileOperation> pfo;
	HRESULT hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
	if (FAILED(hr))
	{
		return hr;
	}

	HWND hwnd = GetActiveWindow(); // 获取活动窗口句柄
	pfo->SetOwnerWindow(hwnd);	   // 设置文件操作的所有者窗口
	// 设置文件操作标志：允许撤销、简单进度、创建文件夹时不确认
	pfo->SetOperationFlags(FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMMKDIR);

	HRESULT move_hr = S_OK;
	UINT failed_count = 0;
	UINT queued_count = 0;

	// 每个文件夹的目标是它自己的父目录，多选不同目录时也正确
	for (const auto &folder : m_folders)
	{
		CComPtr<IShellItem> dest_folder;
		if (FAILED(folder->GetParent(&dest_folder)) || !dest_folder)
		{
			if (SUCCEEDED(move_hr))
			{
				move_hr = E_FAIL;
			}
			failed_count++;
			continue;
		}

		// 迭代遍历整棵目录树，收集所有文件（空文件夹保留在原处）
		std::vector<CComPtr<IShellItem>> pending;
		std::vector<CComPtr<IShellItem>> files;
		pending.push_back(folder);
		while (!pending.empty())
		{
			CComPtr<IShellItem> current = pending.back();
			pending.pop_back();

			CComPtr<IEnumShellItems> enumerator;
			hr = current->BindToHandler(NULL, BHID_EnumItems, IID_PPV_ARGS(&enumerator));
			if (FAILED(hr))
			{
				continue;
			}

			for (;;)
			{
				CComPtr<IShellItem> child;
				if (enumerator->Next(1, &child, NULL) != S_OK)
				{
					break;
				}

				SFGAOF sfgaoFlags = SFGAO_FOLDER;
				hr = child->GetAttributes(SFGAO_FOLDER, &sfgaoFlags);
				if (FAILED(hr))
				{
					continue;
				}

				if (sfgaoFlags & SFGAO_FOLDER)
				{
					pending.push_back(child);
				}
				else
				{
					files.push_back(child);
				}
			}
		}

		for (const auto &file : files)
		{
			// 新文件名传 nullptr，保持原文件名
			hr = pfo->MoveItem(file, dest_folder, NULL, NULL);
			if (FAILED(hr))
			{
				if (SUCCEEDED(move_hr))
				{
					move_hr = hr;
				}
				failed_count++;
			}
			else
			{
				queued_count++;
			}
		}
	}

	// 没有文件可移动时不做空操作（空 IFileOperation 会报错并弹窗）
	if (queued_count == 0 && failed_count == 0)
	{
		return S_OK;
	}

	// 执行所有文件操作
	hr = pfo->PerformOperations();
	BOOL aborted = FALSE;
	HRESULT abort_hr = pfo->GetAnyOperationsAborted(&aborted);
	// 通知Shell刷新所有事件
	SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_FLUSHNOWAIT, NULL, NULL);

	if (failed_count > 0 || FAILED(hr) || FAILED(abort_hr) || aborted)
	{
		HRESULT result = E_ABORT;
		if (FAILED(hr))
		{
			result = hr;
		}
		else if (FAILED(move_hr))
		{
			result = move_hr;
		}
		else if (FAILED(abort_hr))
		{
			result = abort_hr;
		}
		ShowOperationError(hwnd, result, L"部分或全部文件未能移动");
		return result;
	}
	return S_OK;
}

/**
 * @brief 加载上下文菜单项的图标
 * 从shell32.dll加载内置图标并转换为位图
 */
void CBreakFolderExt::loadIcon()
{
	// 获取shell32.dll模块句柄
	HINSTANCE hShell32 = GetModuleHandle(L"shell32.dll");
	// 从shell32.dll加载图标资源（ID为46）
	m_hIcon = (HICON)LoadImage(
		hShell32,
		MAKEINTRESOURCE(46),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXMENUCHECK),		  // 菜单图标宽度
		GetSystemMetrics(SM_CYMENUCHECK),		  // 菜单图标高度
		LR_LOADTRANSPARENT | LR_SHARED | LR_COLOR // 加载标志：透明、共享、彩色
	);
	// 将图标转换为位图，用于菜单项
	m_hBitmap = IconToBitmap(m_hIcon);
}