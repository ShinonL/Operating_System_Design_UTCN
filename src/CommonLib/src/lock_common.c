#include "common_lib.h"
#include "lock_common.h"

#ifndef _COMMONLIB_NO_LOCKS_

PFUNC_LockInit           LockInit = NULL;

PFUNC_LockAcquire        LockAcquire = NULL;

PFUNC_LockTryAcquire     LockTryAcquire = NULL;

PFUNC_LockRelease        LockRelease = NULL;

PFUNC_LockIsOwner        LockIsOwner = NULL;

#pragma warning(push)
// warning C4028: formal parameter 1 different from declaration
#pragma warning(disable:4028)

/*
* 5:
*               Mutex                       |               SpinLock
* ------------------------------------------------------------------------------------
* - uses block-waiting                      | - uses busy-waiting 
* - executive synchronization mechanism     | - primitive synchronization mechanism
* - recommended for longer pieces of code   | - recommended for smaller pieces of code
*   because of the block waiting mechanism  |   because of the busy waiting mechanism
*   releasing the CPU when not used         |   keeping the CPU busy even if it is not
*                                           |   used
*/
void
LockSystemInit(
    IN      BOOLEAN             MonitorSupport
    )
{

    if (MonitorSupport)
    {
        // we have monitor support
        LockInit = MonitorLockInit;
        LockAcquire = MonitorLockAcquire;
        LockTryAcquire = MonitorLockTryAcquire;
        LockIsOwner = MonitorLockIsOwner;
        LockRelease = MonitorLockRelease;
    }
    else
    {
        // use classic spinlock
        LockInit = SpinlockInit;
        LockAcquire = SpinlockAcquire;
        LockTryAcquire = SpinlockTryAcquire;
        LockIsOwner = SpinlockIsOwner;
        LockRelease = SpinlockRelease;
    }
}
#pragma warning(pop)

#endif // _COMMONLIB_NO_LOCKS_
