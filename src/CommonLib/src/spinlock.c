#include "common_lib.h"
#include "lock_common.h"

#ifndef _COMMONLIB_NO_LOCKS_

void
SpinlockInit(
    OUT         PSPINLOCK       Lock
    )
{
    ASSERT(NULL != Lock);

    memzero(Lock, sizeof(SPINLOCK));

    _InterlockedExchange8(&Lock->State, LOCK_FREE);
}

/*
* 1. a:
* The second parameter holds the saved state of the interrupt system returned by the function.
* We need this OUT parameter because the SpinlockAcquire disables the interrupt system.
*/
void
SpinlockAcquire(
    INOUT       PSPINLOCK       Lock,
    OUT         INTR_STATE*     IntrState
    )
{
    PVOID pCurrentCpu;

    ASSERT(NULL != Lock);
    ASSERT(NULL != IntrState);

    *IntrState = CpuIntrDisable();

    pCurrentCpu = CpuGetCurrent();

    ASSERT_INFO(pCurrentCpu != Lock->Holder,
                "Lock initial taken by function 0x%X, now called by 0x%X\n",
                Lock->FunctionWhichTookLock,
                *((PVOID*)_AddressOfReturnAddress())
                );
    
    /*
    * 1. b:
    * _InterlockedCompareExchange8 is an atomic operation to take the lock. By using an atomic operation for this, we
    * can be sure that no other CPU tries to acquire the lock at the same time with the current one. The interrupts are
    * disabled for the current CPU, so any other could come in and disturb the lock acquire function if we would not use
    * an atomic operation.
    */
    while (LOCK_TAKEN == _InterlockedCompareExchange8(&Lock->State, LOCK_TAKEN, LOCK_FREE))
    {
        _mm_pause();
    }

    ASSERT(NULL == Lock->FunctionWhichTookLock);
    ASSERT(NULL == Lock->Holder);

    /*
    * 1. c:
    * Holder = the CPU that acquired the lock
    * FunctionWhichTookLock = the function in which SpinLockAquire was called (used mainly for
    *                         degugging purposes)
    */
    Lock->Holder = pCurrentCpu;
    Lock->FunctionWhichTookLock = *( (PVOID*) _AddressOfReturnAddress() );

    ASSERT(LOCK_TAKEN == Lock->State);
}

BOOL_SUCCESS
BOOLEAN
SpinlockTryAcquire(
    INOUT       PSPINLOCK       Lock,
    OUT         INTR_STATE*     IntrState
    )
{
    PVOID pCurrentCpu;

    BOOLEAN acquired;

    ASSERT(NULL != Lock);
    ASSERT(NULL != IntrState);

    *IntrState = CpuIntrDisable();

    pCurrentCpu = CpuGetCurrent();

    acquired = (LOCK_FREE == _InterlockedCompareExchange8(&Lock->State, LOCK_TAKEN, LOCK_FREE));
    if (!acquired)
    {
        CpuIntrSetState(*IntrState);
    }
    else
    {
        ASSERT(NULL == Lock->FunctionWhichTookLock);
        ASSERT(NULL == Lock->Holder);

        Lock->Holder = pCurrentCpu;
        Lock->FunctionWhichTookLock = *((PVOID*)_AddressOfReturnAddress());

        ASSERT(LOCK_TAKEN == Lock->State);
    }

    return acquired;
}

BOOLEAN
SpinlockIsOwner(
    IN          PSPINLOCK       Lock
    )
{
    return CpuGetCurrent() == Lock->Holder;
}

/*
* 2. c:
* The OldIntrState holds the interrupt state to which the CPU should return after the 
* interrupts are enabled again.
*/
void
SpinlockRelease(
    INOUT       PSPINLOCK       Lock,
    IN          INTR_STATE      OldIntrState
    )
{
    PVOID pCurrentCpu = CpuGetCurrent();

    /*
    * 2. a:
    * This asserts make sure that:
    *   1. We won't try to release a NULL lock (makes no sense to do that)
    *   2. The current CPU that tries to release the lock is the actual owner of that lock
    *   3. Makes sure that interrupts are disabled for the current CPU
    */
    ASSERT(NULL != Lock);
    ASSERT_INFO(pCurrentCpu == Lock->Holder,
                "LockTaken by CPU: 0x%X in function: 0x%X\nNow release by CPU: 0x%X in function: 0x%X\n",
                Lock->Holder, Lock->FunctionWhichTookLock,
                pCurrentCpu, *( (PVOID*) _AddressOfReturnAddress() ) );
    ASSERT(INTR_OFF == CpuIntrGetState());

    Lock->Holder = NULL;
    Lock->FunctionWhichTookLock = NULL;

    /*
    * 2. b:
    * Here the lock is freed. This is another atomic operation. Just ast the lock aquire, we
    * need the interrupts to be disabled so that our function won't be interrupted by external
    * events.
    * 
    * And we are again using an atomic operation _InterlockedExchange8 to make sure other CPUs
    * won't interfere with the release of the lock
    */
    _InterlockedExchange8(&Lock->State, LOCK_FREE);

    CpuIntrSetState(OldIntrState);
}

#endif // _COMMONLIB_NO_LOCKS_
