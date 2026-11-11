// NewFolderExt.h: CNewFolderExt 的声明
// 功能：Windows资源管理器扩展，用于创建新文件夹并将选中的项目移动到该文件夹中
// 支持：本地路径、UNC路径、SAMBA路径等网络路径

#pragma once
#include "resource.h" // 主符号 
#include <ShlObj.h>
#include <atlstr.h>
#include <vector>

#include "ExplorerEnhancement_i.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif

using namespace ATL;

// CNewFolderExt 类
// 功能：实现Windows资源管理器的上下文菜单扩展，用于创建新文件夹并将选中项目移动到该文件夹
// 接口：实现了IShellExtInit（初始化）和IContextMenu（上下文菜单）等接口
class ATL_NO_VTABLE CNewFolderExt : public CComObjectRootEx<CComMultiThreadModel>,
									public CComCoClass<CNewFolderExt, &CLSID_NewFolderExt>,
									public INewFolderExt,
									public IShellExtInit,	// 用于扩展初始化的接口
									public IContextMenu		// 用于上下文菜单操作的接口
{
public:
	DECLARE_NOT_AGGREGATABLE(CNewFolderExt)

	BEGIN_COM_MAP(CNewFolderExt)
	COM_INTERFACE_ENTRY(INewFolderExt)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
	END_COM_MAP()

	// 注册由 DllRegisterServer/DllInstall 直接写注册表，不使用 ATL 的注册资源。
	static HRESULT WINAPI UpdateRegistry(BOOL /*bRegister*/) throw()
	{
		return S_OK;
	}

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	/// <summary>
	/// COM对象最终构造函数
	/// </summary>
	/// <returns>HRESULT 操作结果</returns>
	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	/// <summary>
	/// COM对象最终释放函数
	/// </summary>
	void FinalRelease()
	{
	}

	/// <summary>
	/// 构造函数
	/// </summary>
	CNewFolderExt();

	/// <summary>
	/// 析构函数
	/// </summary>
	~CNewFolderExt();

	/// <summary>
	/// 线程处理函数，用于执行文件操作
	/// </summary>
	/// <returns>DWORD 线程退出码</returns>
	DWORD _ThreadProc();

	/// <summary>
	/// 静态线程处理函数，用于创建线程
	/// </summary>
	/// <param name="pv">线程参数</param>
	/// <returns>DWORD 线程退出码</returns>
	static DWORD __stdcall s_ThreadProc(void *pv);
	// 通过 IShellExtInit 继承
	/// <summary>
	/// 初始化Shell扩展
	/// </summary>
	/// <param name="pidlFolder">当前文件夹的ID列表</param>
	/// <param name="pdtobj">包含选中项的数据对象</param>
	/// <param name="hkeyProgID">程序ID的注册表项</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID) override;

	/// <summary>
	/// 从IDataObject中获取IShellItemArray
	/// </summary>
	/// <param name="dataSource">IDataObject数据源</param>
	/// <param name="items">输出的IShellItemArray指针</param>
	/// <returns>HRESULT 操作结果</returns>
	HRESULT __stdcall GetShellItemArrayFromDataObject(_In_ IUnknown *dataSource, _COM_Outptr_ IShellItemArray **items);

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
	/// 创建新文件夹并移动项目
	/// </summary>
	/// <param name="psiItemArray">选中的Shell项目数组</param>
	/// <returns>HRESULT 操作结果</returns>
	HRESULT __stdcall DoCreateAndMoveItems(IShellItemArray *psiItemArray);

	/// <summary>
	/// 计算两个字符串的第一个公共子串长度
	/// </summary>
	/// <param name="str1">第一个字符串</param>
	/// <param name="str2">第二个字符串</param>
	/// <param name="length_to_compare">比较的最大长度</param>
	/// <returns>int 公共子串长度</returns>
	/// <remarks>2022/3/18 增加length_to_compare参数，提高处理速度</remarks>
	int first_common_substring_length(const ATL::CString &str1, const ATL::CString &str2, int length_to_compare);

	/// <summary>
	/// 计算字符串向量的最长公共前缀
	/// </summary>
	/// <param name="strs">字符串向量</param>
	/// <returns>ATL::CString 最长公共前缀</returns>
	/// <remarks>2022/3/18 实现</remarks>
	ATL::CString longestCommonPrefix(const std::vector<ATL::CString> &strs);

	/// <summary>
	/// 查找文件路径的公共前缀
	/// </summary>
	/// <param name="filepaths">文件路径向量</param>
	/// <param name="default_path">输出的默认路径</param>
	/// <returns>ATL::CString 公共前缀路径</returns>
	ATL::CString find_common_prefix(const std::vector<ATL::CString> filepaths, OUT ATL::CString *default_path);

	/// <summary>
	/// 从文件路径中提取文件名（不包含扩展名）
	/// </summary>
	/// <param name="filepath">文件路径</param>
	/// <returns>ATL::CString 文件名（不含扩展名）</returns>
	ATL::CString extract_filename_without_ext(const ATL::CString filepath);

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
	CComPtr<IDataObject> m_spdo = nullptr;	  // 数据对象指针，包含选中的项目
	ATL::CString m_contextmenu_name;		  // 上下文菜单名称
	std::vector<ATL::CString> selected_files; // 选中的文件路径列表，支持UNC路径
	HBITMAP m_icon = nullptr;				  // 菜单项图标
	IShellItemArray *spsia;					  // Shell项目数组指针
	IStream *_pstmShellItemArray;			  // Shell项目数组的流指针
};

/// <summary>
/// 选择项目的线程函数
/// </summary>
/// <param name="pArguments">新文件夹路径（CString*，由线程负责释放）</param>
/// <returns>unsigned 线程退出码</returns>
unsigned __stdcall selectItem(void *pArguments);

OBJECT_ENTRY_AUTO(__uuidof(NewFolderExt), CNewFolderExt)
