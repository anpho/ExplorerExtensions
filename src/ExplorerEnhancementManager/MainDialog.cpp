#include "pch.h"
#include "MainDialog.h"
#include "AboutDialog.h"

LRESULT CMainDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    CenterWindow();

    CString strTitle, strBreakFolder, strNewFolder, strApply, strAbout, strExit, strDescription;
    strTitle.LoadString(IDS_MAIN_TITLE);
    strBreakFolder.LoadString(IDS_BREAKFOLDER_CHECK);
    strNewFolder.LoadString(IDS_NEWFOLDER_CHECK);
    strApply.LoadString(IDS_APPLY_BUTTON);
    strAbout.LoadString(IDS_ABOUT_BUTTON);
    strExit.LoadString(IDS_EXIT_BUTTON);

    SetWindowText(strTitle);
    SetDlgItemText(IDC_BREAKFOLDER_CHECK, strBreakFolder);
    SetDlgItemText(IDC_NEWFOLDER_CHECK, strNewFolder);
    SetDlgItemText(IDC_APPLY_BUTTON, strApply);
    SetDlgItemText(IDC_ABOUT_BUTTON, strAbout);
    SetDlgItemText(IDC_EXIT_BUTTON, strExit);

    // 未注册（没装或已手工 regsvr32 /u）时不能改开关，否则只是个假开关。
    m_installed = m_registryHelper.IsExplorerEnhancementRegistered() != FALSE;
    strDescription.LoadString(m_installed ? IDS_MAIN_DESCRIPTION : IDS_NOT_REGISTERED);
    SetDlgItemText(IDC_MAIN_DESCRIPTION, strDescription);
    GetDlgItem(IDC_BREAKFOLDER_CHECK).EnableWindow(m_installed ? TRUE : FALSE);
    GetDlgItem(IDC_NEWFOLDER_CHECK).EnableWindow(m_installed ? TRUE : FALSE);
    GetDlgItem(IDC_APPLY_BUTTON).EnableWindow(m_installed ? TRUE : FALSE);

    UpdateUI();

    bHandled = FALSE;
    return TRUE;
}

LRESULT CMainDialog::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DestroyWindow();
    return 0;
}

LRESULT CMainDialog::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    ::PostQuitMessage(0);
    return 0;
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
    DestroyWindow();
    return 0;
}

LRESULT CMainDialog::OnCancelCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    DestroyWindow();
    return 0;
}

void CMainDialog::UpdateUI()
{
    // 「停用」= 在 HKCU 的 Shell Extensions\Blocked 里登记 CLSID。
    const BOOL breakEnabled = m_installed && m_registryHelper.IsBreakFolderEnabled();
    const BOOL newEnabled = m_installed && m_registryHelper.IsNewFolderEnabled();
    CheckDlgButton(IDC_BREAKFOLDER_CHECK, breakEnabled ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_NEWFOLDER_CHECK, newEnabled ? BST_CHECKED : BST_UNCHECKED);
}

void CMainDialog::ApplyChanges()
{
    // 只有两处写入都成功才算成功，不再默认弹「已应用」。
    const BOOL breakOk = m_registryHelper.SetBreakFolderEnabled(
        IsDlgButtonChecked(IDC_BREAKFOLDER_CHECK) == BST_CHECKED);
    const BOOL newOk = m_registryHelper.SetNewFolderEnabled(
        IsDlgButtonChecked(IDC_NEWFOLDER_CHECK) == BST_CHECKED);

    CString strMessage, strTitle;
    strTitle.LoadString(IDS_INFO_TITLE);
    if (breakOk && newOk)
    {
        strMessage.LoadString(IDS_INFO_RESTART_EXPLORER);
        MessageBox(strMessage, strTitle, MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        strMessage.LoadString(IDS_ERROR_APPLY);
        MessageBox(strMessage, strTitle, MB_OK | MB_ICONERROR);
    }

    UpdateUI();
}
