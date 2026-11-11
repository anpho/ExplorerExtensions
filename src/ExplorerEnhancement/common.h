#pragma once
#include "pch.h"

// 启用状态由管理器写入注册表；未写入时默认启用。
// 计算机范围（HKLM）优先于当前用户（HKCU）。
constexpr auto M_REG_BASEKEY = L"Software\\anpho\\ExplorerEnhancement";
constexpr auto M_REG_ENABLE_NEWFOLDER = L"EnableNewFolderPlus";
constexpr auto M_REG_ENABLE_BREAKFOLDER = L"EnableBreakFolderPlus";

inline bool ReadEnabledValue(HKEY root, LPCWSTR valueName, DWORD& enabled)
{
    DWORD size = sizeof(enabled);
    return ::RegGetValueW(root, M_REG_BASEKEY, valueName, RRF_RT_REG_DWORD,
                          nullptr, &enabled, &size) == ERROR_SUCCESS;
}

inline bool IsExtensionEnabled(LPCWSTR valueName)
{
    DWORD enabled = 0;
    if (ReadEnabledValue(HKEY_LOCAL_MACHINE, valueName, enabled))
    {
        return enabled != 0;
    }
    if (ReadEnabledValue(HKEY_CURRENT_USER, valueName, enabled))
    {
        return enabled != 0;
    }
    return true;
}

// 显示文件操作错误：context 说明 + HRESULT 的系统描述 + 十六进制错误码
inline void ShowOperationError(HWND hwnd, HRESULT hr, LPCWSTR context)
{
    wchar_t reason[512] = L"";
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, static_cast<DWORD>(hr), 0,
                   reason, static_cast<DWORD>(ARRAYSIZE(reason)), nullptr);

    wchar_t text[1024];
    swprintf_s(text, L"%s\n\n%s (0x%08X)", context, reason, static_cast<unsigned>(hr));
    MessageBoxW(hwnd, text, L"anpho's Extensions", MB_ICONERROR | MB_OK);
}

static HBITMAP IconToBitmap(HICON hIcon, SIZE *pTargetSize = NULL)
{
    ICONINFO info = {0};
    if (hIcon == NULL || !GetIconInfo(hIcon, &info))
    {
        return NULL;
    }

    INT nWidth = 0;
    INT nHeight = 0;
    if (pTargetSize != NULL)
    {
        nWidth = pTargetSize->cx;
        nHeight = pTargetSize->cy;
    }
    else
    {
        if (info.hbmColor != NULL)
        {
            BITMAP bmp = {0};
            GetObject(info.hbmColor, sizeof(bmp), &bmp);

            nWidth = bmp.bmWidth;
            nHeight = bmp.bmHeight;
        }
    }

    // 释放GetIconInfo分配的资源
    if (info.hbmColor != NULL)
    {
        DeleteObject(info.hbmColor);
        info.hbmColor = NULL;
    }

    if (info.hbmMask != NULL)
    {
        DeleteObject(info.hbmMask);
        info.hbmMask = NULL;
    }

    if (nWidth <= 0 || nHeight <= 0)
    {
        return NULL;
    }

    INT nPixelCount = nWidth * nHeight;

    HDC dc = GetDC(NULL);
    if (dc == NULL)
    {
        return NULL; // 获取设备上下文失败
    }

    INT *pData = NULL;
    HDC dcMem = NULL;
    HBITMAP hBmpOld = NULL;
    bool *pOpaque = NULL;
    HBITMAP dib = NULL;
    BOOL bSuccess = FALSE;

    do
    {
        BITMAPINFOHEADER bi = {0};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = nWidth;
        bi.biHeight = -nHeight;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        dib = CreateDIBSection(dc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (VOID **)&pData, NULL, 0);
        if (dib == NULL)
            break;

        memset(pData, 0, static_cast<size_t>(nPixelCount) * 4);

        dcMem = CreateCompatibleDC(dc);
        if (dcMem == NULL)
            break;

        hBmpOld = (HBITMAP)SelectObject(dcMem, dib);
        if (hBmpOld == NULL)
        {
            // 如果SelectObject失败，需要删除DIB
            DeleteObject(dib);
            dib = NULL;
            break;
        }

        ::DrawIconEx(dcMem, 0, 0, hIcon, nWidth, nHeight, 0, NULL, DI_MASK);

        pOpaque = new (std::nothrow) bool[nPixelCount];
        if (pOpaque == NULL)
        {
            // 如果内存分配失败，需要清理之前分配的资源
            DeleteObject(dib);
            dib = NULL;
            break;
        }

        for (INT i = 0; i < nPixelCount; ++i)
        {
            pOpaque[i] = !pData[i];
        }

        memset(pData, 0, nPixelCount * 4);
        ::DrawIconEx(dcMem, 0, 0, hIcon, nWidth, nHeight, 0, NULL, DI_NORMAL);

        BOOL bPixelHasAlpha = FALSE;
        UINT *pPixel = (UINT *)pData;
        for (INT i = 0; i < nPixelCount; ++i, ++pPixel)
        {
            if ((*pPixel & 0xff000000) != 0)
            {
                bPixelHasAlpha = TRUE;
                break;
            }
        }

        if (!bPixelHasAlpha)
        {
            pPixel = (UINT *)pData;
            for (INT i = 0; i < nPixelCount; ++i, ++pPixel)
            {
                if (pOpaque[i])
                {
                    *pPixel |= 0xFF000000;
                }
                else
                {
                    *pPixel &= 0x00FFFFFF;
                }
            }
        }

        bSuccess = TRUE;

    } while (FALSE);

    if (pOpaque != NULL)
    {
        delete[] pOpaque;
        pOpaque = NULL;
    }

    if (dcMem != NULL)
    {
        if (hBmpOld != NULL)
        {
            SelectObject(dcMem, hBmpOld);  // 恢复旧的位图
        }
        DeleteDC(dcMem);
    }

    ReleaseDC(NULL, dc);

    if (!bSuccess)
    {
        if (dib != NULL)
        {
            DeleteObject(dib);
            dib = NULL;
        }
        return NULL;  // 操作失败，返回NULL
    }

    return dib;
}
