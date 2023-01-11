// Threads 6
#include "HAL9000.h"
#include "thread_internal.h"
#include "barrier.h"

void
BarrierInit(
    OUT     PBARRIER        Barrier,
    IN      DWORD           NoOfParticipants
) {
    ASSERT(NULL != Barrier);
    ASSERT(0 <= NoOfParticipants);

    memzero(Barrier, sizeof(BARRIER));

    LockInit(&Barrier->CountLock);

    Barrier->NoOfParticipants = NoOfParticipants;
    Barrier->LeaveCount = NoOfParticipants;
    Barrier->ArriveCount = 0;
    Barrier->Flag = FALSE;
}

void
BarrierWait(
    INOUT   PBARRIER        Barrier
) {
    ASSERT(NULL != Barrier);

    INTR_STATE dummy;
    LockAcquire(&Barrier->CountLock, &dummy);
    if (Barrier->ArriveCount == 0) {
        if (Barrier->LeaveCount != Barrier->NoOfParticipants) {
            LockRelease(&Barrier->CountLock, dummy);

            // busy-waiting
            while (Barrier->LeaveCount != Barrier->NoOfParticipants);

            LockAcquire(&Barrier->CountLock, &dummy);
        }
        Barrier->Flag = FALSE;
    }

    LOGL("Barrier CPU arrived: %d\n", CpuGetApicId());
    Barrier->ArriveCount++;
    LockRelease(&Barrier->CountLock, dummy);

    if (Barrier->ArriveCount == Barrier->NoOfParticipants) {
        Barrier->ArriveCount = 0;
        Barrier->LeaveCount = 1;
        Barrier->Flag = TRUE;
    }
    else {
        // busy-waiting
        while (!Barrier->Flag);

        LockAcquire(&Barrier->CountLock, &dummy);
        Barrier->LeaveCount++;
        LockRelease(&Barrier->CountLock, dummy);
    }

    LOGL("Barrier CPU left: %d\n", CpuGetApicId());
}
