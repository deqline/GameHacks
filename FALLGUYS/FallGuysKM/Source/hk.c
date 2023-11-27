#include "../Includes/hk.h"

#define KERNEL_DEBUG FALSE

bool NtHook::NtCallKernelFunc(PVOID kernelFnAddy)
{
	if (!kernelFnAddy)
		return false;

	PVOID function = (PVOID)(get_system_module_export(
		"\\SystemRoot\\System32\\drivers\\dxgkrnl.sys",
		"NtQueryCompositionInputSink"
	));

	if (!function)
		return false;

	BYTE payload[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	BYTE shell_code[] = { 0x48, 0xB8 };
	BYTE shell_code_end[] = { 0x50, 0xC3 };
	uintptr_t hookAddress = reinterpret_cast<uintptr_t>(kernelFnAddy);

	uintptr_t relJmp = (hookAddress - (uintptr_t)function) + 5;

	if (KERNEL_DEBUG) {
		KernelPrint("[+] Function offset 0x%p", function);
		KernelPrint("[+] HookFunction offset 0x%p", kernelFnAddy);
	}
	RtlSecureZeroMemory(&payload, sizeof(payload));

	memcpy((PVOID)((ULONG_PTR)payload), &shell_code, sizeof(shell_code));
	memcpy((PVOID)((ULONG_PTR)payload + sizeof(shell_code)), &hookAddress, sizeof(uintptr_t));
	memcpy((PVOID)((ULONG_PTR)payload + sizeof(shell_code) + sizeof(kernelFnAddy)), &shell_code_end, sizeof(shell_code_end));

	write_to_read_only_memory(function, &payload, sizeof(payload));

	return true;
}

NTSTATUS NtHook::handleHook(PVOID lParam)
{
	READ_WRITE_REQUEST* command = (READ_WRITE_REQUEST*)lParam;

	if (KERNEL_DEBUG) {
		KernelPrint("[+] Handling Hook Calls: Read: %x BaseRequest: %x", command->read, command->BaseRequest);
	}

	if (command->BaseRequest)
	{
		ANSI_STRING s;
		UNICODE_STRING modName;

		RtlInitAnsiString(&s, command->module_name);
		RtlAnsiStringToUnicodeString(&modName, &s, TRUE);

		PEPROCESS process;
		PsLookupProcessByProcessId((HANDLE)command->pid, &process);
		ULONG64 base_address64 = get_module_base64(process, modName);
		command->base_address = base_address64;

		if (strlen(command->optionalResourceName) > 0)
		{
			ANSI_STRING res;
			UNICODE_STRING resName;

			RtlInitAnsiString(&res, command->optionalResourceName);
			RtlAnsiStringToUnicodeString(&resName, &res, TRUE);

			command->addy = get_resource_base(process, resName);
			RtlFreeUnicodeString(&resName);
		}

		RtlFreeUnicodeString(&modName);
	}

	//write
	else if (!command->read && !command->BaseRequest) {
		if (command->addy < 0x7FFFFFFFFFFF && command->addy > 0)
		{
			PVOID kernelBuffer = ExAllocatePool(NonPagedPool, command->size);

			if (!kernelBuffer)
				return STATUS_UNSUCCESSFUL;

			if (!memcpy(kernelBuffer, command->bufferAddy, command->size))
			{
				return STATUS_UNSUCCESSFUL;
			}

			PEPROCESS process;
			PsLookupProcessByProcessId((HANDLE)command->pid, &process);
			if (!command->readOnly) {
				if (!KernelWrite((HANDLE)command->pid, command->addy, kernelBuffer, command->size))
					return STATUS_UNSUCCESSFUL;
			}
			else {
				PatchProgram(reinterpret_cast<HANDLE>(command->pid), command->addy, command->bufferAddy, command->size);
			}

			ExFreePool(kernelBuffer);
		}
	}

	if (command->read && !command->BaseRequest) {
		if (command->addy < 0x7FFFFFFFFFFF && command->addy > 0)
		{
			if (!KernelRead((HANDLE)command->pid, command->addy, command->output, command->size))
				return STATUS_UNSUCCESSFUL;
		}

		return STATUS_SUCCESS;
	}
}

bool NtHook::unHook()
{
	//NtOpenCompositionSurfaceDirtyRegion
	PVOID function = (PVOID)(get_system_module_export(
		"\\SystemRoot\\System32\\drivers\\dxgkrnl.sys",
		"NtQueryCompositionSurfaceStatistics"
	));

	//couldn't hook func
	if (!function)
		return false;

	/*
	mov     qword ptr [rsp+8],rbx
	fffff80f`010170d5 56              push    rsi
	fffff80f`010170d6 57              push    rdi
	fffff80f`010170d7 4154            push    r12
	fffff80f`010170d9 4156            push    r14
	fffff80f`010170db 4157            push    r15
	fffff80f`010170dd 4881ec80000000  sub     rsp,80h
	*/

	BYTE original[] = { 0x48, 0x89, 0x5c, 0x24, 0x08, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xec, 0x80, 0x00, 0x00, 0x00 };

	if (!write_to_read_only_memory(function, original, sizeof(original))) {
		KernelPrint("[-] Failed unhooking kernel");
		return false;
	}
	return true;
}