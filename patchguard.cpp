#include "patchguard.h"

uint64_t DecryptDpcRoutine(PKTIMER pTimer) {
    __int64 KiWaitNever = 0;
    __int64 KiWaitAlways = 0;

    if (!KiWaitNever || !KiWaitAlways) {
        uint64_t pKiWaitNever = ResloveOffset(o_KiWaitNever);
        uint64_t pKiWaitAlways = ResloveOffset(o_KiWaitAlways);

        if (!pKiWaitNever || !pKiWaitAlways)
            return 0;

        KiWaitNever = *(__int64*)pKiWaitNever;
        KiWaitAlways = *(__int64*)pKiWaitAlways;
    }

    _KDPC* DecryptedDpc =
        (_KDPC*)(KiWaitAlways ^
            _byteswap_uint64(
                (unsigned __int64)pTimer ^
                __ROL8__((__int64)pTimer->Dpc ^ KiWaitNever, KiWaitNever)));

    return (uint64_t)DecryptedDpc;
}

BOOLEAN GetDpcRoutineName(uint64_t DpcAddress, LPWSTR OutBuffer, SIZE_T BufferSize) {
    if (!OutBuffer || !MmIsAddressValid(OutBuffer) || BufferSize < 64) {
        return FALSE;
    }

    KeGetKernelBase();
    if (!KeKernelBase) {
        wcscpy_s(OutBuffer, BufferSize / sizeof(WCHAR), L"INVALID_KERNEL_BASE");
        return FALSE;
    }

    uint64_t RoutineOffset = DpcAddress - KeKernelBase;
    LPCWSTR RoutineName = NULL;
    switch (RoutineOffset) {
    case o_CmpEnableLazyFlushDpcRoutine:
        RoutineName = L"CmpEnableLazyFlushDpcRoutine";
        break;
    case o_CmpLazyFlushDpcRoutine:
        RoutineName = L"CmpLazyFlushDpcRoutine";
        break;
    case o_ExpCenturyDpcRoutine:
        RoutineName = L"ExpCenturyDpcRoutine";
        break;
    case o_ExpTimerDpcRoutine:
        RoutineName = L"ExpTimerDpcRoutine";
        break;
    case o_ExpTimeRefreshDpcRoutine:
        RoutineName = L"ExpTimeRefreshDpcRoutine";
        break;
    case o_ExpTimeZoneDpcRoutine:
        RoutineName = L"ExpTimeZoneDpcRoutine";
        break;
    case o_IopIrpStackProfilerDpcRoutine:
        RoutineName = L"IopIrpStackProfilerDpcRoutine";
        break;
    case o_IopTimerDispatch:
        RoutineName = L"IopTimerDispatch";
        break;
    case o_KiBalanceSetManagerDeferredRoutine:
        RoutineName = L"KiBalanceSetManagerDeferredRoutine";
        break;
    case o_KiDpcDispatch:
        RoutineName = L"KiDpcDispatch";
        break;
    case o_PopThermalZoneDpc:
        RoutineName = L"PopThermalZoneDpc";
        break;
    case o_KiWaitNever:
        RoutineName = L"KiWaitNever";
        break;
    case o_KiWaitAlways:
        RoutineName = L"KiWaitAlways";
        break;
    case o_ExpNextYearDpcRoutine:
        RoutineName = L"ExpNextYearDpcRoutine";
        break;
    case o_HalpTimerDpcRoutine:
        RoutineName = L"HalpTimerDpcRoutine";
        break;
    case o_PopFxResidentTimeoutDpcRoutine:
        RoutineName = L"PopFxResidentTimeoutDpcRoutine";
        break;
    case o_CcScanDpc:
        RoutineName = L"CcScanDpc";
        break;
    case o_PiDrvDbUnloadNodeDpcRoutine:
        RoutineName = L"PiDrvDbUnloadNodeDpcRoutine";
        break;
    case o_CcBcbProfiler:
        RoutineName = L"CcBcbProfiler";
        break;
    case o_PopFxIdleTimeoutDpcRoutine:
        RoutineName = L"PopFxIdleTimeoutDpcRoutine";
        break;
    case o_PopCheckForIdleness:
        RoutineName = L"PopCheckForIdleness";
        break;
    case o_PfSnTraceTimerRoutine:
        RoutineName = L"PfSnTraceTimerRoutine";
        break;
    default:
        RoutineName = L"UNKNOWN_ROUTINE";
        break;
    }

    wcscpy_s(OutBuffer, BufferSize / sizeof(WCHAR), RoutineName);

    return (RoutineName != L"UNKNOWN_ROUTINE");
}

VOID DisablePatchGuard() {
    KAFFINITY active_procs = KeQueryActiveProcessors();

    _disable();
    KeEnterCriticalRegion();

    for (int cpu = 0; active_procs; cpu++, active_procs >>= 1) {
        if (active_procs & 1) {

            KeSetSystemAffinityThread(1ULL << cpu);

            PKTIMER_TABLE pTimerTable = KeGetCurrentTimerTable();
            if (!pTimerTable || !MmIsAddressValid(pTimerTable)) {
                KeLeaveCriticalRegion();
                _enable();
                return;
            }

            for (int wheel = 0; wheel < 2; wheel++) {
                for (int currentTimerEntry = 0; currentTimerEntry < 256;
                    currentTimerEntry++) {
                    PKTIMER_TABLE_ENTRY pkCurrentTimer =
                        &pTimerTable->TimerEntries[wheel][currentTimerEntry];
                    if (!MmIsAddressValid(pkCurrentTimer))
                        continue;

                    for (PLIST_ENTRY pListEntry = pkCurrentTimer->Entry.Flink;
                        pListEntry != &pkCurrentTimer->Entry &&
                        MmIsAddressValid(pListEntry);
                        pListEntry = pListEntry->Flink) {
                        PKTIMER pKEntry =
                            CONTAINING_RECORD(pListEntry, KTIMER, TimerListEntry);
                        if (!MmIsAddressValid(pKEntry))
                            continue;

                        KDPC* DecryptedDPC = (KDPC*)DecryptDpcRoutine(pKEntry);
                        if (!DecryptedDPC || !MmIsAddressValid((PVOID)DecryptedDPC)) {
                            continue;
                        }

                        if (DecryptedDPC->DeferredRoutine < (PVOID)KeKernelBase || DecryptedDPC->DeferredRoutine >= (PVOID)(KeKernelBase + KeKernelSize))
                            continue;

                        WCHAR RoutineName[256] = { 0 };
                        
                        GetDpcRoutineName((uint64_t)DecryptedDPC->DeferredRoutine, RoutineName, sizeof(RoutineName));

                        if (wcscmp(RoutineName, L"CmpEnableLazyFlushDpcRoutine") == 0 ||
                            wcscmp(RoutineName, L"CmpLazyFlushDpcRoutine") == 0 ||
                            wcscmp(RoutineName, L"ExpCenturyDpcRoutine") == 0 ||
                            wcscmp(RoutineName, L"ExpTimeRefreshDpcRoutine") == 0 ||
                            wcscmp(RoutineName, L"ExpTimeZoneDpcRoutine") == 0 ||
                            wcscmp(RoutineName, L"ExpTimerDpcRoutine") == 0 ||
                            wcscmp(RoutineName, L"IopIrpStackProfilerDpcRoutine") == 0 ||
                            wcscmp(RoutineName, L"IopTimerDispatch") == 0 ||
                            wcscmp(RoutineName, L"KiBalanceSetManagerDeferredRoutine") == 0 ||
                            wcscmp(RoutineName, L"KiDpcDispatch") == 0 ||
                            wcscmp(RoutineName, L"PopThermalZoneDpc") == 0) {
                            KeCancelTimer(pKEntry);
                            unsigned char patch[] = { 0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xC3 };
                            write_to_read_only_memory(DecryptedDPC->DeferredRoutine, &patch, sizeof(patch));
                        }
                    }
                }
            }
        }
    }
    KeLeaveCriticalRegion();
    _enable();
}