// framework.h: 公共包含文件（Windows + ATL）
#pragma once

// 目标平台：Windows 10/11。WTL 10 需要 _WIN32_WINNT/WINVER/_WIN32_IE。
#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0A00
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000000
#endif

#include <SDKDDKVer.h>
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容

#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>
