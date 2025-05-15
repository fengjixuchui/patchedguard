#include <ntifs.h>
#include "ntoskrnl.h"
#include <windef.h>
#include "include/petypes.h"
#include "patchguard.h"

extern "C" NTSTATUS FxDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS Status = STATUS_SUCCESS;

    KeGetKernelBase();
    KeGetKernelSize();

    DisablePatchGuard();

    char buffer[] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3 };
    write_to_read_only_memory(KeBugCheckEx, &buffer, sizeof(buffer));
    
    return Status;
}