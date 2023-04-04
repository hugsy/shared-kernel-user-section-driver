#include "MinifilterDriver.hpp"

#include <ntddk.h>

#include "../Common/Log.hpp"
#include "../Common/Utils.hpp"

#pragma prefast(disable : __WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

#define GOTO_EXIT_IF_FAILED(x)                                                                                         \
    {                                                                                                                  \
        if ( !NT_SUCCESS(Status) )                                                                                     \
        {                                                                                                              \
            err(x ": Failed with %#x\n", Status);                                                                      \
            goto Exit;                                                                                                 \
        }                                                                                                              \
    }

#define RETURN_IF_FAILED(x)                                                                                            \
    {                                                                                                                  \
        if ( !NT_SUCCESS(Status) )                                                                                     \
        {                                                                                                              \
            err(x ": Failed with %#x\n", Status);                                                                      \
            return Status;                                                                                             \
        }                                                                                                              \
    }

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);


NTSTATUS
MinifilterDriverUnload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags);

NTSTATUS
MinifilterDriverInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType);

FLT_PREOP_CALLBACK_STATUS
MinifilterDriverPreCreateOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

FLT_POSTOP_CALLBACK_STATUS
MinifilterDriverPostCreateOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags);

FLT_PREOP_CALLBACK_STATUS
MinifilterDriverPreWriteOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

EXTERN_C_END


const FLT_OPERATION_REGISTRATION Callbacks[] = {
    {IRP_MJ_CREATE, 0, MinifilterDriverPreCreateOperation, MinifilterDriverPostCreateOperation},
    {IRP_MJ_WRITE, FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO, MinifilterDriverPreWriteOperation},
    {IRP_MJ_OPERATION_END}};


const FLT_REGISTRATION g_FilterRegistration = {
    sizeof(FLT_REGISTRATION),      //  Size
    FLT_REGISTRATION_VERSION,      //  Version
    0,                             //  Flags
    nullptr,                       //  Context
    Callbacks,                     //  Operation callbacks
    MinifilterDriverUnload,        //  MiniFilterUnload
    MinifilterDriverInstanceSetup, //  InstanceSetup
    nullptr,                       //  InstanceQueryTeardown
    nullptr,                       //  InstanceTeardownStart
    nullptr,                       //  InstanceTeardownComplete
    nullptr,                       //  GenerateFileName
    nullptr,                       //  GenerateDestinationFileName
    nullptr                        //  NormalizeNameComponent
};


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, MinifilterDriverUnload)
#pragma alloc_text(PAGE, MinifilterDriverInstanceSetup)
#endif

using PsGetContextThread_t = NTSTATUS(NTAPI*)(PETHREAD Thread, PCONTEXT ThreadContext, KPROCESSOR_MODE PreviousMode);

struct
{
    PFLT_FILTER FilterHandle {nullptr};
    HANDLE SectionHandle {nullptr};
    PsGetContextThread_t PsGetContextThread;
} Globals;


NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    PAGED_CODE();
    NTSTATUS Status;
    info("Loading %S\n", DEVICE_NAME);

    // solve PsGetContextThread
    {
        UNICODE_STRING n = RTL_CONSTANT_STRING(L"PsGetContextThread");
        PVOID addr       = ::MmGetSystemRoutineAddress(&n);
        if ( !addr )
            Status = STATUS_PROCEDURE_NOT_FOUND;
        RETURN_IF_FAILED("MmGetSystemRoutineAddress(PsGetContextThread)");

        Globals.PsGetContextThread = (PsGetContextThread_t)addr;
        ok("PsGetContextThread = %p\n", addr);
    }

    // create section
    {
        OBJECT_ATTRIBUTES oa {};
        InitializeObjectAttributes(
            &oa,
            nullptr,
            OBJ_EXCLUSIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK,
            nullptr,
            nullptr);
        LARGE_INTEGER li {.QuadPart = 0x1000};
        Status =
            ::ZwCreateSection(&Globals.SectionHandle, SECTION_MAP_WRITE, &oa, &li, PAGE_READWRITE, SEC_COMMIT, nullptr);
        RETURN_IF_FAILED("ZwCreateSection");
    }
    ok("Section at %p\n", Globals.SectionHandle);

    // do the registration
    Status = ::FltRegisterFilter(DriverObject, &g_FilterRegistration, &Globals.FilterHandle);
    RETURN_IF_FAILED("FltRegisterFilter");

    Status = ::FltStartFiltering(Globals.FilterHandle);
    RETURN_IF_FAILED("FltStartFiltering");

    ok("Loaded fs filter %S\n", DEVICE_NAME);

    return Status;
}


NTSTATUS
MinifilterDriverUnload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Flags);
    PAGED_CODE();

    if ( Globals.FilterHandle )
        ::FltUnregisterFilter(Globals.FilterHandle);

    if ( Globals.FilterHandle )
        ::ZwClose(Globals.FilterHandle);

    ok("Unloaded fs filter %S\n", DEVICE_NAME);

    return STATUS_SUCCESS;
}


NTSTATUS
MinifilterDriverInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    PAGED_CODE();

    return STATUS_SUCCESS;
}

#define PRINT_REG64(r) ok(#r "=%016llx\n", ctx->r)


FLT_PREOP_CALLBACK_STATUS
MinifilterDriverPreCreateOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    PVOID BaseAddress {nullptr};
    PCONTEXT ctx {nullptr};
    HANDLE h {nullptr};
    usize ViewSize {0x1000};

    NTSTATUS Status = ::ZwMapViewOfSection(
        Globals.SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0L,
        ViewSize,
        NULL,
        &ViewSize,
        ViewUnmap,
        0L,
        PAGE_READWRITE);
    GOTO_EXIT_IF_FAILED("ZwMapViewOfSection");

    ok("in PID=%lu/TID=%lu , MappedSection=%p\n", ::PsGetCurrentProcessId(), ::PsGetCurrentThreadId(), BaseAddress);

    h = ::MmSecureVirtualMemory(BaseAddress, ViewSize, PAGE_READWRITE);
    if ( !h )
        Status = STATUS_NOT_LOCKED;
    GOTO_EXIT_IF_FAILED("MmSecureVirtualMemoryEx");

    ctx               = reinterpret_cast<PCONTEXT>(BaseAddress);
    ctx->ContextFlags = CONTEXT_FULL;
    Status            = Globals.PsGetContextThread(PsGetCurrentThread(), ctx, UserMode);
    GOTO_EXIT_IF_FAILED("PsGetContextThread");

    PRINT_REG64(Rip);
    PRINT_REG64(Rbp);
    PRINT_REG64(Rsp);
    PRINT_REG64(Rax);
    PRINT_REG64(Rbx);
    PRINT_REG64(Rcx);
    PRINT_REG64(Rdx);
    PRINT_REG64(Rdx);

    if ( ctx->Rip <= 0x7fffffffffffull )
    {
        ok("PsGetContextThread() succeeded\n");
    }

    DbgBreakPoint();

    ::MmUnsecureVirtualMemory(h);
    Status = ::ZwUnmapViewOfSection(NtCurrentProcess(), BaseAddress);
    GOTO_EXIT_IF_FAILED("ZwUnmapViewOfSection");

    dbg("Section view %p unmapped\n", BaseAddress);

Exit:
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


FLT_POSTOP_CALLBACK_STATUS
MinifilterDriverPostCreateOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_PREOP_CALLBACK_STATUS
MinifilterDriverPreWriteOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
{
    auto Status = FLT_PREOP_SUCCESS_NO_CALLBACK;

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if ( Data->RequestorMode == KernelMode )
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    return Status;
}
