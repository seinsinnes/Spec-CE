; src/native_shim.asm
    assume adl=1

    extern _vm_state
    extern _z80_mbase
    extern _c_stack_backup
    extern _z80_i
    extern _z80_iff2

    section .text
    public _run_native_shim
    public _shim_exit_trap

_run_native_shim:
    push ix
    ld   ix, 0
    add  ix, sp
    push iy
    push hl
    push de
    push bc

    ld   (_c_stack_backup), sp

    di
    ld   a, (_z80_mbase)
    ld   mb, a

    ; Set the 16-bit Game Stack Pointer (SPS) natively
    ld   hl, (_vm_state + 18) ; Offset 6*3 = SP
    db   0x40, 0xF9           ; LD SP, HL (.SIS)

    ; Push the Target PC onto the 16-bit Game Stack.
    ld   hl, (_vm_state + 21) ; Offset 7*3 = PC
    db   0x40, 0xE5           ; PUSH HL (.SIS) -> Pushes 2 bytes to SPS

    ; Load all Game Registers
    ld   bc, (_vm_state + 27)
    ld   de, (_vm_state + 30)
    ld   hl, (_vm_state + 33)
    exx
    
    ld   hl, (_vm_state + 24)
    push hl
    pop  af
    ex   af, af'

    ld   a, (_z80_i)
    ld   i, a

    ld   bc, (_vm_state + 3)
    ld   de, (_vm_state + 6)
    ld   ix, (_vm_state + 12)
    ld   iy, (_vm_state + 15)
    
    ; Now safely overwrite 'A' with the game's true AF register.
    ld   hl, (_vm_state + 0)
    push hl
    pop  af
    
    ; Load the Game's HL register last
    ld   hl, (_vm_state + 9)

    ; 4. Enter 16-bit Z80 Mode and Launch!
    db   0x40, 0xC3, 0x06, 0x00

; -----------------------------------------------------------------------------
_shim_exit_trap:
    assume adl=1
    
    ld   (_vm_state + 3), bc
    ld   (_vm_state + 6), de
    ld   (_vm_state + 9), hl
    ld   (_vm_state + 12), ix
    ld   (_vm_state + 15), iy

    ; Save AF
    push af
    pop  hl
    ld   (_vm_state + 0), hl

    ; Save AF'
    ex   af, af'
    push af
    pop  hl
    ld   (_vm_state + 24), hl
    ex   af, af'


    ; The original AF is backed up. Now read the I register.
    ld   a, i
    ld   (_z80_i), a
    
    ; When 'ld a, i' executes, the Z80 copies the Interrupt Flip-Flop 
    ; state (IFF2) directly into the Parity/Overflow flag.
    push af
    pop  hl
    ld   a, l
    and  0x04           ; Isolate Bit 2 (The P/V flag)
    ld   (_z80_iff2), a ; If 0, Interrupts are Disabled.

    exx
    ld   (_vm_state + 27), bc
    ld   (_vm_state + 30), de
    ld   (_vm_state + 33), hl
    exx

    ld   a, i
    ld   (_z80_i), a

    ; Extract Z80 PC and safely restore SP
    ; POP DE (.SIS) extracts the PC AND restores the Stack Pointer
    db   0x40, 0xD1           
    ld   (_vm_state + 21), de 

    ld   hl, 0
    db   0x40, 0x39           ; ADD HL, SP (.SIS) -> Grabs the SP
    ld   (_vm_state + 18), hl 

    ld   sp, (_c_stack_backup)
    ld   a, 0xD0
    ld   mb, a
    ei

    pop  bc
    pop  de
    pop  hl
    pop  iy
    pop  ix
    ret