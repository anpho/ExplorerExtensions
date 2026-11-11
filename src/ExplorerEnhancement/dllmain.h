// dllmain.h: 模块类的声明。

class CExplorerEnhancementModule : public ATL::CAtlDllModuleT<CExplorerEnhancementModule>
{
public:
	DECLARE_LIBID(LIBID_ExplorerEnhancementLib)
};

extern class CExplorerEnhancementModule _AtlModule;
