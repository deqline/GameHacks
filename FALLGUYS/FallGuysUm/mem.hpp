#pragma once

namespace MemRequests {
	extern UINT_PTR lastReadAddress;
	template<typename ...Args>
	__int64 call_hook(const Args... arg)
	{
		using NtQueryCompositionSurfaceBinding = __int64(__stdcall*)(Args...);

		//get address of hooked function
		auto hooked_func = (NtQueryCompositionSurfaceBinding)GetProcAddress(LoadLibrary("win32u.dll"), "NtQueryCompositionSurfaceStatistics");

		//return a call to it with the custom template arguments
		return hooked_func(arg...);
	}

	template<class T>
	T NtRead(uint32_t pid, UINT_PTR addy) {
		T response = {};

		READ_WRITE_REQUEST command = {};
		command.addy = addy;
		command.size = sizeof(T);
		command.pid = pid;
		command.BaseRequest = FALSE;
		command.read = TRUE;
		command.output = &response;

		call_hook(&command);

		return response;
	}

	template<typename T>
	T NtMultiRead(uint32_t pid, UINT_PTR base, std::vector<DWORD> offsets)
	{
		UINT_PTR address = base;

		for (int i = 0; i < offsets.size(); i++)
		{
			address = NtRead<UINT_PTR>(pid, address);
			address += offsets[i];
		}
		lastReadAddress = address;
		return NtRead<T>(pid, address);
	}

	DWORD getProcessId(std::string_view procName);
	ULONG64 get_module_resource(uint32_t pid, const char* module_name, const char* resource_name);
	ULONG64 get_module_base(uint32_t pid, const char* module_name);

	bool NtWrite(uint32_t pid, UINT_PTR addy, UINT_PTR src, SIZE_T size);
	bool PatchProtected(uint32_t pid, UINT_PTR addy, PVOID src, SIZE_T size);
}