#pragma once

#include <dontuse.h>
#include <fltKernel.h>


#define DEVICE_NAME "CHANGEME"
#define DRIVER_CONTEXT_TAG 'CHGM'
#define DRIVER_TAG DRIVER_CONTEXT_TAG


EXTERN_C_START

NTKERNELAPI
NTSTATUS
ZwQueryInformationProcess(
    _In_ HANDLE ProcessHandle,
    _In_ PROCESSINFOCLASS ProcessInformationClass,
    _Out_ PVOID ProcessInformation,
    _In_ ULONG ProcessInformationLength,
    _Out_opt_ PULONG ReturnLength);


NTKERNELAPI
NTSTATUS
NTAPI
MmCopyVirtualMemory(
    PEPROCESS SourceProcess,
    PVOID SourceAddress,
    PEPROCESS TargetProcess,
    PVOID TargetAddress,
    SIZE_T BufferSize,
    KPROCESSOR_MODE PreviousMode,
    PSIZE_T ReturnSize);


NTKERNELAPI
NTSTATUS
PsLookupProcessByProcessId(_In_ HANDLE ProcessId, _Outptr_ PEPROCESS* Process);


EXTERN_C_END
