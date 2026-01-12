// BreakFolderExt.cpp: CBreakFolderExt 类的实现，用于Windows资源管理器的文件夹操作扩展
// 功能：将选中文件夹中的所有文件移动到它们的父文件夹中

#include "pch.h"
#include "BreakFolderExt.h"
#include "common.h"
#include <atlpath.h> // 用于 ATL::CPath 类

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
	// 输入验证
	if (!pdtobj)
	{
		return E_INVALIDARG;
	}

	m_selectedFolders.clear();
	CComPtr<IShellItemArray> items;
	CComPtr<IDataObject> dataObj;

	HRESULT hr = pdtobj->QueryInterface(IID_PPV_ARGS(&dataObj));
	if (SUCCEEDED(hr))
	{
		hr = SHCreateShellItemArrayFromDataObject(pdtobj, IID_PPV_ARGS(&items));
		if (FAILED(hr))
		{
			return hr;
		}
	}
	else
	{
		// 如果不能从数据对象创建ShellItemArray，尝试直接查询
		hr = pdtobj->QueryInterface(IID_PPV_ARGS(&items));
		if (FAILED(hr))
		{
			return hr;
		}
	}

	if (SUCCEEDED(hr))
	{
		// 获取选中文件的数量
		DWORD file_count = 0;
		hr = items->GetCount(&file_count);
		if (FAILED(hr))
		{
			return hr;
		}

		// 限制处理的文件夹数量，防止过多操作
		const DWORD MAX_FOLDERS_TO_PROCESS = 100;
		DWORD processed_count = 0;

		// 遍历所有选中的项目，保存文件夹到m_selectedFolders
		for (DWORD i = 0; i < file_count && processed_count < MAX_FOLDERS_TO_PROCESS; i++)
		{
			CComPtr<IShellItem> cur_item;  // 当前Shell项目
			LPWSTR filename_buf = nullptr; // 文件名缓冲区

			// 获取当前项目
			hr = items->GetItemAt(i, &cur_item);
			if (FAILED(hr))
			{
				continue;
			}

			// 获取项目的绝对路径
			hr = cur_item->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEPARSING, &filename_buf);
			if (SUCCEEDED(hr) && filename_buf)
			{
				// 验证路径的有效性
				if (filename_buf[0] == L'\0')
				{
					CoTaskMemFree(filename_buf);
					continue;
				}

				// 检查路径长度，防止过长路径
				if (wcslen(filename_buf) >= MAX_PATH)
				{
					CoTaskMemFree(filename_buf);
					continue;
				}

				// 检查是否为文件夹
				if (PathIsDirectoryW(filename_buf))
				{
					// 额外验证路径是否有效
					WIN32_FIND_DATAW findData;
					HANDLE hFind = FindFirstFileW(filename_buf, &findData);
					if (hFind != INVALID_HANDLE_VALUE)
					{
						FindClose(hFind);
						m_selectedFolders.push_back(filename_buf);
						processed_count++;
					}
				}
				// 释放文件名缓冲区
				CoTaskMemFree(filename_buf);
			}
		}
		return S_OK;
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
HRESULT __stdcall CBreakFolderExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{

	// 如果是默认菜单项，返回失败
	if (uFlags & CMF_DEFAULTONLY)
		return E_FAIL;

	// 菜单项启用状态
	bool enabled = true;
	if (!enabled)
	{
		// 菜单项未启用，返回空结果
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

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
	mii.dwTypeData = m_contextMenuName.AllocSysString();
	mii.wID = idCmdFirst++;
	InsertMenuItem(hmenu, 0, TRUE, (LPCMENUITEMINFO)&mii);

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
			// Convert wide string to char string for the output buffer
			wcstombs_s(nullptr, pszName, cchMax, helpText, _TRUNCATE);
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
			// Convert wide string to char string for the output buffer
			wcstombs_s(nullptr, pszName, cchMax, verb, _TRUNCATE);
			return S_OK;
		}
	}

	return S_OK; // Return S_OK when not handling specific types
}

/**
 * @brief 执行文件夹打破操作的核心逻辑
 * 将选中文件夹中的所有文件移动到它们的父文件夹
 * @return S_OK 操作成功，否则返回错误码
 */
HRESULT CBreakFolderExt::OnBreakFoldersTriggered()
{
	CComPtr<IFileOperation> pfo;
	HRESULT hr;
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo))))
	{
		HWND hwnd = GetActiveWindow(); // 获取活动窗口句柄
		pfo->SetOwnerWindow(hwnd);	   // 设置文件操作的所有者窗口
		// 设置文件操作标志：允许撤销、简单进度、创建文件夹时不确认
		pfo->SetOperationFlags(FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMMKDIR);

		// 检查是否有选中的文件夹
		if (m_selectedFolders.empty())
		{
			return E_INVALIDARG;
		}

		// 获取第一个选中文件夹的父路径作为目标路径
		ATL::CPath basePath(m_selectedFolders[0].c_str());
		basePath.RemoveFileSpec(); // 获取父路径

		CComPtr<IShellItem> pDstDir; // 目标文件夹Shell项目

		// 从路径创建目标文件夹Shell项目
		hr = SHCreateItemFromParsingName(basePath, NULL, IID_PPV_ARGS(&pDstDir));
		if (FAILED(hr))
			return hr;

		// 遍历所有选中的文件夹
		for (const auto &folderPath : m_selectedFolders)
		{
			// 使用Shell API递归遍历文件夹
			CComPtr<IShellItem> pFolder;
			hr = SHCreateItemFromParsingName(folderPath.c_str(), NULL, IID_PPV_ARGS(&pFolder));
			if (FAILED(hr))
				continue;

			// 使用IEnumShellItems递归遍历文件夹内容
			CComPtr<IEnumShellItems> pEnum;
			hr = pFolder->BindToHandler(NULL, BHID_EnumItems, IID_PPV_ARGS(&pEnum));
			if (FAILED(hr))
				continue; 

			CComPtr<IShellItem> pItem;
			while (pEnum->Next(1, &pItem, NULL) == S_OK)
			{
				// 检查当前项目是否为文件夹
				SFGAOF sfgaoFlags = SFGAO_FOLDER;
				hr = pItem->GetAttributes(SFGAO_FOLDER, &sfgaoFlags);
				if (FAILED(hr))
				{
					pItem = NULL;
					continue;
				}

				if (sfgaoFlags & SFGAO_FOLDER)
				{
					// 是文件夹，递归处理
					CComPtr<IEnumShellItems> pSubEnum;
					hr = pItem->BindToHandler(NULL, BHID_EnumItems, IID_PPV_ARGS(&pSubEnum));
					if (FAILED(hr))
					{
						pItem = NULL;
						continue;
					}

					CComPtr<IShellItem> pSubItem;
					while (pSubEnum->Next(1, &pSubItem, NULL) == S_OK)
					{
						// 获取子项目路径
						LPWSTR subItemPath = NULL;
						hr = pSubItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &subItemPath);
						if (FAILED(hr))
						{
							pSubItem = NULL;
							continue;
						}

						// 获取子项目文件名
						LPWSTR subItemName = NULL;
						hr = pSubItem->GetDisplayName(SIGDN_FILESYSPATH, &subItemName);
						if (FAILED(hr))
						{
							CoTaskMemFree(subItemPath);
							pSubItem = NULL;
							continue;
						}

						// 提取文件名
						ATL::CPath subItemPathObj(subItemName);
						LPWSTR fileName = ::PathFindFileNameW(subItemName);

						// 将文件移动到目标文件夹
						hr = pfo->MoveItem(pSubItem, pDstDir, fileName, NULL);

						// 释放资源
						CoTaskMemFree(subItemPath);
						CoTaskMemFree(subItemName);
						pSubItem = NULL;
					}
				}
				else
				{
					// 是文件，直接移动
					// 获取文件名
					LPWSTR itemName = NULL;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &itemName);
					if (FAILED(hr))
					{
						pItem = NULL;
						continue;
					}

					// 提取文件名
					ATL::CPath itemPathObj(itemName);
					LPWSTR fileName = ::PathFindFileNameW(itemName);

					// 将文件移动到目标文件夹
					hr = pfo->MoveItem(pItem, pDstDir, fileName, NULL);

					// 释放资源
					CoTaskMemFree(itemName);
				}

				pItem = NULL;
			}
		}

		// 执行所有文件操作
		hr = pfo->PerformOperations();
		// 通知Shell刷新所有事件
		SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_FLUSHNOWAIT, NULL, NULL);
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
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