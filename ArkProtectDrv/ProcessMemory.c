#include "ProcessMemory.h"
#include "GetSSDTFuncAddress.h"
#include <ntifs.h>
extern DYNAMIC_DATA	g_DynamicData;
typedef struct _MEMORY_BASIC_INFORMATION {
	PVOID       BaseAddress;           //查询内存块所占的第一个页面基地址
	PVOID       AllocationBase;        //内存块所占的第一块区域基地址，小于等于BaseAddress，
	DWORD       AllocationProtect;     //区域被初次保留时赋予的保护属性
	SIZE_T      RegionSize;            //从BaseAddress开始，具有相同属性的页面的大小，
	DWORD       State;                 //页面的状态，有三种可能值MEM_COMMIT、MEM_FREE和MEM_RESERVE
	DWORD       Protect;               //页面的属性，其可能的取值与AllocationProtect相同
	DWORD       Type;                  //该内存块的类型，有三种可能值：MEM_IMAGE、MEM_MAPPED和MEM_PRIVATE
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
typedef NTSTATUS
(*pfnNtQueryVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress,
MEMORY_INFORMATION_CLASS MemoryInformationClass,
PVOID MemoryInformation,
SIZE_T MemoryInformationLength,
PSIZE_T ReturnLength);
extern pfnNtQueryVirtualMemory   my_NtQueryVirtualMemoryAddress;
//
//typedef enum _MEMORY_INFORMATION_CLASS {
//	MemoryBasicInformation
//} MEMORY_INFORMATION_CLASS; 
//typedef struct _MEMORY_BASIC_INFORMATION {
//	PVOID BaseAddress;
//	PVOID AllocationBase;
//	ULONG AllocationProtect;
//	SIZE_T RegionSize;
//	ULONG State;
//	ULONG Protect;
//	ULONG Type;
//} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
/************************************************************************
*  Name : APEnumProcessMemoryByZwQueryVirtualMemory
*  Param: EProcess			      进程结构体
*  Param: pmi			          ring3内存
*  Param: MemoryCount
*  Ret  : NTSTATUS
*  
************************************************************************/
NTSTATUS
APEnumProcessMemoryByZwQueryVirtualMemory(IN PEPROCESS EProcess, OUT PPROCESS_MEMORY_INFORMATION pmi, IN UINT32 MemoryCount)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	HANDLE   ProcessHandle = NULL;

	Status = ObOpenObjectByPointer(EProcess,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL,
		GENERIC_ALL,
		*PsProcessType,
		KernelMode,
		&ProcessHandle);
	if (NT_SUCCESS(Status))
	{
		UINT_PTR BaseAddress = 0;

#ifdef _WIN64

		// 处理Wow64程序
		if (PsGetProcessWow64Process(EProcess))
		{
			g_DynamicData.MaxUserSpaceAddress = 0x7FFFFFFF;
		}

#endif // _WIN64

		while ( BaseAddress < g_DynamicData.MaxUserSpaceAddress)
		{
			MEMORY_BASIC_INFORMATION  mbi = { 0 };
			SIZE_T					  ReturnLength = 0;
			my_NtQueryVirtualMemoryAddress = (pfnNtQueryVirtualMemory)GetFuncAddress("NtQueryVirtualMemory");
			if (my_NtQueryVirtualMemoryAddress == NULL)
			{
				return Status;
			}
			Status = my_NtQueryVirtualMemoryAddress(ProcessHandle, (PVOID)BaseAddress, MemoryBasicInformation,
				&mbi, sizeof(MEMORY_BASIC_INFORMATION), &ReturnLength);
			if (NT_SUCCESS(Status))
			{
				if (MemoryCount > pmi->NumberOfMemories)
				{
					pmi->MemoryEntry[pmi->NumberOfMemories].BaseAddress = BaseAddress;
					pmi->MemoryEntry[pmi->NumberOfMemories].RegionSize = mbi.RegionSize;
					pmi->MemoryEntry[pmi->NumberOfMemories].Protect = mbi.Protect;
					pmi->MemoryEntry[pmi->NumberOfMemories].State = mbi.State;
					pmi->MemoryEntry[pmi->NumberOfMemories].Type = mbi.Type;
				}

				pmi->NumberOfMemories++;
				BaseAddress += mbi.RegionSize;
			}
			else
			{
				BaseAddress += PAGE_SIZE;
			}
			Status = STATUS_SUCCESS;
		}

		ZwClose(ProcessHandle);

#ifdef _WIN64

		if (PsGetProcessWow64Process(EProcess))
		{
			g_DynamicData.MaxUserSpaceAddress = 0x000007FFFFFFFFFF;
		}

#endif // _WIN64

	}
	return Status;
}



/************************************************************************
*  Name : APEnumProcessMemory
*  Param: ProcessId			      进程Id
*  Param: OutputBuffer            ring3内存
*  Param: OutputLength
*  Ret  : NTSTATUS
*  枚举进程模块
************************************************************************/
NTSTATUS
APEnumProcessMemory(IN UINT32 ProcessId, OUT PVOID OutputBuffer, IN UINT32 OutputLength)
{
	NTSTATUS  Status = STATUS_UNSUCCESSFUL;

	PPROCESS_MEMORY_INFORMATION pmi = (PPROCESS_MEMORY_INFORMATION)OutputBuffer;
	UINT32    ModuleCount = (OutputLength - sizeof(PROCESS_MEMORY_INFORMATION)) / sizeof(PROCESS_MEMORY_ENTRY_INFORMATION);
	PEPROCESS EProcess = NULL;

	if (ProcessId == 0)
	{
		return Status;
	}
	else
	{
		Status = PsLookupProcessByProcessId((HANDLE)ProcessId, &EProcess);
	}

	if (NT_SUCCESS(Status) && APIsValidProcess(EProcess))
	{
		Status = APEnumProcessMemoryByZwQueryVirtualMemory(EProcess, pmi, ModuleCount);
		if (NT_SUCCESS(Status))
		{
			if (ModuleCount >= pmi->NumberOfMemories)
			{
				Status = STATUS_SUCCESS;
			}
			else
			{
				Status = STATUS_BUFFER_TOO_SMALL;	// 给ring3返回内存不够的信息
			}
		}
	}

	if (EProcess)
	{
		ObDereferenceObject(EProcess);
	}

	return Status;
}