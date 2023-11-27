#pragma once
#include "defs.hpp"

BOOL ReadStruct(DWORD_PTR struct_addy, void* buf , SIZE_T struct_sz) {

   	NTSTATUS stat = NtReadVirtualMemory(hProc, (PVOID)struct_addy, buf, struct_sz, NULL);	
	if(!NT_SUCCESS(stat))
		return FALSE;
	else
		return TRUE;
}

//this will only get called when there are no more offsets
template<typename T>
T read(DWORD_PTR thing) {
	T ret = 0;
	NtReadVirtualMemory(hProc, (PVOID)thing, &ret , sizeof(T), NULL);	
	return ret; 
}

//recursive function	
template<typename R, typename ... T>
R read(DWORD_PTR base, DWORD_PTR offset, T... offsets) {
	//at each call base will be [base]+offset
	auto ptr = read<DWORD_PTR>(base);
	//we update the base for the next call to be the first dereference -> base]+offset and we pass along the rest of the offsets
	return read<R>(ptr+offset, offsets...);
}