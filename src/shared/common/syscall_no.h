#pragma once

typedef enum _SYSCALL_ID
{
    SyscallIdIdentifyVersion,

    // Thread Management
    SyscallIdThreadExit,
    SyscallIdThreadCreate,
    SyscallIdThreadGetTid,
    SyscallIdThreadWaitForTermination,
    SyscallIdThreadCloseHandle,

    // Process Management
    SyscallIdProcessExit,
    SyscallIdProcessCreate,
    SyscallIdProcessGetPid,
    SyscallIdProcessWaitForTermination,
    SyscallIdProcessCloseHandle,

    // Memory management 
    SyscallIdVirtualAlloc,
    SyscallIdVirtualFree,

    // File management
    SyscallIdFileCreate,
    SyscallIdFileClose,
    SyscallIdFileRead,
    SyscallIdFileWrite,

    // Userprog 4
    SyscallIdMemset,

    // Userprog 6
    SyscallIdDisableSyscalls,

    // Userprog 7
    SyscallIdSetGlobalVariable,
    SyscallIdGetGlobalVariable,

    // Userprog 8
    SyscallIdMutexInit,
    SyscallIdMutexAcquire,
    SyscallIdMutexRelease,

    SyscallIdReserved = SyscallIdMutexRelease + 1
} SYSCALL_ID;
