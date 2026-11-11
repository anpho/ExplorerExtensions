// LocalizedString.h: 按用户界面语言从模块资源里取字符串。
//
// 这个扩展不是 MUI 程序，LoadString 的语言回退依赖线程语言/中性资源，
// 中文资源不一定会被选中（表现为中文 Windows 上始终显示英文）。
// 这里用 FindResourceEx 显式指定语言，保证中文界面优先取简体中文资源。
#pragma once

#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>

// 读取 RT_STRING 块里的第 id 个字符串；块内是 16 个“长度 WORD + 内容”的串。
inline ATL::CString LoadStringResource(LANGID language, UINT id)
{
    HINSTANCE instance = ATL::_AtlBaseModule.GetResourceInstance();
    if (instance == nullptr)
    {
        return ATL::CString();
    }

    HRSRC resource = ::FindResourceExW(instance, RT_STRING,
        MAKEINTRESOURCEW(static_cast<WORD>(id / 16 + 1)), language);
    if (resource == nullptr)
    {
        return ATL::CString();
    }

    HGLOBAL data = ::LoadResource(instance, resource);
    if (data == nullptr)
    {
        return ATL::CString();
    }

    const WORD* block = static_cast<const WORD*>(::LockResource(data));
    if (block == nullptr)
    {
        return ATL::CString();
    }

    for (UINT index = id % 16; index > 0; --index)
    {
        block += 1 + *block;
    }
    const WORD length = *block++;
    return ATL::CString(reinterpret_cast<LPCWSTR>(block), length);
}

// 中文界面优先简体中文资源，其余按用户界面语言取资源；都没有时用 fallback。
inline ATL::CString LoadLocalizedString(UINT id, LPCWSTR fallback = L"")
{
    const LANGID uiLanguage = ::GetUserDefaultUILanguage();
    const LANGID desiredLanguage = (PRIMARYLANGID(uiLanguage) == LANG_CHINESE)
        ? MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
        : uiLanguage;

    ATL::CString text = LoadStringResource(desiredLanguage, id);
    if (!text.IsEmpty())
    {
        return text;
    }

    text = LoadStringResource(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), id);
    if (!text.IsEmpty())
    {
        return text;
    }

    return ATL::CString(fallback);
}
