#pragma once
#include "defs.h"

PVOID get_system_module_base(const char* mod_name);
PVOID get_system_module_export(const char* module_name, LPCSTR routine_name);
bool write_memory(void* address, void* buffer, size_t size);
bool write_to_read_only_memory(void* address, void* buffer, ULONG size);
ULONG64 get_module_base64(PEPROCESS proc, UNICODE_STRING modName);
ULONG64 get_resource_base(PEPROCESS proc, UNICODE_STRING resourceName);
bool KernelRead(HANDLE pid, uintptr_t addy, PVOID buffer, SIZE_T size);
bool KernelWrite(HANDLE pid, uintptr_t addy, PVOID buffer, SIZE_T size);
NTSTATUS PatchProgram(HANDLE PID, uintptr_t addy, PVOID buffer, SIZE_T size);