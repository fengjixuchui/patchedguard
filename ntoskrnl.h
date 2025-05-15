#ifndef ntoskrnl_h
#define ntoskrnl_h

#include <intrin.h>
#include <ntifs.h>
#include <ntddk.h>
#include <stdint.h>
#include <stdbool.h>

#include "include/nttypes.h"
#include "include/skcrypter.h"

extern uint64_t KeKernelBase;
extern size_t KeKernelSize;

uint64_t KeGetKernelBase();
size_t KeGetKernelSize();

PKPRCB KeGetCurrentPrcb();
__forceinline uint64_t KeGetcr3()
{
	return KeGetCurrentPrcb()->ProcessorState.SpecialRegisters.Cr3;
}


#endif