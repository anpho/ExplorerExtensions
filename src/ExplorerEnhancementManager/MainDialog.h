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

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancelCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CRegistryHelper m_registryHelper;
    bool m_installed = false;

    void UpdateUI();
    void ApplyChanges();
};
