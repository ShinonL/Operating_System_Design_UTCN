#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    // Userprog 4
    STATUS status;
    BYTE address;

    __try
    {
        status = SyscallMemset(&address, sizeof(BYTE), 7);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallMemset", status);
            __leave;
        }

        status = SyscallMemset((PBYTE)0xFFFF800000000000ULL, 1, 0);
        if (SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallMemset", status);
            __leave;
        }
    }
    __finally
    {
        
    }

    return STATUS_SUCCESS;
}