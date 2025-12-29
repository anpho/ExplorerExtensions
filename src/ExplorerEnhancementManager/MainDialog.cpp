﻿#include "pch.h"
#include "MainDialog.h"
#include "AboutDialog.h"

LRESULT CMainDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // 设置窗口居中
    CenterWindow();

    // 初始化UI状态
    UpdateUI();

    // 动态加载字符串资源，设置对话框标题和控件文本
    CString strTitle, strBreakFolder, strNewFolder, strApply, strAbout, strExit;
    strTitle.LoadString(IDS_MAIN_TITLE);
    strBreakFolder.LoadString(IDS_BREAKFOLDER_CHECK);
    strNewFolder.LoadString(IDS_NEWFOLDER_CHECK);
    strApply.LoadString(IDS_APPLY_BUTTON);
    strAbout.LoadString(IDS_ABOUT_BUTTON);
    strExit.LoadString(IDS_EXIT_BUTTON);

    // 设置对话框标题
    SetWindowText(strTitle);
    // 设置复选框文本
    SetDlgItemText(IDC_BREAKFOLDER_CHECK, strBreakFolder);
    SetDlgItemText(IDC_NEWFOLDER_CHECK, strNewFolder);
    // 设置按钮文本
    SetDlgItemText(IDC_APPLY_BUTTON, strApply);
    SetDlgItemText(IDC_ABOUT_BUTTON, strAbout);
    SetDlgItemText(IDC_EXIT_BUTTON, strExit);

    bHandled = FALSE;
    return TRUE;
}

LRESULT CMainDialog::OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ApplyChanges();
    return 0;
}

LRESULT CMainDialog::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CAboutDialog aboutDlg;
    aboutDlg.DoModal();
    return 0;
}

LRESULT CMainDialog::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CMainDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ApplyChanges();
    EndDialog(wID);
    return 0;
}

LRESULT CMainDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

void CMainDialog::UpdateUI()
{
    // 检查当前的启用状态
    BOOL bBreakFolderEnabled = m_registryHelper.IsBreakFolderEnabled();
    BOOL bNewFolderEnabled = m_registryHelper.IsNewFolderEnabled();

    // 更新复选框状态
    CheckDlgButton(IDC_BREAKFOLDER_CHECK, bBreakFolderEnabled ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_NEWFOLDER_CHECK, bNewFolderEnabled ? BST_CHECKED : BST_UNCHECKED);
}

void CMainDialog::ApplyChanges()
{
    // 获取用户选择的状态
    BOOL bBreakFolderEnabled = IsDlgButtonChecked(IDC_BREAKFOLDER_CHECK) == BST_CHECKED;
    BOOL bNewFolderEnabled = IsDlgButtonChecked(IDC_NEWFOLDER_CHECK) == BST_CHECKED;

    // 应用更改
    m_registryHelper.SetBreakFolderEnabled(bBreakFolderEnabled);
    m_registryHelper.SetNewFolderEnabled(bNewFolderEnabled);

    // 提示用户需要重启Explorer
    CString strMessage, strTitle;
    strMessage.LoadString(IDS_INFO_RESTART_EXPLORER);
    strTitle.LoadString(IDS_INFO_TITLE);
    MessageBox(strMessage, strTitle, MB_OK | MB_ICONINFORMATION);
}