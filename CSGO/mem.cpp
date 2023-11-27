#include "pch.h"
#include "mem.h"

uintptr_t mem::findAddy(uintptr_t base, const std::vector<DWORD> offsets)
{
	uintptr_t basePtr = base;
	for (size_t i{}; i < offsets.size(); i++)
	{
		//at the last iteration, we just add the offset and return the address!
		basePtr = *(uintptr_t*)basePtr;//here we dereference the pointer to go to the address stored
		basePtr += offsets.at(i);//we then add the offset to the pointed-to address
	}
	return basePtr;
}

void mem::Nop(BYTE* dst, size_t len)
{
	BYTE* nop = new BYTE[len];
	memset(nop, 0x90, len);

	DWORD oldProtect;
	VirtualProtect(dst, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dst, nop, len);
	VirtualProtect(dst, len, oldProtect, &oldProtect);
}