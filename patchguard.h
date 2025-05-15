#ifndef patchguard_h
#define patchguard_h

#include <intrin.h>
#include <ntifs.h>
#include <ntddk.h>
#include <stdint.h>
#include <stdbool.h>

#include "include/nttypes.h"
#include "include/skcrypter.h"
#include "include/hexrays.hpp"

#include "ntoskrnl.h"

const uint64_t o_CmpEnableLazyFlushDpcRoutine = 0x306320;
const uint64_t o_CmpLazyFlushDpcRoutine = 0x337E50;
const uint64_t o_ExpCenturyDpcRoutine = 0x306740;
const uint64_t o_ExpTimerDpcRoutine = 0x33A590;
const uint64_t o_ExpTimeRefreshDpcRoutine = 0x306860;
const uint64_t o_ExpTimeZoneDpcRoutine = 0x395370;
const uint64_t o_IopIrpStackProfilerDpcRoutine = 0x3063F0;
const uint64_t o_IopTimerDispatch = 0x501860;
const uint64_t o_KiBalanceSetManagerDeferredRoutine = 0x306980;
const uint64_t o_KiDpcDispatch = 0xA39270;
const uint64_t o_PopThermalZoneDpc = 0x2010F0;
const uint64_t o_ExpNextYearDpcRoutine = 0x5b3070;
const uint64_t o_HalpTimerDpcRoutine = 0x33d110;
const uint64_t o_PopFxResidentTimeoutDpcRoutine = 0x3367d0;
const uint64_t o_CcScanDpc = 0x32ef00;
const uint64_t o_PiDrvDbUnloadNodeDpcRoutine = 0x33e0b0;
const uint64_t o_CcBcbProfiler = 0x3dc0a0;
const uint64_t o_PopFxIdleTimeoutDpcRoutine = 0x3a63e0;
const uint64_t o_PopCheckForIdleness = 0x2ea7f0;
const uint64_t o_PfSnTraceTimerRoutine = 0x2e6180;

const uint64_t o_KiWaitNever = 0xCFC808;
const uint64_t o_KiWaitAlways = 0xCFCA08;

__forceinline uint64_t ResloveOffset(uint64_t o_Offset)
{
	return KeGetKernelBase() + o_Offset;
}
__forceinline PKTIMER_TABLE KeGetCurrentTimerTable()
{
	return &KeGetCurrentPrcb()->TimerTable;
}
__forceinline bool write_to_read_only_memory(void* address, void* buffer, size_t size)
{
    PMDL Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);

    if (!Mdl)
        return false;

    MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);
    PVOID Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
    MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

    memcpy(Mapping, buffer, size);

    MmUnmapLockedPages(Mapping, Mdl);
    MmUnlockPages(Mdl);
    IoFreeMdl(Mdl);

    return true;
}
uint64_t DecryptDpcRoutine(PKTIMER pTimer);
BOOLEAN GetDpcRoutineName(uint64_t DpcAddress, LPWSTR OutBuffer, SIZE_T BufferSize);

VOID DisablePatchGuard();



#endif