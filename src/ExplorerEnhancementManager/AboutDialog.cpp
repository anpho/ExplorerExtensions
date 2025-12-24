#include "pch.h"
#include "AboutDialog.h"
#include <shellapi.h>

LRESULT CAboutDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // 设置窗口居中
    CenterWindow();

    // 动态加载字符串资源，设置对话框标题和控件文本
    CString strTitle, strOk, strBuy;
    strTitle.LoadString(IDS_ABOUT_TITLE);
    strOk.LoadString(IDS_OK_BUTTON);
    strBuy.LoadString(IDS_BUY_BUTTON_TEXT);

    // 设置对话框标题
    SetWindowText(strTitle);
    // 设置按钮文本
    SetDlgItemText(IDOK, strOk);
    SetDlgItemText(IDC_BUY_BUTTON, strBuy);

    bHandled = FALSE;
    return TRUE;
}

LRESULT CAboutDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CAboutDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}
LRESULT CAboutDialog::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
    // 禁用当前点击的按钮
    ::EnableWindow(hWndCtl, FALSE);
    
    // 使用默认浏览器打开GitHub链接
    ShellExecute(NULL, L"open", L"https://github.com", NULL, NULL, SW_SHOWNORMAL);
    
    // 重新启用按钮
    ::EnableWindow(hWndCtl, TRUE);

    return 0;
}
