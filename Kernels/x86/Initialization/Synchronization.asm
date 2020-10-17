;openSpinlock:               ; eax = lock location
    lock bts dword [eax], 0
    jc .spinlockWaitLoop
    ret
 
    .spinlockWaitLoop:
        pause                    ; Tell CPU we're spinning
        test dword [eax], 1      ; Is the lock free?
        jnz .spinlockWaitLoop    ; no, wait
        ;jmp openSpinlock         ; retry
 
;closeSpinlock:
    mov dword [eax], 0
    ret