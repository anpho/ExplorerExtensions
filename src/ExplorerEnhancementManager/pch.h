// pch.h: 预编译头文件
#pragma once

#include "framework.h"

// WTL 10（src/third_party/wtl，MS-PL）：应用模块与消息循环。
// 仅用于进程级框架；对话框仍使用 ATL CDialogImpl。
#include <atlapp.h>
extern WTL::CAppModule _Module;
#include <atlwin.h>
