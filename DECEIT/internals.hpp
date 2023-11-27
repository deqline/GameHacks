#pragma once
#include "defs.hpp"
#include "struct.hpp"

HANDLE hProc;//avoid passing param a thousand times

void SeDbg()
{
	HANDLE hTok;
	TOKEN_PRIVILEGES tkp;

	//Get A token for this process to be able to adjust the token privileges
	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hTok))
		return;
	printf("Current process token %p .\n", hTok);
	//Get privilege LUID
//	Use the LookupPrivilegeName function to convert a LUID to its corresponding string constant.
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
	auto luid_int = *((DWORD64*)&tkp.Privileges[0].Luid);
	printf("Privileges SE_DEBUG_NAME val %llx .\n", luid_int);

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	//update the token privilges for our current process
	AdjustTokenPrivileges(hTok, FALSE, &tkp,0, (PTOKEN_PRIVILEGES)NULL, 0);
}	


PVOID WINAPI NtGetImageBase(HANDLE hProc) {
    PROCESS_BASIC_INFORMATION pbi;

    if(NtQueryInformationProcess(hProc, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL) != 0)
        return NULL;
    
    if(!pbi.PebBaseAddress)
        return NULL;

    printf("PEB %p .\n",pbi.PebBaseAddress); 
	
    PVOID ImageBase = NULL;
    if(!ReadProcessMemory(hProc, (LPVOID)((BYTE*)pbi.PebBaseAddress+offsetof(PEB, Reserved3[1])), &ImageBase, sizeof(PVOID), NULL))
        return NULL;
    printf("IMAGE BASE %p .\n",ImageBase); 
    return ImageBase;	
}

void*  NtGetHandle(ULONG id)
{
	CLIENT_ID ClientId{(HANDLE)id, 0};
	OBJECT_ATTRIBUTES objAttr;
	InitializeObjectAttributes(&objAttr,NULL, 0, NULL, NULL);//no handle inheritance
	HANDLE procHandle;

	if(NtOpenProcess(&procHandle, PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, &objAttr, &ClientId))
		return NULL;
	return procHandle;
}

void* GetProcessBase(PCWSTR process_name) {
    ULONG neededLength = 0;
    ULONG PID = 0;

    //just get needed size -> no Buffer provided neededLength will only be filled
    NtQuerySystemInformation(SystemProcessInformation, NULL, neededLength, &neededLength);

    if(!neededLength)
	return FALSE;

    PVOID buffer = VirtualAlloc(NULL, neededLength, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);//allocate a buffer for the SYSTEM_PROCESS_INFORMATION pointers
    if (!buffer)
	return FALSE;
	
    //Now get the actual pointer table into buffer
    NtQuerySystemInformation(SystemProcessInformation, buffer, neededLength, &neededLength);
	
    //cast the pointer to a SYSTEM_PROCESS_INFORMATION pointer to be able to iterate through the pointers with the correct type 
    PSYSTEM_PROCESS_INFORMATION s1 = (PSYSTEM_PROCESS_INFORMATION)buffer;
    for(;;)
    {
	PWSTR ImageName = s1->ImageName.Buffer; //Is a wchar_t since in ntdll
	//nasty cpp trick to do an ASM jmp if there is no Image name so we can skip the compare
	if (!ImageName)
	    goto next_entry;

	//Compare the image and the provided process name
	if(wcscmp(process_name, ImageName) == 0)
	{
	    PID = HandleToULong(s1->UniqueProcessId);
	    printf("PID %d .\n",PID);
	    VirtualFree(buffer, 0, MEM_RELEASE);
            break;
	}

    next_entry:
		//did we reach the end of the list?
        if (!s1->NextEntryOffset)
            break;
		//add the next entry offset to be able to get to the next system info structure
        s1 = (PSYSTEM_PROCESS_INFORMATION)(reinterpret_cast<BYTE*>(s1)+s1->NextEntryOffset);
    }
     
	 
    VirtualFree(buffer, 0, MEM_RELEASE);
    hProc = NtGetHandle(PID);
    printf("Handle %p .\n",hProc);
    return  NtGetImageBase(hProc);
}

void* GetInnerProcessModuleBase(PCWSTR module_name)
{
	PROCESS_BASIC_INFORMATION pbi;
	ULONG len;

	if (!NT_SUCCESS(NtQueryInformationProcess(
		hProc,
		ProcessBasicInformation,
		&pbi,
		sizeof(pbi),
		&len
	)))
		return NULL;

	PEB _peb{};

	if (!NT_SUCCESS(NtReadVirtualMemory(hProc, (PVOID)pbi.PebBaseAddress, &_peb, sizeof(PEB), NULL)))
		return NULL;
	
	PEB_LDR_DATA pld{};

	if (!NT_SUCCESS(NtReadVirtualMemory(hProc, (PVOID)_peb.Ldr, &pld, sizeof(pld), NULL)))
		return NULL;

	//get the first list entry
	LIST_ENTRY	current_list_entry = (*(LIST_ENTRY*)((BYTE*)&pld+0x10));//InLoadOrderModuleList
	LIST_ENTRY  unchanged_list_entry = (*(LIST_ENTRY*)((BYTE*)&pld + 0x10));
	PWSTR		modName			   = new WCHAR[256];
	UNICODE_STRING BaseName{};

	while (current_list_entry.Flink != unchanged_list_entry.Blink) {

		LDR_DATA_TABLE_ENTRY ldte{};

		//We get the first Flink
		if (!NT_SUCCESS(NtReadVirtualMemory(hProc, (PVOID)current_list_entry.Flink, &ldte, sizeof(ldte), NULL)))
			goto skip;

		BaseName = *(UNICODE_STRING*)((BYTE*)&ldte + 0x58);//BaseDllName

		if (BaseName.Length <= 1)
			goto skip;

		if (!BaseName.Buffer)
			goto skip;

		//if everything is ok read the actual string stored at buffer address in target process
		if (!NT_SUCCESS(NtReadVirtualMemory(hProc, (PVOID)BaseName.Buffer, modName, BaseName.Length*2, NULL)))
			return NULL;

		if (wcscmp(modName, module_name) == 0)
			return ldte.DllBase;

	skip:
		PVOID nextFlink = (*(LIST_ENTRY*)&ldte).Flink;
		if (!NT_SUCCESS(NtReadVirtualMemory(hProc, nextFlink, &current_list_entry, sizeof(LIST_ENTRY), NULL)))
			return NULL;
	}
	return NULL;
}