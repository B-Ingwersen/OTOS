openSpinlock:               ; eax = lock location
    push ebx
    mov ebx, [esp + 8 + 8]
    .tryAgain:

    lock bts dword [eax], 0
    jc .spinlockWaitLoop

    pop ebx
    ret
 
    .spinlockWaitLoop:
        pause                    ; Tell CPU we're spinning
        test dword [eax], 1      ; Is the lock free?
        jnz .spinlockWaitLoop    ; no, wait
        jmp .tryAgain         ; retry
 
closeSpinlock:
    mov dword [eax], 0
    ret