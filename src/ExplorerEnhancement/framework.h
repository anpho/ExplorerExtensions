#pragma once

#ifndef STRICT
#define STRICT
#endif 

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // 某些 CString 构造函数将是显式的

#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

// 定义缺失的宏
#define RETURN_IF_FAILED(hr) \
    do { \
        HRESULT _hr = hr; \
        if (FAILED(_hr)) { \
            return _hr; \
        } \
    } while(0)

#define RETURN_IF_NULL_ALLOC(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            return E_OUTOFMEMORY; \
        } \
    } while(0)
