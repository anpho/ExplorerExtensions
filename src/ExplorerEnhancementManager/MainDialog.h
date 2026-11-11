#pragma once

#include "resource.h"
#include "RegistryHelper.h"

class CMainDialog : public CDialogImpl<CMainDialog>
{
public:
    enum { IDD = IDD_MAIN_DIALOG };

    BEGIN_MSG_MAP(CMainDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_HANDLER(IDC_APPLY_BUTTON, BN_CLICKED, OnApply)
        COMMAND_HANDLER(IDC_ABOUT_BUTTON, BN_CLICKED, OnAbout)
        COMMAND_HANDLER(IDC_EXIT_BUTTON, BN_CLICKED, OnExit)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancelCommand)
    END_MSG_MAP()

    // 打开对话框前告诉它当前注册范围/是否可修改。
    void Configure(InstallScope scope, bool canModify)
    {
        m_scope = scope;
        m_canModify = canModify;
    }

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancelCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CRegistryHelper m_registryHelper;
    InstallScope m_scope = InstallScope::None;
    bool m_canModify = false;

    bool MachineScope() const { return m_scope == InstallScope::Machine; }

    void CloseWithOptionalUnregister();
    void UnregisterOnExit();
    void UpdateUI();
    void ApplyChanges();
};
