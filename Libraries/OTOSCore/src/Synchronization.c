#include "Synchronization.h"

void synchronization_OpenSpinlock(Synchronization_Spinlock * spinlock){
    asm volatile (
        ".synchronization_OpenSpinlock_open%=:          \n\t"\
        "   lock bts dword ptr [eax], 0                 \n\t"\
        "   jc .synchronization_OpenSpinlock_wait%=     \n\t"\
        "   jmp .synchronization_OpenSpinlock_end%=     \n\t"\
        ".synchronization_OpenSpinlock_wait%=:          \n\t"\
        "   pause                                       \n\t"\
        "   test dword ptr [eax], 1                     \n\t"\
        "   jnz .synchronization_OpenSpinlock_wait%=    \n\t"\
        "   jmp .synchronization_OpenSpinlock_open%=    \n\t"\
        ".synchronization_OpenSpinlock_end%=:"
        :: "a" (spinlock)
        : "cc", "memory"
    );
}

void synchronization_CloseSpinlock(Synchronization_Spinlock * spinlock) {
    asm volatile (
        "mov dword ptr [eax], 0 \n\t"\
        :: "a" (spinlock)
        : "cc", "memory"
    );
}
