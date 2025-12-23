// framework.h
#pragma once

// 包括 SDKDDKVer.h 将定义可用的最高版本的 Windows 平台。

// 如果您希望为以前的 Windows 平台构建应用程序，请包括 WinSDKVer.h，并将
// _WIN32_WINNT 宏设置为希望支持的平台，然后再包括 SDKDDKVer.h。

#include <SDKDDKVer.h>
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>

// ATL 头文件
#include <atlbase.h>
#include <atlstr.h>