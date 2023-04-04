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
Log(_In_ const char* FormatString, ...);

}; // namespace Log


#ifdef _DEBUG
#define dbg(fmt, ...) Log::Log("[=] " fmt, __VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif // _DEBUG

#define ok(fmt, ...) Log::Log("[+] " fmt, __VA_ARGS__)
#define info(fmt, ...) Log::Log("[*] " fmt, __VA_ARGS__)
#define warn(fmt, ...) Log::Log("[!] " fmt, __VA_ARGS__)
#define err(fmt, ...) Log::Log("[-] " fmt, __VA_ARGS__)
