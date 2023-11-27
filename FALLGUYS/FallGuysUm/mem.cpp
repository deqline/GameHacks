#include "includes.hpp"
#include "structs.hpp"
#include "mem.hpp"

DWORD MemRequests::getProcessId(std::string_view procName)
{
	using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;
	PROCESSENTRY32 processEntry;
	const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL));

	if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
		return NULL;
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	while (Process32Next(snapshot_handle.get(), &processEntry) == TRUE)
	{
		if (procName.compare(processEntry.szExeFile) == 0)
		{
			return processEntry.th32ProcessID;
		}
	}
	return NULL;
}

ULONG64 MemRequests::get_module_resource(uint32_t pid, const char* module_name, const char* resource_name)
{
	READ_WRITE_REQUEST command = { 0 };
	command.pid = pid;
	command.BaseRequest = TRUE;
	command.read = FALSE;
	command.module_name = module_name;
	command.optionalResourceName = resource_name;
	call_hook(&command);

	return command.addy;
}

ULONG64 MemRequests::get_module_base(uint32_t pid, const char* module_name)
{
	READ_WRITE_REQUEST command = { 0 };
	command.pid = pid;
	command.BaseRequest = TRUE;
	command.read = FALSE;
	command.module_name = module_name;
	command.optionalResourceName = "";
	call_hook(&command);

	return command.base_address;
}

bool MemRequests::NtWrite(uint32_t pid, UINT_PTR addy, UINT_PTR src, SIZE_T size)
{
	READ_WRITE_REQUEST command = {};
	command.readOnly = FALSE;
	command.addy = addy;
	command.pid = pid;
	command.BaseRequest = FALSE;
	command.read = FALSE;
	command.bufferAddy = (PVOID)src;
	command.size = sizeof(size);

	call_hook(&command);

	return true;
}

bool MemRequests::PatchProtected(uint32_t pid, UINT_PTR addy, PVOID src, SIZE_T size) {
	READ_WRITE_REQUEST command = {};
	command.readOnly = TRUE;
	command.addy = addy;
	command.pid = pid;
	command.BaseRequest = FALSE;
	command.read = FALSE;
	command.bufferAddy = (PVOID)src;
	command.size = sizeof(size);

	call_hook(&command);
	return true;
}