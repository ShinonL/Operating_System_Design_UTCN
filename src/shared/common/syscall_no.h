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

    SyscallIdReserved = SyscallIdMemset + 1
} SYSCALL_ID;
