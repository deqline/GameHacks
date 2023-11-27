#pragma once
#include "includes.h"

#define KernelPrint(x, ...) DbgPrintEx(0, 0, x, __VA_ARGS__)
#pragma comment(lib, "ntoskrnl.lib")

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation, // 0
	SystemProcessorInformation, // 1
	SystemPerformanceInformation, // 2
	SystemTimeOfDayInformation, // 3
	SystemPathInformation, // 4
	SystemProcessInformation, // 5
	SystemCallCountInformation, // 6
	SystemDeviceInformation, // 7
	SystemProcessorPerformanceInformation, // 8
	SystemFlagsInformation, // 9
	SystemCallTimeInformation, // 10
	SystemModuleInformation, // 11
	SystemLocksInformation, // 12
	SystemStackTraceInformation, // 13
	SystemPagedPoolInformation, // 14
	SystemNonPagedPoolInformation, // 15
	SystemHandleInformation, // 16
	SystemObjectInformation, // 17
	SystemPageFileInformation, // 18
	SystemVdmInstemulInformation, // 19
	SystemVdmBopInformation, // 20
	SystemFileCacheInformation, // 21
	SystemPoolTagInformation, // 22
	SystemInterruptInformation, // 23
	SystemDpcBehaviorInformation, // 24
	SystemFullMemoryInformation, // 25
	SystemLoadGdiDriverInformation, // 26
	SystemUnloadGdiDriverInformation, // 27
	SystemTimeAdjustmentInformation, // 28
	SystemSummaryMemoryInformation, // 29
	SystemNextEventIdInformation, // 30
	SystemEventIdsInformation, // 31
	SystemCrashDumpInformation, // 32
	SystemExceptionInformation, // 33
	SystemCrashDumpStateInformation, // 34
	SystemKernelDebuggerInformation, // 35
	SystemContextSwitchInformation, // 36
	SystemRegistryQuotaInformation, // 37
	SystemExtendServiceTableInformation, // 38
	SystemPrioritySeperation, // 39
	SystemPlugPlayBusInformation, // 40
	SystemDockInformation, // 41
	SystemPwrInformation, // 42
	SystemProcessorSpeedInformation, // 43
	SystemCurrentTimeZoneInformation, // 44
	SystemLookasideInformation // 45
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES,
* PRTL_PROCESS_MODULES;

typedef struct _PEB_LDR_DATA {
	ULONG	Length;
	BOOLEAN	Initialized;
	PVOID   SsHandle;
	LIST_ENTRY ModuleListLoadOrder;
	LIST_ENTRY ModuleListMemoryOrder;
	LIST_ENTRY ModuleListInitOrder;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderModuleList;                                    //0x0
	LIST_ENTRY InMemoryOrderModuleList;                                  //0x8
	LIST_ENTRY InInitializationOrderModuleList;                          //0x10
	PVOID DllBase;                                                          //0x18
	PVOID EntryPoint;                                                       //0x1c
	ULONG SizeOfImage;       //in bytes                                               //0x20
	UNICODE_STRING FullDllName;                                     //0x24
	UNICODE_STRING BaseDllName;                                     //0x2c
	ULONG Flags;                                                            //0x34
	USHORT LoadCount;                                                       //0x38
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	PVOID SectionPointer;
	ULONG CheckSum;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	BYTE           Reserved1[16];
	PVOID          Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef void(__stdcall* PPS_POST_PROCESS_INIT_ROUTINE)(void);

typedef struct _PEB {
	BYTE                          Reserved1[2];
	BYTE                          BeingDebugged;
	BYTE                          Reserved2[1];
	PVOID                         Reserved3[2];
	PPEB_LDR_DATA                 Ldr;
	PRTL_USER_PROCESS_PARAMETERS  ProcessParameters;
	PVOID                         Reserved4[3];
	PVOID						  AtlThunkSListPtr;
	PVOID                         Reserved5;
	ULONG                         Reserved6;
	PVOID                         Reserved7;
	ULONG                         Reserved8;
	ULONG						  AtlThunkSListPtr32;
	PVOID						  Reserved9[45];
	BYTE						  Reserved10[96];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE                          Reserved11[128];
	PVOID                         Reserved12[1];
	ULONG                         SessionId;
} PEB, * PPEB;

extern "C" __declspec(dllimport) NTSTATUS NTAPI ZwProtectVirtualMemory(
	IN HANDLE ProcessHandle,
	IN OUT PVOID * UnsafeBaseAddress,
	IN OUT SIZE_T * UnsafeNumberOfBytesToProtect,
	IN ULONG 	NewAccessProtection,
	OUT PULONG 	UnsafeOldAccessProtection
);

extern "C" NTKERNELAPI PVOID NTAPI RtlFindExportedRoutineByName(_In_ PVOID ImageBase, _In_ PCCH RoutineName);

extern "C" NTSTATUS ZwQuerySystemInformation(ULONG InfoClass, PVOID Buffer, ULONG Length, PULONG ReturnLength);

//PEPROCESS is a pointer to an EPROCESS structure -> The EPROCESS structure is an opaque structure that serves as the process object for a process.

extern "C" NTKERNELAPI PPEB PsGetProcessPeb(IN PEPROCESS Process);

extern "C" NTSTATUS NTAPI MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T returnSize
);

//UM stuff
typedef struct _READ_WRITE_REQUEST {
	BOOLEAN read;
	PVOID bufferAddy;
	UINT_PTR addy;
	ULONGLONG size;
	DWORD pid;
	PVOID output;
	BOOLEAN BaseRequest;
	const char* module_name;
	ULONG64 base_address;
	const char* optionalResourceName;
	BOOLEAN readOnly;
} READ_WRITE_REQUEST, * PREAD_WRITE_REQUEST;

extern "C" NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject);