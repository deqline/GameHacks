#pragma once
#include "mem.h"

namespace NtHook {
	bool NtCallKernelFunc(PVOID);
	NTSTATUS handleHook(PVOID);
	bool unHook();
};