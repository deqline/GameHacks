#include "../Includes/mem.h"

PVOID get_system_module_base(const char* mod_name) {
	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);

	if (!bytes)//invalid bytes
		return NULL;

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x686178);//last is page name

	if (!modules)
		return NULL;

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status))
		return NULL;

	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	PVOID module_base = 0;
	PVOID module_size = 0;

	for (ULONG i = 0; i < modules->NumberOfModules; i++) {
		if (strcmp((char*)module[i].FullPathName, mod_name) == 0)
		{
			module_base = module[i].ImageBase;
			module_size = (PVOID)module[i].ImageSize;
			break;
		}
	}
	ExFreePoolWithTag(modules, NULL);

	if (module_base <= 0)
		return NULL;

	return module_base;
}

PVOID get_system_module_export(const char* module_name, LPCSTR routine_name)
{
	PVOID lpModule = get_system_module_base(module_name);

	if (!lpModule)
		return NULL;

	return RtlFindExportedRoutineByName(lpModule, routine_name);
}

bool write_memory(void* address, void* buffer, size_t size)
{
	if (!RtlCopyMemory(address, buffer, size))//memcpy
	{
		return false;
	}
	return true;
}

bool write_to_read_only_memory(void* address, void* buffer, ULONG size)
{
	//Allocate in virtual memory a (memory descriptor list) to prepare the address for a mapping (associating with another thing)
	PMDL Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);

	//If we cant prepare a memory descriptor list, then we return
	if (!Mdl)
		return false;

	//if successful the MDL now describes the underlying physical pages which was given as virtual memory in the beginning
	MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);

	//We use kernelMode because we are in a driver, we take the Mdl pointer which was locked to physical memory to transform it to a virtual address
	//returns the starting address of the mapped/associated pages
	PVOID Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);

	//The underlying memory pages at the virtual address can now be read or written.
	MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

	if (!write_memory(Mapping, buffer, size))
		return false;

	//UnAssociate the locked pages
	MmUnmapLockedPages(Mapping, Mdl);//we give it the base virtual address and the memory descriptor list to be cleared
	MmUnlockPages(Mdl);//we unlock the previously locked virtual memory in the MDL
	IoFreeMdl(Mdl);//we free the module descriptor list

	return true;
}

ULONG64 get_resource_base(PEPROCESS proc, UNICODE_STRING resourceName) {
	PPEB pPeb = PsGetProcessPeb(proc);
	if (!pPeb)
		return NULL;

	KAPC_STATE state;
	KeStackAttachProcess(proc, &state);
	PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)pPeb->Ldr;

	if (!pLdr)
	{
		KeUnstackDetachProcess(&state);
		return NULL;
	}

	PLIST_ENTRY list = (PLIST_ENTRY)pLdr->ModuleListMemoryOrder.Flink;//first entry in doubly linked list

	for (; list != &pLdr->ModuleListLoadOrder;
		list = (PLIST_ENTRY)list->Flink//next element in doubly linked list
		)
	{
		PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InMemoryOrderModuleList);

		if (RtlCompareUnicodeString(&pEntry->BaseDllName, &resourceName, TRUE) == NULL)
		{
			ULONG64 baseAddr = (ULONG64)pEntry->DllBase;
			KeUnstackDetachProcess(&state);
			return baseAddr;
		}
	}
	KeUnstackDetachProcess(&state);
	return NULL;
}

ULONG64 get_module_base64(PEPROCESS proc, UNICODE_STRING modName)
{
	PPEB pPeb = PsGetProcessPeb(proc);
	if (!pPeb)
		return NULL;

	KAPC_STATE state;
	KeStackAttachProcess(proc, &state);
	PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)pPeb->Ldr;

	if (!pLdr)
	{
		KeUnstackDetachProcess(&state);
		return NULL;
	}

	for (
		PLIST_ENTRY list = (PLIST_ENTRY)pLdr->ModuleListLoadOrder.Flink;
		list != &pLdr->ModuleListLoadOrder;
		list = (PLIST_ENTRY)list->Flink
		)
	{
		PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);

		if (RtlCompareUnicodeString(&pEntry->BaseDllName, &modName, TRUE) == NULL)
		{
			ULONG64 baseAddr = (ULONG64)pEntry->DllBase;
			KeUnstackDetachProcess(&state);
			return baseAddr;
		}
	}
	KeUnstackDetachProcess(&state);
	return NULL;
}

bool KernelRead(HANDLE pid, uintptr_t addy, PVOID buffer, SIZE_T size)
{
	if (!pid || !addy || !buffer)
		return false;
	SIZE_T bytes = 0;
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS process;
	PsLookupProcessByProcessId((HANDLE)pid, &process);

	status = MmCopyVirtualMemory(
		process,
		(PVOID)addy,
		(PEPROCESS)PsGetCurrentProcess(),
		buffer,
		size,
		KernelMode,
		&bytes);

	if (!NT_SUCCESS(status))
		return false;

	return true;
}

bool KernelWrite(HANDLE pid, uintptr_t addy, PVOID buffer, SIZE_T size)
{
	if (!pid || !addy || !buffer || !size)
		return false;

	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS process;
	PsLookupProcessByProcessId((HANDLE)pid, &process);

	KAPC_STATE state;
	KeStackAttachProcess((PEPROCESS)process, &state);

	MEMORY_BASIC_INFORMATION info;

	status = ZwQueryVirtualMemory(
		ZwCurrentProcess(),
		(PVOID)addy,
		MemoryBasicInformation,
		&info,
		sizeof(info),
		NULL);

	if (!NT_SUCCESS(status))
	{
		KeUnstackDetachProcess(&state);
		return false;
	}

	//is our src memory bigger than the writeTo mem
	if (((uintptr_t)info.BaseAddress + info.RegionSize) < (addy + size))
	{
		KeUnstackDetachProcess(&state);
		return false;
	}

	//is the memory unwritable
	if (!(info.State & MEM_COMMIT) || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
	{
		KeUnstackDetachProcess(&state);
		return false;
	}

	//is the memory writable?
	if ((info.Protect & PAGE_EXECUTE_READWRITE) ||
		(info.Protect & PAGE_EXECUTE_WRITECOPY) ||
		(info.Protect & PAGE_READWRITE) ||
		(info.Protect & PAGE_WRITECOPY))
	{
		KernelPrint("Successfully wrote to memory");
		RtlCopyMemory((PVOID)addy, buffer, size);
		KeUnstackDetachProcess(&state);
		return true;
	}
	KeUnstackDetachProcess(&state);
	return false;
}

NTSTATUS PatchProgram(HANDLE PID, uintptr_t addy, PVOID buffer, SIZE_T size)
{
	PEPROCESS Process;
	KAPC_STATE APC;
	NTSTATUS Status;

	//look for the eprocess structure of the target process using the PID
	if (!NT_SUCCESS(PsLookupProcessByProcessId((PVOID)PID, &Process)))
		return STATUS_INVALID_PARAMETER_1;

	//create variables for the details for the address to patch
	PVOID Address = (PVOID)addy;
	PVOID ProtectedAddress = (PVOID)addy;
	SIZE_T Size = size;
	SIZE_T ProtectedSize = size;

	//allocate a memory pool to store our srcBuffer
	PVOID* Buffer = (PVOID*)ExAllocatePool(NonPagedPool, Size); // Pointer to Allocated Memory
	if (Buffer == NULL) {
		//free EPRocess pointer
		ObfDereferenceObject(Process);

		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	//Make all memory NULL to avoid writing to avoid overwriting memory
	RtlSecureZeroMemory(Buffer, Size);

	__try {
		//copy our passed buffer in the allocated pool buffer
		memcpy(Buffer, buffer, Size);

		//attach our thread to the process address space
		KeStackAttachProcess(Process, &APC);

		ULONG OldProtection;
		//change the protection of the page
		Status = ZwProtectVirtualMemory(ZwCurrentProcess(), &ProtectedAddress, &ProtectedSize, PAGE_EXECUTE_READWRITE, &OldProtection);
		if (!NT_SUCCESS(Status)) {
			KeUnstackDetachProcess(&APC);

			ExFreePool(Buffer);
			ObfDereferenceObject(Process);

			return Status;
		}

		ProtectedAddress = Address;
		ProtectedSize = Size;

		MEMORY_BASIC_INFORMATION info;

		//determines the state, protection, and type of a region of pages within the virtual address space of the subject process.
		Status = ZwQueryVirtualMemory(ZwCurrentProcess(), Address, MemoryBasicInformation, &info, sizeof(MEMORY_BASIC_INFORMATION), NULL);
		if (!NT_SUCCESS(Status)) {
			KeUnstackDetachProcess(&APC);

			ExFreePool(Buffer);
			ObfDereferenceObject(Process);

			return Status;
		}

		//is the memory commitable? if no change the protection back to the old one
		if (!(info.State & MEM_COMMIT)) {
			ZwProtectVirtualMemory(ZwCurrentProcess(), &ProtectedAddress, &ProtectedSize, OldProtection, &OldProtection);
			KeUnstackDetachProcess(&APC);

			ExFreePool(Buffer);
			ObfDereferenceObject(Process);

			Status = STATUS_ACCESS_DENIED;

			return Status;
		}

		//if everything went right then copy our buffer in the usermode address
		RtlCopyMemory(Address, Buffer, Size);

		ZwProtectVirtualMemory(ZwCurrentProcess(), &ProtectedAddress, &ProtectedSize, OldProtection, &OldProtection);
		KeUnstackDetachProcess(&APC);

		Status = STATUS_SUCCESS;
		KernelPrint("Successfully wrote to protected memory %p!", Address);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		KeUnstackDetachProcess(&APC);
	}

	ExFreePool(Buffer);
	ObfDereferenceObject(Process);

	return Status;
}