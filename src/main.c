/* src/main.c */
#include <ti/screen.h>
#include <keypadc.h>
#include <stdio.h>
#include <string.h>
#include <graphx.h>
#include <fileioc.h>
#include <stdint.h>
#include <debug.h>

#include "native_shim.h"

/* Z80 Memory will live at the start of VRAM (Hidden) */
#define Z80_RAM_ADDR  0xD40000
#define Z80_MBASE     0xD4

/* LCD Display will live higher up in VRAM (Visible) */
#define LCD_BUFFER_ADDR 0xD52C00

/* Hardware Registers for LCD Controller */
#define LCD_UPBASE      ((volatile uint32_t*)0xE30010)

/* ------------------------------------------------------------------------- */
/* GLOBALS EXPORTED TO ASSEMBLY                                              */
/* ------------------------------------------------------------------------- */
uint32_t c_stack_backup = 0;
uint8_t  z80_mbase = 0;

/* Array to hold state. uint24_t is 3-bytes native to the TI-84 CE. */
/* Index map: [0]=AF, [1]=BC, [2]=DE, [3]=HL, [4]=IX, [5]=IY, [6]=SP, [7]=PC */
/* the Shadow Registers */
/* [8]=AF', [9]=BC', [10]=DE', [11]=HL' */
uint24_t vm_state[12] = {0};

uint8_t z80_i = 0;
uint8_t z80_im = 1;

/* ------------------------------------------------------------------------- */
void load_spectrum_data(uint8_t *z80_mem) {
    ti_var_t slot;
    uint8_t *data_ptr;

    /* Load the 16KB ROM at 0x0000 */
    slot = ti_Open("ZXROM", "r");
    if (slot) {
        data_ptr = (uint8_t*)ti_GetDataPtr(slot);
        memcpy(&z80_mem[0x0000], data_ptr, 16384);
        ti_Close(slot);
    }

    /* Load the 48KB SNA Snapshot at 0x4000 */
    slot = ti_Open("ZXGAME", "r");
    if (slot) {
        uint8_t *header = (uint8_t*)ti_GetDataPtr(slot);
        
        /* The actual RAM dump starts at byte 27 */
        memcpy(&z80_mem[0x4000], header + 27, 49152);
        ti_Close(slot);

        /* --- EXTRACT INTERRUPT STATE --- */
        z80_i  = header[0];
        z80_im = header[25];

        /* --- PARSE SNA HEADER --- */
        /* Read Prime (Shadow) Registers */
        uint16_t hl_prime = header[1] | (header[2] << 8);
        uint16_t de_prime = header[3] | (header[4] << 8);
        uint16_t bc_prime = header[5] | (header[6] << 8);
        uint16_t af_prime = header[7] | (header[8] << 8);

        /* Read Standard Registers */
        uint16_t hl = header[9]  | (header[10] << 8);
        uint16_t de = header[11] | (header[12] << 8);
        uint16_t bc = header[13] | (header[14] << 8);
        uint16_t iy = header[15] | (header[16] << 8);
        uint16_t ix = header[17] | (header[18] << 8);
        uint16_t af = header[21] | (header[22] << 8);
        uint16_t sp = header[23] | (header[24] << 8);

        uint16_t pc = z80_mem[sp] | (z80_mem[sp + 1] << 8);
        sp += 2; 

        /* Store everything in the array for the assembly shim */
        vm_state[0] = af;  vm_state[1] = bc;  vm_state[2] = de;  vm_state[3] = hl;
        vm_state[4] = ix;  vm_state[5] = iy;  vm_state[6] = sp;  vm_state[7] = pc;
        vm_state[8] = af_prime; 
        vm_state[9] = bc_prime; 
        vm_state[10] = de_prime; 
        vm_state[11] = hl_prime;
    }
}

void setup_spectrum_palette(void) {
    /* 16-bit RGB565 colors for the Spectrum */
    uint16_t spec_colors[16] = {
        0x0000, 0x001D, 0xB000, 0xB01D, 0x05A0, 0x05BD, 0xB5A0, 0xB5BD, // Normal
        0x0000, 0x001F, 0xF800, 0xF81F, 0x07E0, 0x07FF, 0xFFE0, 0xFFFF  // Bright
    };
    gfx_SetPalette(spec_colors, 16, 0);
}

void render_spectrum_frame(void) {
    uint8_t *z80_mem = (uint8_t*)Z80_RAM_ADDR;
    uint8_t *lcd_mem = (uint8_t*)LCD_BUFFER_ADDR;
    
    int x, y, b;
    uint16_t bitmap_offset, attr_offset;
    uint8_t pixels, attr, ink, paper;

    /* Loop over the 256x192 Spectrum screen */
    for (y = 0; y < 192; y++) {
        bitmap_offset = ((y & 0xC0) << 5) | ((y & 0x07) << 8) | ((y & 0x38) << 2);
        attr_offset = 0x1800 + ((y >> 3) * 32);

        int lcd_y = y + 24;

        for (x = 0; x < 32; x++) { 
            pixels = z80_mem[0x4000 + bitmap_offset + x];
            attr   = z80_mem[0x4000 + attr_offset + x];

            uint8_t bright = (attr & 0x40) ? 8 : 0; 
            ink   = (attr & 0x07) + bright;
            paper = ((attr >> 3) & 0x07) + bright;

            int lcd_x = (x * 8) + 32;
            int lcd_index = (lcd_y * 320) + lcd_x;

            for (b = 7; b >= 0; b--) {
                if (pixels & (1 << b)) {
                    lcd_mem[lcd_index] = ink;
                } else {
                    lcd_mem[lcd_index] = paper;
                }
                lcd_index++;
            }
        }
    }
}



void fire_interrupt(uint8_t *z80_mem) {
    /* Standard Spectrum IM 1 Interrupt runs at 0x0038 */
    uint16_t current_pc = vm_state[7];
    uint16_t current_sp = vm_state[6];

    /* Push the current Z80 PC to the Z80 Stack */
    current_sp--;
    z80_mem[current_sp] = (current_pc >> 8) & 0xFF;
    current_sp--;
    z80_mem[current_sp] = current_pc & 0xFF;

    /* Update the state so the Z80 jumps to the ROM's ISR */
    vm_state[6] = current_sp;
    vm_state[7] = 0x0038;
}


int main(void) {
    uint8_t *z80_mem = (uint8_t*)Z80_RAM_ADDR;
    uint8_t *screen_mem = (uint8_t*)LCD_BUFFER_ADDR;

    /* Initialize Graphics */
    gfx_Begin();
    gfx_SetDrawBuffer(); 
    setup_spectrum_palette();
    *LCD_UPBASE = LCD_BUFFER_ADDR;
    
    /* Clear Memory completely */
    memset(z80_mem, 0, 65536);
    memset(screen_mem, 0, 320*240);
    
    z80_mbase = Z80_MBASE;
    memset(vm_state, 0, sizeof(vm_state));

    /* ==================================================================== */
    /* LOAD THE SNAPSHOT (ROM + RAM + CPU STATE)                         */
    /* ==================================================================== */
    
    /* This function opens ZXROM and ZXGAME, loads them into memory, 
       and extracts all the exact CPU registers saved in the SNA file! */
    load_spectrum_data(z80_mem);

    /* ==================================================================== */
    /* TRAPS                                                                */
    /* ==================================================================== */
    
    uint32_t trap_address = (uint32_t)&shim_exit_trap;
    
    /* Assign the Traps (AVOIDING 0x0020!) */
    z80_mem[0x0000] = 0x5B; z80_mem[0x0001] = 0xC3; /* RST 00h (Catches HALT) */
    z80_mem[0x0002] = trap_address & 0xFF; z80_mem[0x0003] = (trap_address >> 8) & 0xFF; z80_mem[0x0004] = (trap_address >> 16) & 0xFF;
    
    z80_mem[0x0008] = 0x5B; z80_mem[0x0009] = 0xC3; /* RST 08h (Catches IN A, 254) */
    z80_mem[0x000A] = trap_address & 0xFF; z80_mem[0x000B] = (trap_address >> 8) & 0xFF; z80_mem[0x000C] = (trap_address >> 16) & 0xFF;

    z80_mem[0x0030] = 0x5B; z80_mem[0x0031] = 0xC3; /* RST 30h (Catches IN A, C) */
    z80_mem[0x0032] = trap_address & 0xFF; z80_mem[0x0033] = (trap_address >> 8) & 0xFF; z80_mem[0x0034] = (trap_address >> 16) & 0xFF;
    
    z80_mem[0x0006] = 0xC9; // Z80 Entrance Stub


    /* Patch the ROM */
    z80_mem[0x0296] = 0xF7; z80_mem[0x0297] = 0x00; 
    z80_mem[0x1F3D] = 0xC7; /* PAUSE -> Replaces HALT with RST 00h */

    z80_mem[0x03E1] = 0x00; z80_mem[0x03E2] = 0x00; /* Mute Beeper */
    
    /* Disable TAPE Communications*/
    z80_mem[0x04DA] = 0x00; z80_mem[0x04DB] = 0x00;
    z80_mem[0x04EC] = 0x00; z80_mem[0x04ED] = 0x00;
    z80_mem[0x04F4] = 0x00; z80_mem[0x04F5] = 0x00;
    z80_mem[0x051C] = 0x00; z80_mem[0x051D] = 0x00;
    z80_mem[0x0531] = 0x00; z80_mem[0x0532] = 0x00;

    z80_mem[0x055C] = 0x00; z80_mem[0x055D] = 0x00;
    z80_mem[0x0562] = 0x00; z80_mem[0x0563] = 0x00;


    z80_mem[0x229B] = 0x00; z80_mem[0x229C] = 0x00; /* Mute Border */

    z80_mem[0x1F56] = 0x00; z80_mem[0x1F57] = 0x00; /* Disable break-key*/
    z80_mem[0x1F5C] = 0x00; z80_mem[0x1F5D] = 0x00;

    volatile uint32_t *INT_MASK = (volatile uint32_t*)0xF00028;
    int virtual_cycles = 0;
    kb_Scan();

    /* Loop UNTIL the user presses the [clear] button (Group 6, kb_Clear) */
    while (!(kb_Data[6] & kb_Clear)) { 
        
        uint32_t int_backup = *INT_MASK;
        *INT_MASK = 0;
        run_native_shim();
        *INT_MASK = int_backup;

        uint16_t pc = vm_state[7];
        uint8_t trap_cause = z80_mem[pc - 1]; 

        /* ========================================================== */
        /* HARDWARE TRAP (RST 08h and RST 30h)                        */
        /* ========================================================== */
        if (trap_cause == 0xCF || trap_cause == 0xF7) {
            
            uint8_t port_low, port_high;
            if (trap_cause == 0xCF) {
                port_high = (vm_state[0] >> 8) & 0xFF;
                port_low  = 0xFE;
            } else {
                port_high = (vm_state[1] >> 8) & 0xFF;
                port_low  = vm_state[1] & 0xFF;
            }

            uint8_t hardware_result = 0xFF;

            /* THE ULA KEYBOARD */
            if ((port_low & 0x01) == 0) {
                uint8_t keys = 0x1F; 
                
                if (!(port_high & 0x04)) {
                    if (kb_Data[7] & kb_Right) keys &= ~0x04; /* E */
                    if (kb_Data[7] & kb_Left)  keys &= ~0x02; /* W */
                }
                if (!(port_high & 0x02)) {
                    if (kb_Data[7] & kb_Down) keys &= ~0x02; /* S */
                }
                if (!(port_high & 0x80)) {
                    if (kb_Data[1] & kb_2nd) keys &= ~0x01; /* SPACE */
                    if (kb_Data[7] & kb_Up)  keys &= ~0x08; /* N */
                }
                if (!(port_high & 0x40)) {
                    if (kb_Data[2] & kb_Alpha) keys &= ~0x01; /* ENTER */
                }
                
                hardware_result = keys | 0xE0;
            }

            vm_state[0] = (hardware_result << 8) | (vm_state[0] & 0xFF);
            vm_state[7]++;

            virtual_cycles++;
            if (virtual_cycles > 16) { 
                render_spectrum_frame();
                if (vm_state[7] >= 0x4000) fire_interrupt(z80_mem);
                virtual_cycles = 0;
            }
        }
        /* ========================================================== */
        /* HALT TRAP (RST 00h)                                        */
        /* ========================================================== */
        else if (trap_cause == 0xC7) {
            render_spectrum_frame();
            fire_interrupt(z80_mem);
            virtual_cycles = 0;
        }
        else {
            dbg_printf("CRASH: Unexpected Trap %02X at %04X\n", trap_cause, pc);
            break;
        }

        kb_Scan();
    }

    gfx_End();
    return 0;
}