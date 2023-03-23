#include "DeviceDriver.hpp"

#include "../Common/Log.hpp"

static PDEVICE_OBJECT g_pDeviceObject;


///
/// @brief Generic IRP completion function
///
/// @param [inout] Irp
/// @param [inout] Status
/// @param [inout] Information
/// @return NTSTATUS
///
EXTERN_C
NTSTATUS
CompleteRequest(_In_ PIRP Irp, _In_ NTSTATUS Status, _In_ ULONG_PTR Information)
{
    Irp->IoStatus.Status      = Status;
    Irp->IoStatus.Information = Information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


///
/// @brief Generic routine for unsupported major types.
///
/// @param [inout] DeviceObject
/// @param [inout] Irp
/// @return NTSTATUS
///
EXTERN_C
NTSTATUS
IrpNotImplementedHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    PAGED_CODE();
    CompleteRequest(Irp, STATUS_NOT_IMPLEMENTED, 0);
    return STATUS_NOT_IMPLEMENTED;
}


///
/// @brief Unload routine
///
/// @param [inout] DriverObject
///
EXTERN_C
void
DriverUnloadRoutine(_In_ PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(DOS_DEVICE_PATH);
    ::IoDeleteSymbolicLink(&symLink);
    ::IoDeleteDevice(DriverObject->DeviceObject);

    ok(L"Device '%s' unloaded\n", DEVICE_NAME);
    return;
}


///
/// @brief
///
/// @param [inout] DriverObject
/// @param [inout] RegistryPath
/// @return NTSTATUS
///
EXTERN_C
NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    g_pDeviceObject = nullptr;

    info(L"Loading '%s'\n", DEVICE_NAME);
    do
    {
        UNICODE_STRING name    = RTL_CONSTANT_STRING(DEVICE_PATH);
        UNICODE_STRING symLink = RTL_CONSTANT_STRING(DOS_DEVICE_PATH);

        for ( auto i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++ )
        {
            DriverObject->MajorFunction[i] = IrpNotImplementedHandler;
        }

        DriverObject->DriverUnload = DriverUnloadRoutine;

        Status = ::IoCreateDevice(
            DriverObject,
            0,
            &name,
            FILE_DEVICE_UNKNOWN,
            FILE_DEVICE_SECURE_OPEN,
            true,
            &g_pDeviceObject);
        if ( !NT_SUCCESS(Status) )
        {
            err(L"Error creating device object (0x%08X)\n", Status);
            break;
        }

        ok(L"device '%s' successfully created\n", DEVICE_NAME);

        Status = ::IoCreateSymbolicLink(&symLink, &name);
        if ( !NT_SUCCESS(Status) )
        {
            err(L"IoCreateSymbolicLink() failed: 0x%08X\n", Status);
            break;
        }

        ok(L"Symlink for '%s' created\n", DEVICE_NAME);

        g_pDeviceObject->Flags |= DO_DIRECT_IO;
        g_pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

        ok(L"Device initialization  for '%s' done\n", DEVICE_NAME);

    } while ( false );

    return Status;
}
