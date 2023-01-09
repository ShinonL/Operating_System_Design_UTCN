#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    /*STATUS status;
    STATUS terminationStatus;*/

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    /*status = SyscallThreadWaitForTermination(0x700, &terminationStatus);
    if (SUCCEEDED(status))
    {
        LOG_ERROR("SyscallThreadWaitForTermination should have failed for invalid handle!\n");
    }*/


    // Userprog 6
    STATUS status1, status2;
    BYTE address;

    __try
    {
        status2 = SyscallDisableSyscalls(TRUE);
        if (!SUCCEEDED(status2))
        {
            LOG_FUNC_ERROR("SyscallDisableSyscalls", status2);
            __leave;
        }

        status1 = SyscallMemset(&address, sizeof(BYTE), 7);
        if (SUCCEEDED(status1))
        {
            LOG_FUNC_ERROR("SyscallMemset", status1);
            __leave;
        }

        status2 = SyscallDisableSyscalls(FALSE);
        if (!SUCCEEDED(status2))
        {
            LOG_FUNC_ERROR("SyscallDisableSyscalls", status2);
            __leave;
        }

        status1 = SyscallMemset(&address, sizeof(BYTE), 7);
        if (!SUCCEEDED(status1))
        {
            LOG_FUNC_ERROR("SyscallMemset", status1);
            __leave;
        }
    }
    __finally
    {
        LOG("Finished testing SyscallDisableSyscalls\n");
    }

    return STATUS_SUCCESS;
}