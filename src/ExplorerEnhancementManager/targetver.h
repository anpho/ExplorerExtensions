// targetver.h
#pragma once

// 定义应用程序要支持的最低平台版本

// 此宏定义了此项目的最低 Windows 平台。
// 最新的 Windows SDK 将支持最低平台版本。
// 如果要支持较早的 Windows 平台，则必须修改这些定义。
// 有关不同 Windows 平台的相应值的信息，请参阅：
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745.aspx

#ifndef WINVER
#define WINVER 0x0601       // Windows 7
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601  // Windows 7
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410  // Windows 98
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0800      // Internet Explorer 8.0
#endif