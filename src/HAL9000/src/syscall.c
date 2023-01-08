#include "HAL9000.h"
#include "syscall.h"
#include "gdtmu.h"
#include "syscall_defs.h"
#include "syscall_func.h"
#include "syscall_no.h"
#include "mmu.h"
#include "process_internal.h"
#include "dmp_cpu.h"
// Userprog 1
#include "thread_internal.h"
// VM 3
#include "vmm.h"

extern void SyscallEntry();

#define SYSCALL_IF_VERSION_KM       SYSCALL_IMPLEMENTED_IF_VERSION

void
SyscallHandler(
    INOUT   COMPLETE_PROCESSOR_STATE    *CompleteProcessorState
    )
{
    SYSCALL_ID sysCallId;
    PQWORD pSyscallParameters;
    PQWORD pParameters;
    STATUS status;
    REGISTER_AREA* usermodeProcessorState;

    ASSERT(CompleteProcessorState != NULL);

    // It is NOT ok to setup the FMASK so that interrupts will be enabled when the system call occurs
    // The issue is that we'll have a user-mode stack and we wouldn't want to receive an interrupt on
    // that stack. This is why we only enable interrupts here.
    ASSERT(CpuIntrGetState() == INTR_OFF);
    CpuIntrSetState(INTR_ON);

    LOG_TRACE_USERMODE("The syscall handler has been called!\n");

    status = STATUS_SUCCESS;
    pSyscallParameters = NULL;
    pParameters = NULL;
    usermodeProcessorState = &CompleteProcessorState->RegisterArea;

    __try
    {
        if (LogIsComponentTraced(LogComponentUserMode))
        {
            DumpProcessorState(CompleteProcessorState);
        }

        // Check if indeed the shadow stack is valid (the shadow stack is mandatory)
        pParameters = (PQWORD)usermodeProcessorState->RegisterValues[RegisterRbp];
        status = MmuIsBufferValid(pParameters, SHADOW_STACK_SIZE, PAGE_RIGHTS_READ, GetCurrentProcess());
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("MmuIsBufferValid", status);
            __leave;
        }

        sysCallId = usermodeProcessorState->RegisterValues[RegisterR8];

        LOG_TRACE_USERMODE("System call ID is %u\n", sysCallId);

        // The first parameter is the system call ID, we don't care about it => +1
        pSyscallParameters = (PQWORD)usermodeProcessorState->RegisterValues[RegisterRbp] + 1;

        // Dispatch syscalls
        switch (sysCallId)
        {
        case SyscallIdIdentifyVersion:
            status = SyscallValidateInterface((SYSCALL_IF_VERSION)*pSyscallParameters);
            break;
        // STUDENT TODO: implement the rest of the syscalls
        // Userprog 1
        case SyscallIdProcessExit:
            status = SyscallProcessExit((STATUS)pSyscallParameters[0]);
            break;
        // Userprog 1
        case SyscallIdThreadExit:
            SyscallThreadExit((STATUS)pSyscallParameters[0]);
            break;
        // Userprog 2
        case SyscallIdFileWrite:
            SyscallFileWrite(
                (UM_HANDLE)pSyscallParameters[0],
                (PVOID)pSyscallParameters[1],
                (QWORD)pSyscallParameters[2],
                (QWORD*)pSyscallParameters[3]);
            break;
        // Userprog 4
        case SyscallIdMemset:
            SyscallMemset(
                (PBYTE)pSyscallParameters[0],
                (DWORD)pSyscallParameters[1],
                (BYTE)pSyscallParameters[2]);
            break;
        // VM 3
        case SyscallIdVirtualAlloc:
            SyscallVirtualAlloc(
                (PVOID)pSyscallParameters[0],
                (QWORD)pSyscallParameters[1],
                (VMM_ALLOC_TYPE)pSyscallParameters[2],
                (PAGE_RIGHTS)pSyscallParameters[3],
                (UM_HANDLE)pSyscallParameters[4],
                (QWORD)pSyscallParameters[5],
                (PVOID*)pSyscallParameters[6]);
            break;
        // Userprog 5
        case SyscallIdProcessCreate:
            SyscallProcessCreate(
                (char*)pSyscallParameters[0],
                (QWORD)pSyscallParameters[1],
                (char*)pSyscallParameters[2],
                (QWORD)pSyscallParameters[3],
                (UM_HANDLE*)pSyscallParameters[4]);
            break;
        default:
            LOG_ERROR("Unimplemented syscall called from User-space!\n");
            status = STATUS_UNSUPPORTED;
            break;
        }

    }
    __finally
    {
        LOG_TRACE_USERMODE("Will set UM RAX to 0x%x\n", status);

        usermodeProcessorState->RegisterValues[RegisterRax] = status;

        CpuIntrSetState(INTR_OFF);
    }
}

void
SyscallPreinitSystem(
    void
    )
{

}

STATUS
SyscallInitSystem(
    void
    )
{
    return STATUS_SUCCESS;
}

STATUS
SyscallUninitSystem(
    void
    )
{
    return STATUS_SUCCESS;
}

void
SyscallCpuInit(
    void
    )
{
    IA32_STAR_MSR_DATA starMsr;
    WORD kmCsSelector;
    WORD umCsSelector;

    memzero(&starMsr, sizeof(IA32_STAR_MSR_DATA));

    kmCsSelector = GdtMuGetCS64Supervisor();
    ASSERT(kmCsSelector + 0x8 == GdtMuGetDS64Supervisor());

    umCsSelector = GdtMuGetCS32Usermode();
    /// DS64 is the same as DS32
    ASSERT(umCsSelector + 0x8 == GdtMuGetDS32Usermode());
    ASSERT(umCsSelector + 0x10 == GdtMuGetCS64Usermode());

    // Syscall RIP <- IA32_LSTAR
    __writemsr(IA32_LSTAR, (QWORD) SyscallEntry);

    LOG_TRACE_USERMODE("Successfully set LSTAR to 0x%X\n", (QWORD) SyscallEntry);

    // Syscall RFLAGS <- RFLAGS & ~(IA32_FMASK)
    __writemsr(IA32_FMASK, RFLAGS_INTERRUPT_FLAG_BIT);

    LOG_TRACE_USERMODE("Successfully set FMASK to 0x%X\n", RFLAGS_INTERRUPT_FLAG_BIT);

    // Syscall CS.Sel <- IA32_STAR[47:32] & 0xFFFC
    // Syscall DS.Sel <- (IA32_STAR[47:32] + 0x8) & 0xFFFC
    starMsr.SyscallCsDs = kmCsSelector;

    // Sysret CS.Sel <- (IA32_STAR[63:48] + 0x10) & 0xFFFC
    // Sysret DS.Sel <- (IA32_STAR[63:48] + 0x8) & 0xFFFC
    starMsr.SysretCsDs = umCsSelector;

    __writemsr(IA32_STAR, starMsr.Raw);

    LOG_TRACE_USERMODE("Successfully set STAR to 0x%X\n", starMsr.Raw);
}

// SyscallIdIdentifyVersion
// Userprog 1 - era deja implementat
STATUS
SyscallValidateInterface(
    IN  SYSCALL_IF_VERSION          InterfaceVersion
)
{
    LOG_TRACE_USERMODE("Will check interface version 0x%x from UM against 0x%x from KM\n",
        InterfaceVersion, SYSCALL_IF_VERSION_KM);

    if (InterfaceVersion != SYSCALL_IF_VERSION_KM)
    {
        LOG_ERROR("Usermode interface 0x%x incompatible with KM!\n", InterfaceVersion);
        return STATUS_INCOMPATIBLE_INTERFACE;
    }

    return STATUS_SUCCESS;
}

// STUDENT TODO: implement the rest of the syscalls

// Userprog 1 - same as project and LAB
STATUS
SyscallProcessExit(
    IN      STATUS                  ExitStatus
) {
    PPROCESS pProcess = GetCurrentProcess();

    ProcessTerminate(pProcess);
    pProcess->TerminationStatus = ExitStatus;

    return STATUS_SUCCESS;
}

// Userprog 1 - same as project and LAB
STATUS
SyscallThreadExit(
    IN  STATUS                      ExitStatus
) {
    ThreadExit(ExitStatus);

    return STATUS_SUCCESS;
}

// Userprog 2
STATUS
SyscallFileWrite(
    IN  UM_HANDLE                   FileHandle,
    IN_READS_BYTES(BytesToWrite)
    PVOID                       Buffer,
    IN  QWORD                       BytesToWrite,
    OUT QWORD* BytesWritten
) {
    UNREFERENCED_PARAMETER(BytesWritten);

    if (FileHandle != UM_FILE_HANDLE_STDOUT) {
        return STATUS_INVALID_PARAMETER1;
    }

    if (!SUCCEEDED(MmuIsBufferValid(Buffer, BytesToWrite, PAGE_RIGHTS_READ, GetCurrentProcess())) || ((char*) Buffer)[BytesToWrite - 1] != 0) {
        return STATUS_INVALID_BUFFER;
    }

    if (strlen((char*) Buffer) != BytesToWrite || BytesToWrite < 0) {
        return STATUS_INVALID_PARAMETER3;
    }

    LOG("%s\n", Buffer);
    *BytesWritten = ((QWORD) strlen((char*) Buffer)) + 1;

    return STATUS_SUCCESS;
}

// Userprog 4
STATUS
SyscallMemset(
    OUT_WRITES(BytesToWrite)    PBYTE   Address,
    IN                          DWORD   BytesToWrite,
    IN                          BYTE    ValueToWrite
) {
    if (!SUCCEEDED(MmuIsBufferValid(Address, BytesToWrite, PAGE_RIGHTS_WRITE, GetCurrentProcess()))) {
        return STATUS_INVALID_BUFFER;
    }

    memset(Address, ValueToWrite, BytesToWrite);

    return STATUS_SUCCESS;
}

// VM 3
STATUS
SyscallVirtualAlloc(
    IN_OPT      PVOID                   BaseAddress,
    IN          QWORD                   Size,
    IN          VMM_ALLOC_TYPE          AllocType,
    IN          PAGE_RIGHTS             PageRights,
    IN_OPT      UM_HANDLE               FileHandle,
    IN_OPT      QWORD                   Key,
    OUT         PVOID* AllocatedAddress
) {
    UNREFERENCED_PARAMETER(Key);

    if (Size < 0) {
        return STATUS_INVALID_PARAMETER2;
    }

    if (!SUCCEEDED(MmuIsBufferValid(BaseAddress, Size, PageRights, GetCurrentProcess()))) {
        return STATUS_INVALID_PARAMETER1;
    }

    if (FileHandle == UM_INVALID_HANDLE_VALUE) {
        return STATUS_INVALID_PARAMETER5;
    }

    PPROCESS pProcess = GetCurrentProcess();
    *AllocatedAddress = VmmAllocRegionEx(BaseAddress, Size, AllocType, PageRights, FALSE, NULL, pProcess->VaSpace, pProcess->PagingData, NULL);

    return STATUS_SUCCESS;
}

// Userprog 5
STATUS
SyscallProcessCreate(
    IN_READS_Z(PathLength)
    char* ProcessPath,
    IN          QWORD               PathLength,
    IN_READS_OPT_Z(ArgLength)
    char* Arguments,
    IN          QWORD               ArgLength,
    OUT         UM_HANDLE* ProcessHandle
)
{
    if (PathLength <= 0) {
        return STATUS_INVALID_PARAMETER2;
    }

    if (ProcessPath == NULL || strlen(ProcessPath) != PathLength) {
        return STATUS_INVALID_PARAMETER1;
    }

    if (ArgLength < 0) {
        return STATUS_INVALID_PARAMETER4;
    }

    if (Arguments == NULL || strlen(Arguments) != ArgLength) {
        return STATUS_INVALID_PARAMETER3;
    }
    
    char absolutePath[MAX_PATH];
    if (ProcessPath == strrchr(ProcessPath, '\\')) {
        sprintf(absolutePath, "C:\\Applications\\%s", ProcessPath);
    } else {
        strcpy(absolutePath, ProcessPath);
    }

    PPROCESS pProcess;
    STATUS status = ProcessCreate(absolutePath, Arguments, &pProcess);

    if (!SUCCEEDED(status)) {
        *ProcessHandle = UM_INVALID_HANDLE_VALUE;
        return STATUS_UNSUCCESSFUL;
    }

    *ProcessHandle = pProcess->Id;

    return STATUS_SUCCESS;
}