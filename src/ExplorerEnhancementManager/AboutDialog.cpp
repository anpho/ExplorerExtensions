#include "pch.h"
#include "AboutDialog.h"

LRESULT CAboutDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    CenterWindow();

    CString strTitle, strOk, strDescription;
    strTitle.LoadString(IDS_ABOUT_TITLE);
    strOk.LoadString(IDS_OK_BUTTON);
    strDescription.LoadString(IDS_ABOUT_DESCRIPTION);

    SetWindowText(strTitle);
    SetDlgItemText(IDOK, strOk);
    SetDlgItemText(IDC_ABOUT_DESCRIPTION, strDescription);

    bHandled = FALSE;
    return TRUE;
}

LRESULT CAboutDialog::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}

LRESULT CAboutDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}
