#pragma once

namespace Hooks {
	class Hook {
		BYTE* oBytes;//original bytes at hkAddress
		BYTE* hkAddress;//address of the instruction to hook
		size_t length;//length of the stolen bytes
		DWORD prevPermissionConstant;
		bool isTramp;
	public:
		Hook(BYTE* hkAddress, BYTE* original_bytes, size_t len);
		void setPagePermission(DWORD permissionConstant);
		void restorePagePermission();
		bool Detour32(BYTE* hkedFunction);
		uintptr_t TrampHook32(BYTE* hkedFunction);
		bool RemoveDetour32();
		bool unTrampHook32(BYTE* gateway, size_t len);
		uintptr_t* getHooked() const;
		size_t getHookLength() const;
	};
}
