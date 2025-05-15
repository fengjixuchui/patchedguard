#include "ntoskrnl.h"

PIO_STACK_LOCATION g;

uint64_t KeKernelBase = 0;
size_t KeKernelSize = 0;

uint64_t KeGetKernelBase() {
    uint64_t KeKernelBase = 0;

    if (::KeKernelBase)
        return ::KeKernelBase;

    UNICODE_STRING x_PsLoadedModuleList{};
    RtlInitUnicodeString(&x_PsLoadedModuleList, skCrypt(L"PsLoadedModuleList"));

    PLIST_ENTRY PsLoadedModuleList =
        (PLIST_ENTRY)MmGetSystemRoutineAddress(&x_PsLoadedModuleList);

    if (!MmIsAddressValid(PsLoadedModuleList) && PsLoadedModuleList == nullptr)
        return KeKernelBase;

    UNICODE_STRING x_ntoskrnl{};
    RtlInitUnicodeString(&x_ntoskrnl, skCrypt(L"ntoskrnl.exe"));

    PLIST_ENTRY currentEntry = PsLoadedModuleList->Flink;
    while (currentEntry != PsLoadedModuleList && MmIsAddressValid(currentEntry)) {
        PKLDR_DATA_TABLE_ENTRY moduleEntry = CONTAINING_RECORD(
            currentEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (MmIsAddressValid(moduleEntry)) {
            UNICODE_STRING moduleName = moduleEntry->BaseDllName;

            if (!RtlCompareUnicodeString(&moduleEntry->BaseDllName, &x_ntoskrnl,
                FALSE)) {
                KeKernelBase = (uint64_t)moduleEntry->DllBase;
                break;
            }
        }

        currentEntry = currentEntry->Flink;
    }

    ::KeKernelBase = KeKernelBase;
    return KeKernelBase;
}

size_t KeGetKernelSize() {
    size_t KeKernelSize = 0;

    UNICODE_STRING x_PsLoadedModuleList{};
    RtlInitUnicodeString(&x_PsLoadedModuleList, skCrypt(L"PsLoadedModuleList"));

    PLIST_ENTRY PsLoadedModuleList =
        (PLIST_ENTRY)MmGetSystemRoutineAddress(&x_PsLoadedModuleList);

    if (!MmIsAddressValid(PsLoadedModuleList) && PsLoadedModuleList == nullptr)
        return KeKernelSize;

    UNICODE_STRING x_ntoskrnl{};
    RtlInitUnicodeString(&x_ntoskrnl, skCrypt(L"ntoskrnl.exe"));

    PLIST_ENTRY currentEntry = PsLoadedModuleList->Flink;
    while (currentEntry != PsLoadedModuleList && MmIsAddressValid(currentEntry)) {
        PKLDR_DATA_TABLE_ENTRY moduleEntry = CONTAINING_RECORD(
            currentEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (MmIsAddressValid(moduleEntry)) {
            UNICODE_STRING moduleName = moduleEntry->BaseDllName;

            if (!RtlCompareUnicodeString(&moduleEntry->BaseDllName, &x_ntoskrnl,
                FALSE)) {
                KeKernelSize = (uint64_t)moduleEntry->SizeOfImage;
                break;
            }
        }

        currentEntry = currentEntry->Flink;
    }

    ::KeKernelSize = KeKernelSize;
    return KeKernelSize;
}

PKPRCB KeGetCurrentPrcb() {
    PKPRCB pprcb = (PKPRCB)__readgsqword(0x20);
    if (pprcb)
        return pprcb;

    __debugbreak(); // wtf???

    return nullptr;
}

