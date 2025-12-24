#include "pch.h"
#include "AboutDialog.h"
#include <shellapi.h>

LRESULT CAboutDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // 设置窗口居中
    CenterWindow();

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
