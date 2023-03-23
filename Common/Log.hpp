#pragma once

#include <ntddk.h>

#define _NO_CRT_STDIO_INLINE 1
#include <stdarg.h>
#include <stdio.h>

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)


namespace Log
{
void
log(_In_ const WCHAR* lpFormatString, ...);

void
ntperror(_In_ const wchar_t* prefix, _In_ NTSTATUS Status);
}; // namespace Log


#ifdef _DEBUG
#define dbg(fmt, ...) Log::log(L"[=] " fmt, __VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif // _DEBUG

#define ok(fmt, ...) Log::log(L"[+] " fmt, __VA_ARGS__)
#define info(fmt, ...) Log::log(L"[*] " fmt, __VA_ARGS__)
#define warn(fmt, ...) Log::log(L"[!] " fmt, __VA_ARGS__)
#define err(fmt, ...) Log::log(L"[-] " fmt, __VA_ARGS__)
#define perror(fmt, ...) Log::ntperror(fmt, __VA_ARGS__)
