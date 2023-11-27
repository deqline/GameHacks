#pragma once
namespace mem {
	uintptr_t findAddy(uintptr_t base, const std::vector<DWORD> offsets);
	void Nop(BYTE* dst, size_t len);
}