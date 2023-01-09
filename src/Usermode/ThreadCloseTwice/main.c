#include "common_lib.h"
#include "syscall_if.h"
#include "um_lib_helper.h"
// Userprog 8
#include "../../../HAL9000/headers/mutex.h"

static
STATUS
(__cdecl _ThreadFunc)(
    IN_OPT      PVOID       Context
    )
{
    UNREFERENCED_PARAMETER(Context);

    return STATUS_SUCCESS;
}

STATUS
__main(
    DWORD       argc,
    char**      argv
)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    // Userprog 8
    MUTEX mutex;
    STATUS status;

    __try
    {
        status = SyscallMutexInit((UM_HANDLE*)&mutex);

        if (!SUCCEEDED(status)) 
        {
            LOG_FUNC_ERROR("SyscallMutexInit", status);
            __leave;
        }

        status = SyscallMutexAcquire((UM_HANDLE)&mutex);
        if (!SUCCEEDED(status)) 
        {
            LOG_FUNC_ERROR("SyscallMutexInit", status);
            __leave;
        }

        status = SyscallMutexRelease((UM_HANDLE)&mutex);
        if (!SUCCEEDED(status)) 
        {
            LOG_FUNC_ERROR("SyscallMutexInit", status);
            __leave;
        }
    }
    __finally
    {
        LOGL("Finished testing SyscallMutexInit SyscallMutexAcquire SyscallMutexRelease\n");
    }

    return STATUS_SUCCESS;
}