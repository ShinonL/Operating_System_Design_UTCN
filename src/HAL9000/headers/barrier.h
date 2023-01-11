// Threads 6
#pragma once

#include "common_lib.h"
#include "lock_common.h"

typedef struct _BARRIER
{
    DWORD                   NoOfParticipants;

    _Guarded_by_(CountLock)
        DWORD               ArriveCount;
    _Guarded_by_(CountLock)
        DWORD               LeaveCount;
    LOCK                    CountLock;

    BOOLEAN                 Flag;
} BARRIER, * PBARRIER;

void
BarrierInit(
    OUT     PBARRIER        Barrier,
    IN      DWORD           NoOfParticipants
);

void
BarrierWait(
    INOUT   PBARRIER        Barrier
);