#include "pch.h"
#include "Hooks.h"
#include "mem.h"

int filter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
	if (code == EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_EXECUTE_HANDLER;
	else
		return EXCEPTION_CONTINUE_SEARCH;
}

Hooks::Hook::Hook(BYTE* hkAddress, BYTE* original_bytes, size_t len)
{
	this->hkAddress = hkAddress;
	this->length = len;
	this->oBytes = new BYTE[len];
	memcpy_s(this->oBytes, len, original_bytes, len);
	this->prevPermissionConstant = NULL;
	this->isTramp = false;
}

void Hooks::Hook::setPagePermission(DWORD permissionConstant)
{
	//structured exception handling (SEH)
	__try {
		VirtualProtect(this->hkAddress, this->length, permissionConstant, &this->prevPermissionConstant);
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		puts("access violation while trying to change the page protection!");
	}
}

void  Hooks::Hook::restorePagePermission()
{
	__try {
		VirtualProtect(this->hkAddress, this->length, this->prevPermissionConstant, &prevPermissionConstant);
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		puts("access violation while trying to change the page protection!");
	}
}

bool Hooks::Hook::Detour32(BYTE* src)
{
	//on x86 a relative jmp is 5 bytes minimum
	//so the one byte jmp instruction and the 4 bytes address
	if (this->length < 5) return false;

	DWORD curProtection;
	//does not have write permissions because they are in execution section
	VirtualProtect(this->hkAddress, this->length, PAGE_EXECUTE_READWRITE, &curProtection);

	mem::Nop(this->hkAddress, this->length);

	//calculate the relative jump offset to jmp from the overwritten first bytes to our hook function
	uintptr_t jmpToAddress = (src - this->hkAddress) - 5;//remove the jump because we dont want to jump 5 bytes after our hook

	*this->hkAddress = 0xE9;//jmp instruction at the first byte of dst

	*(uintptr_t*)(this->hkAddress + 1) = jmpToAddress;//holds an address where the function is not arbitrary bytes 1 byte after the jmp
	VirtualProtect(this->hkAddress, this->length, curProtection, &curProtection);
	return true;
}

uintptr_t Hooks::Hook::TrampHook32(BYTE* hkedFunction)
{
	isTramp = true;
	uintptr_t gateway = (uintptr_t)VirtualAlloc(0, this->length+5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	memcpy_s((BYTE*)gateway, this->length, this->oBytes, this->length);

	uintptr_t jmpBackAddy = ((uintptr_t)this->hkAddress - gateway) - 5;//jmp after the stolen bytes

	*(BYTE*)(gateway + this->length) = 0xE9;//relative x86 jmp
	*(uintptr_t*)(gateway + this->length + 1) = jmpBackAddy;

	Detour32(hkedFunction);

	return gateway;
}

bool Hooks::Hook::RemoveDetour32()
{
	DWORD oldProtect;
	VirtualProtect(this->hkAddress, this->length, PAGE_EXECUTE_READWRITE, &oldProtect);

	memcpy_s(this->hkAddress, this->length, this->oBytes, this->length);

	VirtualProtect(this->hkAddress, this->length, PAGE_EXECUTE_READWRITE, &oldProtect);
	return true;
}

bool Hooks::Hook::unTrampHook32(BYTE* gateway, size_t len)
{
	//restore original bytes at mid-function hook
	RemoveDetour32();

	//free gateway
	if (!VirtualFree(gateway, 0, MEM_RELEASE))
		return false;

	return true;
}

uintptr_t* Hooks::Hook::getHooked() const
{
	return (uintptr_t*)this->hkAddress;
}

size_t Hooks::Hook::getHookLength() const
{
	return this->length;
}