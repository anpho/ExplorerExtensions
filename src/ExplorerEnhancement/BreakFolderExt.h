// BreakFolderExt.h: CBreakFolderExt 的声明
// 功能：Windows资源管理器扩展，用于将选中文件夹中的文件移动到父文件夹
// 支持：本地路径、UNC路径、SAMBA路径等网络路径 

#pragma once
#include "resource.h" // 主符号
#include <ShlObj.h>
#include <vector>
#include <string>
#include <atlstr.h>

#include "ExplorerEnhancement_i.h"

using namespace ATL;

// CBreakFolderExt 类
// 功能：实现Windows资源管理器的上下文菜单扩展，用于将选中文件夹中的文件移动到父文件夹
// 接口：实现了IShellExtInit（初始化）和IContextMenu（上下文菜单）等接口
class ATL_NO_VTABLE CBreakFolderExt : public CComObjectRootEx<CComMultiThreadModel>, // 使用多线程模型
									  public CComCoClass<CBreakFolderExt, &CLSID_BreakFolderExt>,
									  public IBreakFolderExt,
									  public IShellExtInit, // 用于扩展初始化的接口
									  public IContextMenu	// 用于上下文菜单操作的接口
{
public:
	/// <summary>
	/// 构造函数
	/// </summary>
	CBreakFolderExt()
	{
		(m_contextMenuName.LoadStringW(IDS_MENU_BREAKFOLDERS)); // 加载上下文菜单名称
		if (!m_hBitmap)
		{
			loadIcon(); // 加载图标
		}
	}

	DECLARE_REGISTRY_RESOURCEID(107)
	DECLARE_NOT_AGGREGATABLE(CBreakFolderExt)

	BEGIN_COM_MAP(CBreakFolderExt)
	COM_INTERFACE_ENTRY(IBreakFolderExt)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	/// <summary>
	/// COM对象最终构造函数
	/// </summary>
	/// <returns>HRESULT 操作结果</returns>
	HRESULT FinalConstruct()
	{
		if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
		{
			return S_OK;
		}
		else
		{
			return E_FAIL;
		}
	}

	/// <summary>
	/// COM对象最终释放函数
	/// </summary>
	void FinalRelease()
	{
		if (m_hIcon)
		{
			DeleteObject(m_hIcon); // 释放图标资源
			m_hIcon = nullptr;
		}
		CoUninitialize(); // 释放COM资源
	}

public:
	// 通过 IShellExtInit 继承
	/// <summary>
	/// 初始化Shell扩展
	/// </summary>
	/// <param name="pidlFolder">当前文件夹的ID列表</param>
	/// <param name="pdtobj">包含选中项的数据对象</param>
	/// <param name="hkeyProgID">程序ID的注册表项</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID) override;

	// 通过 IContextMenu 继承
	/// <summary>
	/// 查询上下文菜单，添加自定义菜单项
	/// </summary>
	/// <param name="hmenu">目标菜单句柄</param>
	/// <param name="indexMenu">菜单项插入位置</param>
	/// <param name="idCmdFirst">第一个可用命令ID</param>
	/// <param name="idCmdLast">最后一个可用命令ID</param>
	/// <param name="uFlags">菜单标志</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) override;

	/// <summary>
	/// 调用命令，执行菜单项操作
	/// </summary>
	/// <param name="pici">命令调用信息</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall InvokeCommand(CMINVOKECOMMANDINFO *pici) override;

	/// <summary>
	/// 获取命令字符串（用于菜单项名称等）
	/// </summary>
	/// <param name="idCmd">命令ID</param>
	/// <param name="uType">字符串类型</param>
	/// <param name="pReserved">保留参数</param>
	/// <param name="pszName">输出的字符串缓冲区</param>
	/// <param name="cchMax">缓冲区大小</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pReserved, CHAR *pszName, UINT cchMax) override;

private:
	ATL::CString m_contextMenuName;				 // 上下文菜单名称
	std::vector<std::wstring> m_selectedFolders; // 存储选中的文件夹路径，支持UNC路径
	HRESULT OnBreakFoldersTriggered();			 // 处理"打破文件夹"操作的函数
	HICON m_hIcon = nullptr;					 // 菜单项图标句柄
	HBITMAP m_hBitmap = nullptr;				 // 菜单项位图句柄
	void loadIcon();							 // 加载图标的函数
};

OBJECT_ENTRY_AUTO(__uuidof(BreakFolderExt), CBreakFolderExt)
