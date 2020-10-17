[bits 16]
[org 0x3000]	; offsets by start of memory location

start:
    xor eax, eax
    mov es, ax
    mov ss, ax
    mov ds, ax
    mov fs, ax
    mov gs, ax

    mov ax, 0x03			; ensure 80x25 text mode
    int 0x10

    setUpBootLoaderStack:
        mov sp, 0x3000		; sets up the stack at 0x8000
        mov bp, sp

;mov ax, 0x03			; ensure 80x25 text mode
;int 0x10

RelocateCode:
    mov si, 0x7C00
    mov di, 0x2000
    mov cx, 512
    rep movsb

    mov si, 0x8000
    mov di, 0x3000
    mov cx, 4096
    rep movsb

    jmp 0x0000:relocationJump

relocationJump:

call initializeFat32
loadNeededFiles:
    mov di, 0x800
    mov bp, 0
    .loadInitializeKernel:
        mov eax, [rootDirClusterNum]
        mov si, fileName1_1
        mov cx, 8
        call findInDirectoryClusterChain
        mov si, fileName1_2
        mov cx, 17
        call findInDirectoryClusterChain
        call openClusterContentsToBuffer
        xor eax, eax
        mov ax, di
        shl eax, 4
        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE], eax
        xor eax, eax
        mov ax, cx
        shl eax, 4
        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE + 4], eax
        mov di, 0x900
    mov bp, 1
    .loadKernel:
        mov eax, [rootDirClusterNum]
        mov si, fileName1_1
        mov cx, 8
        call findInDirectoryClusterChain
        mov si, fileName2_2
        mov cx, 7
        call findInDirectoryClusterChain
        call openClusterContentsToBuffer
        xor eax, eax
        mov ax, di
        shl eax, 4
        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE + 8], eax
        xor eax, eax
        mov ax, cx
        shl eax, 4
        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE + 12], eax
        add di, cx
    mov bp, 2
    .loadProcessManager:
        mov eax, [rootDirClusterNum]
        mov si, fileName3_1
        mov cx, 9
        call findInDirectoryClusterChain
        mov si, fileName3_2
        mov cx, 15
        call findInDirectoryClusterChain
        call openClusterContentsToBuffer
        xor eax, eax
        mov ax, di
        shl eax, 4
        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE + 16], eax
        xor eax, eax
        mov ax, cx
        shl eax, 4
        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE + 20], eax
        add di, cx
    mov bp, 3
    .loadDiskManager:
        mov eax, [rootDirClusterNum]
        mov si, fileName3_1
        mov cx, 9
        call findInDirectoryClusterChain
        mov si, fileName4_2
        mov cx, 21
        call findInDirectoryClusterChain
        call openClusterContentsToBuffer
        xor eax, eax
        mov ax, di
        shl eax, 4
        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE + 24], eax
        xor eax, eax
        mov ax, cx
        shl eax, 4
        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE + 28], eax
        add di, cx

call getBiosMemMap
call enableA20
jmp switchToProtectedMode
BeginProtectedMode:
    [bits 32]
    mov eax, [0x8000]
    jmp CODE_SEG:0x8000
    [bits 16]


fileName1_1:
    db "Kernels", 0
fileName1_2:
    db "InitializeKernel", 0
fileName2_2:
    db "Kernel", 0
fileName3_1:
    db "Managers", 0
fileName3_2:
    db "ProcessManager", 0
fileName4_2:
    db "DiskManager_FAT32MBR", 0


INITIALIZE_KERNEL_LOAD_LOCATION_TABLE equ 0x8004
INITIALIZE_KERNEL_LOAD_LOCATION_TABLE_GDTR equ INITIALIZE_KERNEL_LOAD_LOCATION_TABLE + 32


;PRINT SERVICES
    print_char:
        mov ah, 0x0e
        int 0x10		; uses BIOS interupt to print character in al
        ret

    print_string_length:	;prints the value specified at di
        pusha
        .loop:
            mov al, [di]		; moves the next character into al
            call print_char		; character in al is printed
            inc di			; moves bx to next address in memory; next character in the string
            loop .loop		; goes back to the beginning of the loop
        .end:		; ends the function
            popa
            ret

    newLine:
        mov ah, 0x0E
        mov al, 0x0a	; adds a space for asthetics
        int 0x10
        mov al, 0x0d	; adds a space for asthetics
        int 0x10
        ret
    
    print_hex:	; prints word at address in bx in hexidecimal
        pusha

        mov al, '0'
        call print_char
        mov al, 'x'
        call print_char

        mov edx, [bx]
        mov cx, 8
        .loop:
            mov eax, edx
            shr eax, 28
            add al, 0x30
            cmp al, 0x3A
            jb .isHexNumeral
            add al, 0x07
            .isHexNumeral:
            call print_char
            shl edx, 4
            loop .loop

        popa
        ret
    
    print_spaces:
        .loop:
            mov al, ' '
            call print_char
            loop .loop
        ret
;PRINT SERVICES

saveBootDrive:
    db 0

;Fat32 SERVICES:

    initializeFat32:
        pusha

        mov [saveBootDrive], dl
        xor eax, eax
        mov edx, eax
        mov ax, [bytesPerSector]
        shr ax, 9
        mov [sectorSize], eax
        mul byte [sectorsPerCluster]
        mov [clusterSize], eax

        mov eax, [sectorsPerFAT]
        mov dl, [nFATS]
        mul edx
        mov dx, [nReservedSectors]
        add eax, edx
        mov dx, [sectorSize]
        mul edx
        mov [firstDataSector], eax

        popa
        ret

    loadCluster:                        ; eax = clusterNumber, di=load SEGMENT
        pushad
        sub eax, 2
        mov edx, eax
        mov eax, [clusterSize]
        mul edx
        add eax, [firstDataSector]
        add eax, [partitionBaseAddress]
        
        call loadDisk

        popad
        ret

    getNextCluster:                 ; eax = cluster, return in eax, di = 
        push ebx
        push ecx
        push di

        mov ebx, eax
        shl ebx, 2                                  ; multiply by 4 for offset in table

        xor eax, eax
        mov ax, [nReservedSectors]
        mul word [sectorSize]
        
        mov ecx, ebx
        shr ecx, 9
        add eax, ecx                                ; eax is now the sector offset
        add eax, [partitionBaseAddress]
        mov di, 0x400
        call loadDisk

        and bx, 0x1FF
        mov eax, [bx + 0x4000]
        and eax, 0x0FFFFFFF

        pop di
        pop ecx
        pop ebx

        ret
    

    findInDirectoryClusterChain:                ; eax= root cluster, si=fileName, cx=fileNameLength
        push bx
        push cx
        push dx
        push si
        push di

        xor di, di
        findInDir_gotNextCluster:
        cmp eax, 0x0FFFFFF7
        jae findInDirectory_Fail

        push di
            mov di, 0x400       ; load to 0x4000
            call loadCluster
        pop di

        xor bx, bx
        checkFileName:
            cmp di, 2
            je foundFileName

            cmp [bx + 0x4000 + 11], byte 0x0F
            jne checkFileName_skipEntry
            cmp [bx + 0x4000], byte 0xE5
            je checkFileName_skipEntry

            copy13byteFileName:
                pushad

                mov si, assembleFileName
                mov cx, 5
                .loop1:
                    mov dl, [bx + 0x4000 + 1]
                    mov [si], dl
                    add bx, 2
                    inc si
                    loop .loop1
                and bl, 0xE0
                
                mov si, assembleFileName + 5
                mov cx, 6
                .loop2:
                    mov dl, [bx + 0x4000 + 14]
                    mov [si], dl
                    add bx, 2
                    inc si
                    loop .loop2
                and bl, 0xE0

                mov dl, [bx + 0x4000 + 28]
                mov [assembleFileName + 11], dl
                mov dl, [bx + 0x4000 + 30]
                mov [assembleFileName + 12], dl

                popad
        
            xor dx, dx
            mov dl, [bx + 0x4000]
            and dl, 0x1F                            ; dl is the sequence
            dec dl
            ;multiply by 13
                push ax
                mov ax, dx
                mov dx, 13
                mul dx
                mov dx, ax
                pop ax
            
            cmp dx, cx                              ; longer than sequence name
            jae checkFileName_skipEntry

            ;check if first in LFN sequence
                add dx, 13
                cmp dx, cx
                jae .firstLFNEntry
                    cmp di, 0
                    je checkFileName_skipEntry

                .firstLFNEntry:
                sub dx, 13

            ;check the characters
                push cx
                push bx
                push si

                sub cx, dx
                cmp cx, 13
                jb .notAbove13
                    mov cx, 13
                    .notAbove13:

                xor bx, bx
                add si, dx
                .checkLoop:
                    mov dl, [si]
                    cmp dl, [assembleFileName + bx]
                    jne .checkLoop_Fail
                    inc si
                    inc bx
                    loop .checkLoop
                    jmp .checkLoop_Success
                
                .checkLoop_Fail:
                    mov di, 0
                    pop si
                    pop bx
                    pop cx
                    jmp checkFileName_skipEntry
                
                .checkLoop_Success:
                    mov di, 1
                    pop si
                    pop bx
                    pop cx

            mov dl, [bx + 0x4000]
            and dl, 0x1F
            cmp dl, 1
            jne checkFileName_skipEntry

            mov di, 2

            checkFileName_skipEntry:
            add bx, 32
            mov dx, bx
            shr dx, 9
            cmp dx, [clusterSize]
            jb checkFileName

            .endOfCluster:
            call getNextCluster
            jmp findInDir_gotNextCluster
        
        foundFileName:
            mov ax, [bx + 0x4000 + 20]
            shl eax, 16
            mov ax, [bx + 0x4000 + 26]
            
            pop di
            pop si
            pop dx
            pop cx
            pop bx

            ret


    openClusterContentsToBuffer:                ; eax = cluster, di=load SEGMENT, return cx=loadSize / 16 (compatibl with di)
        push eax
        push bx
        push di

        xor cx, cx
        mov bx, [clusterSize]
        shl bx, (9 - 4)                         ; bx = cluster size in bytes / 16

        .loop:
            cmp eax, 0x0FFFFFF7
            je openClusterError
            cmp eax, 0x0FFFFFF8
            jae openClusterContentsToBuffer_endOfChain

            call loadCluster
            
            add di, bx
            add cx, bx
            call getNextCluster
            jmp .loop

        openClusterContentsToBuffer_endOfChain:
        pop di
        pop bx
        pop eax

        ret


    clusterSize:        dd 0            ; in sectors
    sectorSize:         dd 0            ; in sectors
    firstDataSector:    dd 0
    assembleFileName:   times 13 db 0

;ERRORS:
    findInDirectory_Fail:
    openClusterError:
        mov di, errorMessage0
        mov cx, 23
        call print_string_length

        mov di, errorMessage1
        mov cx, 41
        call print_string_length

        call newLine
        call newLine
        mov cx, 8
        call print_spaces
        mov al, '"'
        call print_char
        mov al, '/'
        call print_char
    
        .load0:
            cmp bp, 0
            jne .load1

            mov di, fileName1_1
            mov cx, 7
            call print_string_length
            mov al, '/'
            call print_char
            mov di, fileName1_2
            mov cx, 16
            call print_string_length

            jmp .end
        .load1:
            cmp bp, 1
            jne .load2

            mov di, fileName1_1
            mov cx, 7
            call print_string_length
            mov al, '/'
            call print_char
            mov di, fileName2_2
            mov cx, 6
            call print_string_length

            jmp .end
        .load2:
            cmp bp, 2
            jne .load3

            mov di, fileName3_1
            mov cx, 8
            call print_string_length
            mov al, '/'
            call print_char
            mov di, fileName3_2
            mov cx, 14
            call print_string_length

            jmp .end
        .load3:
            cmp bp, 3
            jne .end

            mov di, fileName3_1
            mov cx, 8
            call print_string_length
            mov al, '/'
            call print_char
            mov di, fileName4_2
            mov cx, 20
            call print_string_length

        .end:

        mov al, '"'
        call print_char
    
        call newLine
        call newLine

        mov di, errorMessage2
        mov cx, 71
        call print_string_length

        mov cx, 6
        .loop:
            call newLine
            loop .loop

        mov di, errorMessage3
        mov cx, 48
        call print_string_length

        mov ah, 0
        int 0x16

        mov di, 0x7C0
        xor eax, eax
        mov cx, [clusterSize]
        call loadDisk

        mov dl, [saveBootDrive]
        jmp 0x0000:0x7C00

    errorMessage0:
        db "Boot Program FAT32MBR: "
    errorMessage1:
        db "An error occured when trying to open file"
    errorMessage2:
        db "From another OS, check that the correct file is saved on this partition"
    errorMessage3:
        db "Press any key to return to the Master Bootloader"
;ERRORS

;DISK SERVICES
    loadDisk:                           ; eax=sector, always loads 1 clusters worth of data
                                        ; di = SEGMENT to load to
        pushad
        mov [loadSector], eax

        mov [loadToLocation], word 0x0000
        mov [loadToSegement], di

        mov ax, [clusterSize]
        mov [loadSizeSectors], ax

        mov si, DiskAddressPacket
        mov dl, [saveBootDrive]
        mov ah, 0x42

        int 0x13

        popad
        ret
;DISK SERVICES

;GET MEMORY MAP
    getBiosMemMap:

        mov edi, MEM_MAP_LOCATION + 0x20
        mov ebx, 0
        mov edx, 0x534D4150
        mov eax, 0xE820
        mov ecx, 24

        int 0x15

        jc .MMfailed
        cmp eax, 0x534D4150
        jne .MMfailed

        .continueGettingMM:
            add edi, 0x20			; increment 32 bits in the table for easy access
        .skipMMEntry:
            mov edx, 0x534D4150
            mov eax, 0xE820
            mov ecx, 24

            int 0x15

            jc .doneBiosMap
            cmp eax, 0x534D4150
            jne .doneBiosMap
            cmp ebx, 0
            je .doneBiosMap

            mov ecx, [edi + 8]
            or ecx, [edi + 12]
            cmp ecx, 0
            je .skipMMEntry

            jmp .continueGettingMM


        .doneBiosMap:
        sub edi, MEM_MAP_LOCATION
        shr edi, 5
        sub edi, 1
        mov [MEM_MAP_LOCATION], edi

        ret

        .MMfailed:

        mov [MEM_MAP_LOCATION], dword 0

        ret

    MEM_MAP_LOCATION equ 0x1000
;GET MEMORY MAP

;ENABLE A20
    enableA20:
        cli

        call    .a20wait
        mov     al,0xAD
        out     0x64,al

        call    .a20wait
        mov     al,0xD0
        out     0x64,al

        call    .a20wait2
        in      al,0x60
        push    eax

        call    .a20wait
        mov     al,0xD1
        out     0x64,al

        call    .a20wait
        pop     eax
        or      al,2
        out     0x60,al

        call    .a20wait
        mov     al,0xAE
        out     0x64,al

        call    .a20wait
        ret
    
        .a20wait:
                in      al,0x64
                test    al,2
                jnz     .a20wait
                ret
        
        
        .a20wait2:
                in      al,0x64
                test    al,1
                jz      .a20wait2
                ret
;ENABLE A20

;SWITCH TO PROTECTED MODE
    switchToProtectedMode:
        cli	;clear interrupt flags; do not use until protected mode interrupt vector is established to prevent problems

        mov [INITIALIZE_KERNEL_LOAD_LOCATION_TABLE_GDTR], dword gdtDescriptor
        lgdt [gdtDescriptor]	;the location of the global descriptor table is passed to the cpu

        mov eax, cr0	; cr0 can't be directly modified, so it's first moved to a register
        or eax, 0x1	; the first bit is changed to 1
        mov cr0, eax	; cr0 is updated, and now we are in 32-bit protected mode

        jmp CODE_SEG:initiateProtectedMode	;Far jump issued to prevent pipelining errors

        [bits 32]
        initiateProtectedMode:	; Now, code can be fully executed in protected mode

            mov ax, DATA_SEG 	; segment registers are redirected to what's defined in the GDT
            mov ds, ax		; set all of them through ax
            mov ss, ax
            mov es, ax
            mov fs, ax
            mov gs, ax

            jmp BeginProtectedMode	; location to go when done
        [bits 16]

    gdtStart:	; used to calculate the the size of the GDT

    gdtNull:	;the null descriptor signaling the start of the gdt
        dd 0x0	;sets 8 bytes to 0
        dd 0x0

    gdtCodeSegment:
        ; base = 0x0
        ; limit = 0xfffff
        ; 1st flags: (prsesent) 1 (privilege ring) 00 (descriptor type)1		: 1001b
        ; type flags (code) 1 (conforming) 0 (readable) 1 (accessed) 0			: 1010b
        ; 2nd flags (granularity) 1 (32-bit default) 1 (64-bit segment) 0 (AVL) 0	: 1100b

        dw 0xffff	; Limit bits 0-15
        dw 0x0		; Base bits 0-15
        db 0x0		; Base bits 16-23
        db 10011010b	; Flags
        db 11001111b	; Flags and Limit bits 16-19
        db 0x0		; Base bits 24-31

    gdtDataSegment:
        ; same as segment descriptor other than flag types
        ; type flags (code) 0 (conforming) 0 (readable) 1 (accessed) 0			: 0010b

        dw 0xffff	; Limit bits 0-15
        dw 0x0		; Base bits 0-15
        db 0x0		; Base bits 16-23
        db 10010010b	; Flags
        db 11001111b	; Flags and Limit bits 16-19
        db 0x0		; Base bits 24-31

    gdtEnd:	; used to calculate the the size of the GDT

    ; Descriptor given ot the processor
    gdtDescriptor:
        dw gdtEnd - gdtStart - 1	; Size of GDT minus 1 for cpu
        dd gdtStart			; Address of the 

    CODE_SEG equ gdtCodeSegment - gdtStart
    DATA_SEG equ gdtDataSegment - gdtStart
;SWITCH TO PROTECTED MODE

times 4096 - 16 - ($-$$) db 0
    DiskAddressPacket:
        db 0x10
        db 0
        loadSizeSectors:
        dw 1
        loadToLocation:
        dw 0
        loadToSegement:
        dw 0
        loadSector:
        dd 0
        dd 0

;FAT32 Definitions
    oemIdentifier       equ 0x2000 + 3
    bytesPerSector      equ 0x2000 + 11
    sectorsPerCluster   equ 0x2000 + 13
    nReservedSectors    equ 0x2000 + 14
    nFATS               equ 0x2000 + 16
    nDirectoryEntries   equ 0x2000 + 17
    totalSectors        equ 0x2000 + 19
    mediaDescriptType   equ 0x2000 + 21
    fat1216SecsPerFAT   equ 0x2000 + 22
    sectorsPerTrack     equ 0x2000 + 24
    headsInMedia        equ 0x2000 + 26
    nHiddenSectors      equ 0x2000 + 28
        partitionBaseAddress equ nHiddenSectors
    largeSectorCount    equ 0x2000 + 32
    sectorsPerFAT       equ 0x2000 + 36
    flagsOffset         equ 0x2000 + 40
    FATVersionNumber    equ 0x2000 + 42
    rootDirClusterNum   equ 0x2000 + 44
    fsInfoSector        equ 0x2000 + 48
    backupBootSector    equ 0x2000 + 50
    reservedBytes12     equ 0x2000 + 52
    driveNumber         equ 0x2000 + 64
    windowsNTFlags      equ 0x2000 + 65
    signature           equ 0x2000 + 66
    volumeID            equ 0x2000 + 67
    volumeLabel         equ 0x2000 + 71
    identifierString    equ 0x2000 + 82
;FAT32 Definitions