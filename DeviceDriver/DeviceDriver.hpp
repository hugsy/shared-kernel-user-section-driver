#pragma once

#include <ntddk.h>

#include "../Common/Utils.hpp"

#define _NO_CRT_STDIO_INLINE 1
#include <stdarg.h>
#include <stdio.h>


#define DEVICE_NAME L"CHANGEME"
#define DEVICE_PATH L"\\Device\\" DEVICE_NAME
#define DOS_DEVICE_PATH L"\\??\\" DEVICE_NAME
