// NewFolderExt.h: CNewFolderExt 的声明
// 功能：Windows资源管理器扩展，用于创建新文件夹并将选中的项目移动到该文件夹中
// 支持：本地路径、UNC路径、SAMBA路径等网络路径

#pragma once
#include "resource.h" // 主符号 
#include <ShlObj.h>
#include <wil/resource.h>
#include <atlstr.h>
#include <vector>
#include <new>

#include "ExplorerEnhancement_i.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif

using namespace ATL;

// CNewFolderExt 类
// 功能：实现Windows资源管理器的上下文菜单扩展，用于创建新文件夹并将选中项目移动到该文件夹
// 接口：实现了IShellExtInit（初始化）、IContextMenu（上下文菜单）和IExplorerCommand（Explorer命令）等接口
class ATL_NO_VTABLE CNewFolderExt : public CComObjectRootEx<CComSingleThreadModel>,
									public CComCoClass<CNewFolderExt, &CLSID_NewFolderExt>,
									public INewFolderExt,
									public IShellExtInit,	// 用于扩展初始化的接口
									public IContextMenu,	// 用于上下文菜单操作的接口
									public IExplorerCommand // Windows 11及以上支持的新命令接口
{
public:
	DECLARE_REGISTRY_RESOURCEID(106)

	DECLARE_NOT_AGGREGATABLE(CNewFolderExt)

	BEGIN_COM_MAP(CNewFolderExt)
	COM_INTERFACE_ENTRY(INewFolderExt)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
	COM_INTERFACE_ENTRY(IExplorerCommand)
	END_COM_MAP()

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
	/// <param name="pici">命令调用信息</param>
	/// <param name="psiItemArray">选中的Shell项目数组</param>
	/// <returns>HRESULT 操作结果</returns>
	HRESULT __stdcall DoCreateAndMoveItems(CMINVOKECOMMANDINFO *pici, IShellItemArray *psiItemArray);

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
	/// 从文件路径中提取文件名（包含扩展名）
	/// </summary>
	/// <param name="filepath">文件路径</param>
	/// <returns>ATL::CString 文件名</returns>
	ATL::CString extract_filename(const ATL::CString filepath);

	/// <summary>
	/// 从文件路径中提取文件名（不包含扩展名）
	/// </summary>
	/// <param name="filepath">文件路径</param>
	/// <returns>ATL::CString 文件名（不含扩展名）</returns>
	ATL::CString extract_filename_without_ext(const ATL::CString filepath);

	/// <summary>
	/// 重定向文件路径到新的基础文件夹
	/// </summary>
	/// <param name="src_path">源文件路径</param>
	/// <param name="new_base_folder">新的基础文件夹</param>
	/// <returns>ATL::CString 重定向后的路径</returns>
	ATL::CString rebase_path(ATL::CString src_path, ATL::CString new_base_folder);

	/// <summary>
	/// 组合文件夹名称和后缀
	/// </summary>
	/// <param name="folder_name">基础文件夹名称</param>
	/// <param name="suffix">后缀</param>
	/// <returns>ATL::CString 组合后的文件夹名称</returns>
	ATL::CString combine_path(ATL::CString folder_name, ATL::CString suffix);

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

	// 通过 IExplorerCommand 继承
	/// <summary>
	/// 获取命令标题（用于上下文菜单显示）
	/// </summary>
	/// <param name="psiItemArray">选中的Shell项目数组</param>
	/// <param name="ppszName">输出的标题字符串指针</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall GetTitle(IShellItemArray *psiItemArray, LPWSTR *ppszName) override;

	/// <summary>
	/// 获取命令图标路径
	/// </summary>
	/// <param name="psiItemArray">选中的Shell项目数组</param>
	/// <param name="ppszIcon">输出的图标路径字符串指针</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall GetIcon(IShellItemArray *psiItemArray, LPWSTR *ppszIcon) override;

	/// <summary>
	/// 获取命令工具提示
	/// </summary>
	/// <param name="psiItemArray">选中的Shell项目数组</param>
	/// <param name="ppszInfotip">输出的工具提示字符串指针</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall GetToolTip(IShellItemArray *psiItemArray, LPWSTR *ppszInfotip) override;

	/// <summary>
	/// 获取命令的规范名称（GUID）
	/// </summary>
	/// <param name="pguidCommandName">输出的命令GUID</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall GetCanonicalName(GUID *pguidCommandName) override;

	/// <summary>
	/// 获取命令状态（启用/禁用等）
	/// </summary>
	/// <param name="psiItemArray">选中的Shell项目数组</param>
	/// <param name="fOkToBeSlow">是否允许慢操作</param>
	/// <param name="pCmdState">输出的命令状态</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall GetState(IShellItemArray *psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE *pCmdState) override;

	/// <summary>
	/// 调用命令（IExplorerCommand接口）
	/// </summary>
	/// <param name="psiItemArray">选中的Shell项目数组</param>
	/// <param name="pbc">绑定上下文</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall Invoke(IShellItemArray *psiItemArray, IBindCtx *pbc) override;

	/// <summary>
	/// 获取命令标志
	/// </summary>
	/// <param name="pFlags">输出的命令标志</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall GetFlags(EXPCMDFLAGS *pFlags) override;

	/// <summary>
	/// 枚举子命令
	/// </summary>
	/// <param name="ppEnum">输出的子命令枚举器</param>
	/// <returns>HRESULT 操作结果</returns>
	virtual HRESULT __stdcall EnumSubCommands(IEnumExplorerCommand **ppEnum) override;

private:
	CComPtr<IDataObject> m_spdo = nullptr;	  // 数据对象指针，包含选中的项目
	CComPtr<IBindCtx> ibindctx = nullptr;	  // 绑定上下文指针
	ATL::CString m_contextmenu_name;		  // 上下文菜单名称
	std::vector<ATL::CString> selected_files; // 选中的文件路径列表，支持UNC路径
	HBITMAP m_icon = nullptr;				  // 菜单项图标
	IShellItemArray *spsia;					  // Shell项目数组指针
	IStream *_pstmShellItemArray;			  // Shell项目数组的流指针
};

/// <summary>
/// 线程参数结构体
/// </summary>
struct threadParam
{
	HWND hwnd;				  // 窗口句柄
	LPCWSTR folder_to_select; // 要选择的文件夹路径
};

/// <summary>
/// 选择项目的线程函数
/// </summary>
/// <param name="pArguments">线程参数</param>
/// <returns>unsigned 线程退出码</returns>
unsigned __stdcall selectItem(void *pArguments);

OBJECT_ENTRY_AUTO(__uuidof(NewFolderExt), CNewFolderExt)
