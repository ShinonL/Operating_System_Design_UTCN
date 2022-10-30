#pragma once

#include "common_lib.h"
#include "synch.h"
#include "list.h"

typedef struct _SEMAPHORE {
	DWORD			Value; 
	LOCK			SemaphoreLock;

	// for block waiting
	_Guarded_by_(SemaphoreLock)
	LIST_ENTRY		WaitingList;
} SEMAPHORE, * PSEMAPHORE;

void
SemaphoreInit (
	OUT PSEMAPHORE	Semaphore,
	IN	DWORD		InitialValue
);

void
SemaphoreUp(
	INOUT	PSEMAPHORE	Semaphore,
	IN      DWORD		Value
);

void
SemaphoreDown (
	INOUT	PSEMAPHORE	Semaphore,
	IN		DWORD		Value
);