#include "HAL9000.h"
#include "thread_internal.h"
#include "semaphore.h"

/*
* BUSY WAITING

void
SemaphoreInit(
	OUT PSEMAPHORE	Semaphore,
	IN	DWORD		InitialValue
) {
	INTR_STATE dummyState;

	ASSERT(NULL != Semaphore);

	memzero(Semaphore, sizeof(SEMAPHORE));

	LockInit(&Semaphore->SemaphoreLock);

	LockAcquire(&Semaphore->SemaphoreLock, &dummyState);
	Semaphore->Value = InitialValue;
	LockRelease(&Semaphore->SemaphoreLock, dummyState);
}

void
SemaphoreUp(
	INOUT	PSEMAPHORE	Semaphore,
	IN      DWORD		Value
) {
	INTR_STATE dummyState;

	ASSERT(NULL != Semaphore);

	LockAcquire(&Semaphore->SemaphoreLock, &dummyState);
	Semaphore->Value += Value;
	LockRelease(&Semaphore->SemaphoreLock, dummyState);
}

void
SemaphoreDown(
	INOUT	PSEMAPHORE	Semaphore,
	IN		DWORD		Value
) {
	INTR_STATE dummyState;

	LockAcquire(&Semaphore->SemaphoreLock, &dummyState);

	if (Semaphore->Value < Value) {
		LockRelease(&Semaphore->SemaphoreLock, dummyState);
		_mm_pause();
		LockAcquire(&Semaphore->SemaphoreLock, &dummyState);
	}

	Semaphore->Value -= Value;

	LockRelease(&Semaphore->SemaphoreLock, dummyState);
}

*/

void
SemaphoreInit(
	OUT PSEMAPHORE	Semaphore,
	IN	DWORD		InitialValue
) {
	ASSERT(NULL != Semaphore);

	memzero(Semaphore, sizeof(SEMAPHORE));

	LockInit(&Semaphore->SemaphoreLock);
	InitializeListHead(&Semaphore->WaitingList);
	Semaphore->Value = InitialValue;
}

void
SemaphoreUp(
	INOUT	PSEMAPHORE	Semaphore,
	IN      DWORD		Value
) {
	INTR_STATE oldState;

	ASSERT(NULL != Semaphore);

	LockAcquire(&Semaphore->SemaphoreLock, &oldState);
	
	for (DWORD i = 0; i < Value && !IsListEmpty(&Semaphore->WaitingList); i++) {
		PLIST_ENTRY pListEntry = RemoveHeadList(&Semaphore->WaitingList);
		PTHREAD pThread = CONTAINING_RECORD(pListEntry, THREAD, ReadyList);
		ThreadUnblock(pThread);
	}

	Semaphore->Value += Value;

	LockRelease(&Semaphore->SemaphoreLock, oldState);
}

void
SemaphoreDown(
	INOUT	PSEMAPHORE	Semaphore,
	IN		DWORD		Value
) {
	INTR_STATE dummyState;
	INTR_STATE oldState;

	ASSERT(NULL != Semaphore);
	ASSERT(0 != Value);

	oldState = CpuIntrDisable();

	LockAcquire(&Semaphore->SemaphoreLock, &dummyState);

	PTHREAD pCurrentThread = GetCurrentThread();
	if (Semaphore->Value < Value) {
		InsertTailList(&Semaphore->WaitingList, &pCurrentThread->ReadyList);

		ThreadTakeBlockLock();
		LockRelease(&Semaphore->SemaphoreLock, dummyState);

		ThreadBlock();
		LockAcquire(&Semaphore->SemaphoreLock, &dummyState);
	}

	Semaphore->Value -= Value;

	LockRelease(&Semaphore->SemaphoreLock, dummyState);

	CpuIntrSetState(oldState);
}