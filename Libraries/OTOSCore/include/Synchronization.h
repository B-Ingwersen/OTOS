#ifndef OTOS_CORE___SYNCHRONIZATION_H
#define OTOS_CORE___SYNCHRONIZATION_H

#define synchronization_InitializeSpinlock(spinlock) synchronization_CloseSpinlock(spinlock)

#include "Definitions.h"

typedef volatile u32 Synchronization_Spinlock;

void synchronization_OpenSpinlock(Synchronization_Spinlock * spinlock);

void synchronization_CloseSpinlock(Synchronization_Spinlock * spinlock);

#endif
