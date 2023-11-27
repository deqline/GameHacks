#include "../Includes/hk.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING reg_path)
{
	UNREFERENCED_PARAMETER(reg_path);
	UNREFERENCED_PARAMETER(driver_object);

    KernelPrint("Driver Loaded");
	NtHook::NtCallKernelFunc(&NtHook::handleHook);

	return STATUS_SUCCESS;
}