#include "pch.h"
#include "MainDialog.h"
#include "AboutDialog.h"
#include "../Common/LocalizedString.h"

LRESULT CMainDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    CenterWindow();

    CString strTitle = LoadLocalizedString(IDS_MAIN_TITLE);
    CString strBreakFolder = LoadLocalizedString(IDS_BREAKFOLDER_CHECK);
    CString strNewFolder = LoadLocalizedString(IDS_NEWFOLDER_CHECK);
    CString strApply = LoadLocalizedString(IDS_APPLY_BUTTON);
    CString strAbout = LoadLocalizedString(IDS_ABOUT_BUTTON);
    CString strExit = LoadLocalizedString(IDS_EXIT_BUTTON);

    SetWindowText(strTitle);
    SetDlgItemText(IDC_BREAKFOLDER_CHECK, strBreakFolder);
    SetDlgItemText(IDC_NEWFOLDER_CHECK, strNewFolder);
    SetDlgItemText(IDC_APPLY_BUTTON, strApply);
    SetDlgItemText(IDC_ABOUT_BUTTON, strAbout);
    SetDlgItemText(IDC_EXIT_BUTTON, strExit);

    // “所有用户”范围需要管理员；未注册/注册失败时也不能改。
    UINT descriptionId = IDS_MAIN_DESCRIPTION;
    if (!m_canModify)
    {
        descriptionId = MachineScope() ? IDS_MACHINE_READONLY : IDS_NOT_REGISTERED;
    }
    const CString strDescription = LoadLocalizedString(descriptionId);
    SetDlgItemText(IDC_MAIN_DESCRIPTION, strDescription);

    GetDlgItem(IDC_BREAKFOLDER_CHECK).EnableWindow(m_canModify ? TRUE : FALSE);
    GetDlgItem(IDC_NEWFOLDER_CHECK).EnableWindow(m_canModify ? TRUE : FALSE);
    GetDlgItem(IDC_APPLY_BUTTON).EnableWindow(m_canModify ? TRUE : FALSE);

    UpdateUI();

    bHandled = FALSE;
    return TRUE;
}

LRESULT CMainDialog::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CloseWithOptionalUnregister();
    return 0;
}

LRESULT CMainDialog::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    ::PostQuitMessage(0);
    return 0;
}

// 按住 Shift 关闭/退出时注销 DLL（先确认，避免误触）。
void CMainDialog::CloseWithOptionalUnregister()
{
    if (::GetKeyState(VK_SHIFT) < 0)
    {
        UnregisterOnExit();
    }
    DestroyWindow();
}

void CMainDialog::UnregisterOnExit()
{
    const CString title = LoadLocalizedString(IDS_INFO_TITLE);
    if (m_scope == InstallScope::None)
    {
        return;
    }
    if (m_scope == InstallScope::Machine && !m_canModify)
    {
        MessageBox(LoadLocalizedString(IDS_UNREGISTER_NEED_ADMIN), title, MB_OK | MB_ICONINFORMATION);
        return;
    }
    if (MessageBox(LoadLocalizedString(IDS_CONFIRM_UNREGISTER), title,
                   MB_YESNO | MB_ICONQUESTION) != IDYES)
    {
        return;
    }

    CString error;
    if (m_registryHelper.UnregisterExtension(m_scope, error))
    {
        MessageBox(LoadLocalizedString(IDS_UNREGISTER_OK), title, MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        CString message = LoadLocalizedString(IDS_UNREGISTER_FAILED);
        message += L"\n\n";
        message += error;
        MessageBox(message, title, MB_OK | MB_ICONERROR);
    }
}

LRESULT CMainDialog::OnApply(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ApplyChanges();
    return 0;
}

LRESULT CMainDialog::OnAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CAboutDialog aboutDlg;
    aboutDlg.DoModal();
    return 0;
}

LRESULT CMainDialog::OnExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CloseWithOptionalUnregister();
    return 0;
}

LRESULT CMainDialog::OnCancelCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CloseWithOptionalUnregister();
    return 0;
}

void CMainDialog::UpdateUI()
{
    const bool machineScope = MachineScope();
    const BOOL breakEnabled = m_canModify && m_registryHelper.IsBreakFolderEnabled(machineScope);
    const BOOL newEnabled = m_canModify && m_registryHelper.IsNewFolderEnabled(machineScope);
    CheckDlgButton(IDC_BREAKFOLDER_CHECK, breakEnabled ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_NEWFOLDER_CHECK, newEnabled ? BST_CHECKED : BST_UNCHECKED);
}

void CMainDialog::ApplyChanges()
{
    if (!m_canModify)
    {
        const CString message = LoadLocalizedString(
            MachineScope() ? IDS_MACHINE_READONLY : IDS_NOT_REGISTERED);
        const CString title = LoadLocalizedString(IDS_INFO_TITLE);
        MessageBox(message, title, MB_OK | MB_ICONINFORMATION);
        return;
    }

    const bool machineScope = MachineScope();
    const BOOL breakOk = m_registryHelper.SetBreakFolderEnabled(
        IsDlgButtonChecked(IDC_BREAKFOLDER_CHECK) == BST_CHECKED, machineScope);
    const BOOL newOk = m_registryHelper.SetNewFolderEnabled(
        IsDlgButtonChecked(IDC_NEWFOLDER_CHECK) == BST_CHECKED, machineScope);

    CString strMessage, strTitle = LoadLocalizedString(IDS_INFO_TITLE);
    if (breakOk && newOk)
    {
        strMessage = LoadLocalizedString(IDS_INFO_RESTART_EXPLORER);
        MessageBox(strMessage, strTitle, MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        strMessage = LoadLocalizedString(IDS_ERROR_APPLY);
        MessageBox(strMessage, strTitle, MB_OK | MB_ICONERROR);
    }

    UpdateUI();
}
