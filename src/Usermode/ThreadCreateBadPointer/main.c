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

    // Userprog 7
    STATUS status;
    QWORD value;

    __try
    {
        LOG("bEFORE SET\n");
        status = SyscallSetGlobalVariable("cool", sizeof("cool"), 0x300);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallSetGlobalVariable", status);
            __leave;
        }

        LOG("after SET\n");
        status = SyscallGetGlobalVariable("Cool", sizeof("Cool"), &value);
        if (SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallGetGlobalVariable", status);
            __leave;
        }

        LOG("after failed gET\n");
        status = SyscallGetGlobalVariable("cool", sizeof("cool"), &value);
        if (!SUCCEEDED(status))
        {
            LOG_FUNC_ERROR("SyscallGetGlobalVariable", status);
            __leave;
        }
        LOG("after succ gET\n");
    }
    __finally
    {
        LOG("Finished testing SyscallSetGlobalVariable and SyscallGetGlobalVariable\n");
    }

    return STATUS_SUCCESS;
}