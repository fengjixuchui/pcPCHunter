/***************************************************************************************
* AUTHOR : MZ
* DATE   : 2016-3-18
* MODULE : ZwQueryVirtualMemory.C
* 
* Command: 
*    Source of IOCTRL Sample Driver
*
* Description:
*        Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2010 MZ.
****************************************************************************************/

//#######################################################################################
//# I N C L U D E S
//#######################################################################################

#ifndef CXX_ZWQUERYVIRTUALMEMORY_H
#    include "ZwQueryVirtualMemory.h"
#include "Common.h"
#include "GetSSDTFuncAddress.h"
#endif
typedef struct _MEMORY_SECTION_NAME  {
	UNICODE_STRING Name;
	WCHAR     Buffer[260];
}MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;
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

extern ULONG_PTR  ObjectTableOffsetOf_EPROCESS;
extern ULONG_PTR  PreviousModeOffsetOf_KTHREAD;
extern ULONG_PTR  IndexOffsetOfFunction;

extern ULONG_PTR  ObjectHeaderSize;
extern ULONG_PTR  ObjectTypeOffsetOf_OBJECT_HEADER;
extern ULONG_PTR  HighUserAddress;

extern WIN_VERSION WinVersion;
extern ULONG_PTR LdrInPebOffset;
extern ULONG_PTR ModListInLdrOffset;

extern ULONG_PTR  HighUserAddress;

extern pfnNtQueryVirtualMemory   my_NtQueryVirtualMemoryAddress;

//NTSTATUS
//DriverEntry2(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryString)
//{
//    NTSTATUS        status = STATUS_SUCCESS;
//    UNICODE_STRING  ustrLinkName;
//    UNICODE_STRING  ustrDevName;  
//    PDEVICE_OBJECT  pDevObj;
//    PEPROCESS Process = NULL;
//    HANDLE Id = NULL;
//
//    WinVersion = GetWindowsVersion();
//    InitGlobalVariable();
//    NtQueryVirtualMemoryAddress = (pfnNtQueryVirtualMemory)GetFuncAddress("NtQueryVirtualMemory");
//
//    Process = PsGetCurrentProcess();
//    Id = PsGetProcessId(Process);
//    EnumMoudleByNtQueryVirtualMemory((ULONG)Id);
//    pDriverObj->DriverUnload = DriverUnload;
//
//    return STATUS_SUCCESS;
//}
//
//VOID
//DriverUnload2(IN PDRIVER_OBJECT pDriverObj)
//{    
//    return;
//}
//

//NTSTATUS EnumMoudleByNtQueryVirtualMemory(ULONG ProcessId)
//{
//    NTSTATUS Status;
//    PEPROCESS  Process = NULL;
//    HANDLE    hProcess = NULL;
//    SIZE_T ulRet = 0;
//    WCHAR DosPath[260] = {0};
//
//    if (ProcessId)
//    {
//        Status = PsLookupProcessByProcessId((HANDLE)ProcessId, &Process);
//        if (!NT_SUCCESS(Status))
//        {
//            return Status;
//        }    
//    }
//    if (IsRealProcess(Process))   //判断是否为僵尸进程，我只是判断了对象类型和句柄表是否存在
//    {
//        ObfDereferenceObject(Process);
//        Status = ObOpenObjectByPointer(Process, 
//            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
//            NULL, 
//            GENERIC_ALL, 
//            *PsProcessType, 
//            KernelMode, 
//            &hProcess
//            );
//        if (NT_SUCCESS(Status))
//        {
//            ULONG_PTR ulBase = 0;
//            //改变PreviousMode
//            PETHREAD EThread = PsGetCurrentThread();
//            CHAR PreMode     = ChangePreMode(EThread);   //KernelMode
//            do 
//            {
//                MEMORY_BASIC_INFORMATION mbi = {0};
//                Status = NtQueryVirtualMemoryAddress(hProcess, 
//                    (PVOID)ulBase, 
//                    MemoryBasicInformation, 
//                    &mbi, 
//                    sizeof(MEMORY_BASIC_INFORMATION), 
//                    &ulRet);
//                if (NT_SUCCESS(Status))
//                {    
//                    //如果是Image 再查询SectionName,即FileObject Name
//                    if (mbi.Type==MEM_IMAGE)
//                    {
//                        MEMORY_SECTION_NAME msn = {0};
//                        Status = NtQueryVirtualMemoryAddress(hProcess,
//                            (PVOID)ulBase,
//                            MemorySectionName,
//                            &msn,
//                            sizeof(MEMORY_SECTION_NAME),
//                            &ulRet);
//                        if (NT_SUCCESS(Status)) 
//                        {
//                            DbgPrint("SectionName:%wZ\r\n",&(msn.Name));  
//                            NtPathToDosPathW(msn.Name.Buffer,DosPath);
//                            DbgPrint("DosName:%S\r\n",DosPath);
//                        }
//                    }
//                    ulBase += mbi.RegionSize;
//                }
//                else ulBase += PAGE_SIZE;    
//            } while (ulBase < (ULONG_PTR)HighUserAddress);
//            NtClose(hProcess);
//            RecoverPreMode(EThread,PreMode);
//        }
//    }
//    return Status;
//}
//

