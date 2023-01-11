#include "HAL9000.h"
#include "ex_system.h"
#include "thread_internal.h"
// vm 6
#include "vmm.h"

void
ExSystemTimerTick(
    void
    )
{
    ThreadTick();

    // vm 6
    VmmTick();
}