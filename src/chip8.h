#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// Display format:
// (0,0) -------- (63,0)
//   |              |
//   |              |
//   |              |
//   |              |
//   |              |
//   |              |
// (0,31) ------- (63,31)
#define CHIP8_WIDTH  64
#define CHIP8_HEIGHT 32

// List of font sprites for hexadecimal digits 0-F,
// stored in the interpreter area of memory (0x050 to 0x0A0).
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
// | "0"  | Hex  | "1"  | Hex  | "2"  | Hex  | "3"  | Hex  |
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
// | **** | 0xF0 |   *  | 0x20 | **** | 0xF0 | **** | 0xF0 |
// | *  * | 0x90 |  **  | 0x60 |    * | 0x10 |    * | 0x10 |
// | *  * | 0x90 |   *  | 0x20 | **** | 0xF0 | **** | 0xF0 |
// | *  * | 0x90 |   *  | 0x20 | *    | 0x80 |    * | 0x10 |
// | **** | 0xF0 |  *** | 0x70 | **** | 0xF0 | **** | 0xF0 |
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
// | "4"  | Hex  | "5"  | Hex  | "6"  | Hex  | "7"  | Hex  |
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
// | *  * | 0x90 | **** | 0xF0 | **** | 0xF0 | **** | 0xF0 |
// | *  * | 0x90 | *    | 0x80 | *    | 0x80 |    * | 0x10 |
// | **** | 0xF0 | **** | 0xF0 | **** | 0xF0 |   *  | 0x20 |
// |    * | 0x10 |    * | 0x10 | *  * | 0x90 |  *   | 0x40 |
// |    * | 0x10 | **** | 0xF0 | **** | 0xF0 |  *   | 0x40 |
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
// | "8"  | Hex  | "9"  | Hex  | "A"  | Hex  | "B"  | Hex  |
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
// | **** | 0xF0 | **** | 0xF0 | **** | 0xF0 | ***  | 0xE0 |
// | *  * | 0x90 | *  * | 0x90 | *  * | 0x90 | *  * | 0x90 |
// | **** | 0xF0 | **** | 0xF0 | **** | 0xF0 | ***  | 0xE0 |
// | *  * | 0x90 |    * | 0x10 | *  * | 0x90 | *  * | 0x90 |
// | **** | 0xF0 | **** | 0xF0 | *  * | 0x90 | ***  | 0xE0 |
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
// | "C"  | Hex  | "D"  | Hex  | "E"  | Hex  | "F"  | Hex  |
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
// | **** | 0xF0 | ***  | 0xE0 | **** | 0xF0 | **** | 0xF0 |
// | *    | 0x80 | *  * | 0x90 | *    | 0x80 | *    | 0x80 |
// | *    | 0x80 | *  * | 0x90 | **** | 0xF0 | **** | 0xF0 |
// | *    | 0x80 | *  * | 0x90 | *    | 0x80 | *    | 0x80 |
// | **** | 0xF0 | ***  | 0xE0 | **** | 0xF0 | *    | 0x80 |
// + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
uint8_t CHIP8_FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

#define ROM_START           0x200
#define CHIP8_ASM_MAX_ROM   (0x1000 - ROM_START)
#define CHIP8_ASM_MAX_LINES 8192
#define CHIP8_ASM_MAX_LINE  512
#define CHIP8_ASM_MAX_SYMS  512

typedef struct {

    // Memory map:
    // +---------------+= 0xFFF (4095) End of CHIP-8 RAM
    // |               |
    // |               |
    // |               |
    // |               |
    // |               |
    // | 0x200 to 0xFFF|
    // |     CHIP-8    |
    // | Program / Data|
    // |     Space     |
    // |               |
    // |               |
    // |               |
    // +- - - - - - - -+= 0x600 (1536) Start of ETI 660 CHIP-8 programs
    // |               |
    // |               |
    // |               |
    // +---------------+= 0x200 (512) Start of most CHIP-8 programs
    // | 0x000 to 0x1FF|
    // | Reserved for  |
    // |  interpreter  |
    // +- - - - - - - -+= 0x0A0 (160) End of fontset
    // | 0x050 to 0x0A0|
    // | Fontset (5B * |
    // |  16 digits)   |
    // +- - - - - - - -+= 0x050 (80) Start of fontset
    // |               |
    // +---------------+= 0x000 (0) Start of CHIP-8 RAM
    uint8_t memory[4096]; // 4KB memory

    // The graphics of the CHIP-8 are black and white and the screen has
    // a total of 2048 pixels (64 x 32).
    uint8_t gfx[CHIP8_WIDTH * CHIP8_HEIGHT];

    // 16 general purpose 8-bit registers (V0 to VF)
    uint8_t V[16];

    // The CHIP-8 has a hexadecimal keypad (0x0-0xF)
    // Each key state is stored in this array.
    uint8_t key[16];

    // Stack (used to store return addresses when subroutines are called)
    uint16_t stack[16];

    // Compatibility flags for various quirks.
    // See chip8_quirk_t enum for details.
    uint32_t quirks;

    // Index register (only the lowest (rightmost) 12 bits are usually used)
    uint16_t I;

    // Program counter (used to store the currently executing address in memory)
    uint16_t pc;

    // CHIP-8 has 35 opcodes, all 2 bytes long
    uint16_t opcode;

    // There are two timers that count down at 60Hz when they are non-zero.
    // When they reach zero, they stop.
    // The system's buzzer sounds whenever the sound timer reaches zero.
    uint8_t delay_timer;
    uint8_t sound_timer;

    // Stack pointer (used to point to the topmost level of the stack)
    uint8_t sp;
    
    // Flag to indicate if the screen needs to be redrawn
    bool draw;

    // Flag to indicate if the CHIP-8 is running
    bool running;

    // Size of the loaded ROM in bytes (set by chip8_load_program and chip8_asm_load)
    size_t rom_size;

} chip8_t;

// Keyboard layout (PC keys -> CHIP-8 keys):
// +---+---+---+---+      +---+---+---+---+
// | 1 | 2 | 3 | 4 |      | 1 | 2 | 3 | C |
// +---+---+---+---+      +---+---+---+---+
// | Q | W | E | R |      | 4 | 5 | 6 | D |
// +---+---+---+---+  ->  +---+---+---+---+
// | A | S | D | F |      | 7 | 8 | 9 | E |
// +---+---+---+---+      +---+---+---+---+
// | Z | X | C | V |      | A | 0 | B | F |
// +---+---+---+---+      +---+---+---+---+
//
// Enum values are the canonical CHIP-8 hex key values (0x0..0xF).
// This lets chip8->key[VX & 0x0F] match EX9E/EXA1/FX0A semantics directly.
typedef enum {
    CHIP8_KEY_X = 0x0,
    CHIP8_KEY_1 = 0x1,
    CHIP8_KEY_2 = 0x2,
    CHIP8_KEY_3 = 0x3,
    CHIP8_KEY_Q = 0x4,
    CHIP8_KEY_W = 0x5,
    CHIP8_KEY_E = 0x6,
    CHIP8_KEY_A = 0x7,
    CHIP8_KEY_S = 0x8,
    CHIP8_KEY_D = 0x9,
    CHIP8_KEY_Z = 0xA,
    CHIP8_KEY_C = 0xB,
    CHIP8_KEY_4 = 0xC,
    CHIP8_KEY_R = 0xD,
    CHIP8_KEY_F = 0xE,
    CHIP8_KEY_V = 0xF,
    CHIP8_KEY_UNDEFINED = 0x10
} chip8_key_t;

typedef enum {
    CHIP8_KEY_UP   = 0,
    CHIP8_KEY_DOWN = 1
} chip8_key_value_t;

typedef enum {
    QUIRK_SHIFT      = 0x1, // 8XY6, 8XYE
    QUIRK_LOAD_STORE = 0x2, // FX55, FX65
    QUIRK_JUMP_REL   = 0x4, // BNNN
    QUIRK_VF_RESET   = 0x8  // 8XY1, 8XY2, 8XY3
} chip8_quirk_t;

// Logging macros.
#define LOG_INFO(msg) \
    do { \
        fprintf(stdout, "[INFO] - " msg "\n"); \
    } while(0)

#define LOG_INFO_ARGS(msg, ...) \
    do { \
        fprintf(stdout, "[INFO] - " msg "\n", __VA_ARGS__); \
    } while(0)

#define LOG_ERROR(msg) \
    do { \
        fprintf(stderr, "\033[0;31m[ERROR] - " msg "\033[0m\n"); \
    } while(0)

#define LOG_ERROR_ARGS(msg, ...) \
    do { \
        fprintf(stderr, "\033[0;31m[ERROR] - " msg "\033[0m\n", __VA_ARGS__); \
    } while(0)

// Every opcode is 2 bytes long.
// The opcode is stored in memory as two consecutive bytes.
// To fetch the opcode, we need to combine these two bytes into a single 16-bit value.
// This can be done by shifting the first byte to the left by 8 bits and then performing a
// bitwise OR with the second byte.
// For example, if memory[pc] = 0x6A and memory[pc + 1] = 0xBC:
// opcode = (0x6A << 8) | 0xBC = 0x6ABC
#define FETCH_OPCODE(chip8) \
    ((uint16_t)(chip8->memory[chip8->pc] << 8) | (uint16_t)(chip8->memory[chip8->pc + 1]))

// Function prototypes for emulation of the CHIP-8 system.
chip8_t *chip8_init(void);
void chip8_destroy(chip8_t *chip8);
void chip8_load_program(chip8_t *chip8, const char *program);
void chip8_update_timers(chip8_t *chip8);
void chip8_step(chip8_t *chip8);
void chip8_execute_opcode(chip8_t *chip8, uint16_t opcode);

#if defined(CHIP8_IMPLEMENTATION)

chip8_t *chip8_init(void) {
    chip8_t *chip8 = calloc(1, sizeof(chip8_t));
    if (!chip8) {
        LOG_ERROR("Could not allocate memory for CHIP-8 instance!");
        exit(1);
    }
    memcpy(chip8->memory, CHIP8_FONTSET, sizeof(CHIP8_FONTSET));

    chip8->running = true;      // Start the CHIP-8
    chip8->pc      = ROM_START; // Program counter starts at 0x200
    chip8->opcode  = 0;         // Reset initial opcode
    chip8->I       = 0;         // Reset index register
    chip8->sp      = 0;         // Reset stack pointer
    chip8->draw    = false;     // Do not draw
    
    // Use the standard quirks
    chip8->quirks  = QUIRK_SHIFT | QUIRK_LOAD_STORE;

    // Init Pseudo-Random Number Generator used for CXNN
    srand(time(NULL));

    return chip8;
}

void chip8_destroy(chip8_t *chip8) {
    if (chip8) {
        free(chip8);
    }
}

void chip8_load_program(chip8_t *chip8, const char *program) {
    FILE *f = fopen(program, "rb");
    if (!f) {
        LOG_ERROR_ARGS("Could not open ROM at path '%s'", program);
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        LOG_ERROR_ARGS("Could not seek to end of ROM file at path '%s'", program);
        exit(1);
    }
    size_t file_size = ftell(f);
    if ((file_size == 0) || (file_size > CHIP8_ASM_MAX_ROM)) {
        LOG_ERROR_ARGS("Could not load ROM into memory, the ROM is too big: %ld", file_size);
        exit(1);
    }

    rewind(f);
    size_t result = fread(chip8->memory + ROM_START, 1, file_size, f);
    if (result != file_size) {
        LOG_ERROR_ARGS("Could not load ROM into memory, the ROM is too big: %ld", file_size);
        exit(1);
    }
    chip8->rom_size = file_size;

    if (fclose(f) != 0) {
        LOG_ERROR_ARGS("Could not close ROM file at path '%s'", program);
        exit(1);
    }
}

void chip8_update_timers(chip8_t *chip8) {
    if (chip8->delay_timer > 0) {
        chip8->delay_timer -= 1;
    }

    if (chip8->sound_timer > 0) {
        chip8->sound_timer -= 1;
    }
}

void chip8_step(chip8_t *chip8) {
    if (chip8->pc >= 4095) {
        LOG_ERROR_ARGS("Could not increment PC: 0x%03X", chip8->pc);
        exit(1);
    }

    // Fetch stage
    uint16_t opcode = FETCH_OPCODE(chip8);
    chip8->opcode = opcode;
    chip8->pc += 2;
    chip8_execute_opcode(chip8, opcode);
}

void chip8_execute_opcode(chip8_t *chip8, uint16_t opcode) {
    // Decode stage
    // Opcode structure (16-bit, shown as 4 nibbles):
    //
    // +--------+--------+--------+--------+
    // | 15..12 | 11.. 8 |  7.. 4 |  3.. 0 |
    // +--------+--------+--------+--------+
    // |  type  |   X    |   Y    |   N    |
    // +--------+--------+--------+--------+
    //           <------- NNN ------------>
    //                    <----- NN ------>
    //                             <-- N ->
    //
    // Field  | Mask   | Shift | Bits | Description
    // -------+--------+-------+------+--------------------------------------
    //   X    | 0x0F00 |  >> 8 |   4  | Lower nibble of the high byte
    //   Y    | 0x00F0 |  >> 4 |   4  | Upper nibble of the low byte
    //   N    | 0x000F |   -   |   4  | Lower nibble of the low byte
    //   NN   | 0x00FF |   -   |   8  | Full low byte
    //   NNN  | 0x0FFF |   -   |  12  | Lower 12 bits (X, Y and N combined)
    //
    // Example: opcode = 0x6ABC
    // +--------+--------+--------+--------+
    // |  0x6   |  0xA   |  0xB   |  0xC   |
    // +--------+--------+--------+--------+
    //   type=6   X=10     Y=11     N=12
    //            NNN = 0xABC = 2748
    //                     NN = 0xBC = 188
    uint8_t  X   = (opcode & 0x0F00) >> 8;
    uint8_t  Y   = (opcode & 0x00F0) >> 4;
    uint16_t N   = opcode & 0x000F;
    uint16_t NN  = opcode & 0x00FF;
    uint16_t NNN = opcode & 0x0FFF;

    if (X >= 16) {
        LOG_ERROR_ARGS("Invalid value for X: %d > 15", X);
        exit(1);
    }

    if (Y >= 16) {
        LOG_ERROR_ARGS("Invalid value for Y: %d > 15", Y);
        exit(1);
    }

#ifdef DEBUG
    LOG_INFO_ARGS("[0x%03X] Executing 0x%04X", chip8->pc - 2,  opcode);
#endif

    // Execute stage
    switch ((opcode & 0xF000) >> 12) {
        case 0x0: {
            switch (opcode & 0x00FF) {
                case 0xE0: // 00E0, clear the screen
                    memset(chip8->gfx, 0, CHIP8_WIDTH * CHIP8_HEIGHT);
                    break;
                
                case 0xEE: // 00EE, return from subroutine
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    break;
                default: // 0NNN, execute machine language subroutine at address NNN
                    LOG_INFO("Executing 0NNN");
                    break;
            }
        } break;

        case 0x1: {
            // 1NNN, jump to address NNN
            chip8->pc = NNN;
        } break;

        case 0x2: {
            // 2NNN, execute subroutine starting at address NNN

            // We make sure there is enough space in the stack.
            if (chip8->sp == 16) {
                LOG_ERROR("Call stack overflow!");
                exit(1);
            }

            // We need to save the return address in the call stack.
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;

            // After we have saved the return address in the stack
            // we can jump to the new address to execute the new subroutine.
            chip8->pc = NNN;
        } break;

        case 0x3: {
            // 3XNN, Skip instruction if VX == NN
            if (chip8->V[X] == NN) {
                chip8->pc += 2;
            }
        } break;

        case 0x4: {
            // 4XNN, Skip instruction if VX != NN
            if (chip8->V[X] != NN) {
                chip8->pc += 2;
            }
        } break;

        case 0x5: {
            // 5XY0, Skip instruction if  VX == VY
            if (chip8->V[X] == chip8->V[Y]) {
                chip8->pc += 2;
            }
        } break;

        case 0x6: {
            // 6XNN, LD VX, NN
            chip8->V[X] = NN;
        } break;

        case 0x7: {
            // 7XNN, ADD VX, NN
            chip8->V[X] += NN;
        } break;

        case 0x8: {
            switch (opcode & 0x000F) {
                case 0x0: // 8XY0 - LD VX, VY
                    chip8->V[X] = chip8->V[Y];
                    break;

                case 0x1: // 8XY1 - OR
                    chip8->V[X] |= chip8->V[Y];
                    if (!(chip8->quirks & QUIRK_VF_RESET)) chip8->V[15] = 0;
                    break;

                case 0x2: // 8XY2 - AND
                    chip8->V[X] &= chip8->V[Y];
                    if (!(chip8->quirks & QUIRK_VF_RESET)) chip8->V[15] = 0;
                    break;

                case 0x3: // 8XY3 - XOR
                    chip8->V[X] ^= chip8->V[Y];
                    if (!(chip8->quirks & QUIRK_VF_RESET)) chip8->V[15] = 0;
                    break;

                case 0x4: { // 8XY4 - ADD VX, VY
                    uint8_t carry = (chip8->V[X] + chip8->V[Y] > 255) ? 1 : 0;
                    chip8->V[X] += chip8->V[Y];
                    chip8->V[15] = carry;
                    break;
                }

                case 0x5: { // 8XY5 - SUB VX, VY
                    uint8_t not_borrow = (chip8->V[X] >= chip8->V[Y]) ? 1 : 0;
                    chip8->V[X] -= chip8->V[Y];
                    chip8->V[15] = not_borrow;
                    break;
                }

                case 0x6: { // 8XY6 - SHR
                    if (!(chip8->quirks & QUIRK_SHIFT)) chip8->V[X] = chip8->V[Y];
                    uint8_t lsb = chip8->V[X] & 0x1;
                    chip8->V[X] >>= 1;
                    chip8->V[15] = lsb;
                    break;
                }

                case 0x7: { // 8XY7 - SUBN
                    uint8_t not_borrow = (chip8->V[Y] >= chip8->V[X]) ? 1 : 0;
                    chip8->V[X] = chip8->V[Y] - chip8->V[X];
                    chip8->V[15] = not_borrow;
                    break;
                }

                case 0xE: { // 8XYE - SHL
                    if (!(chip8->quirks & QUIRK_SHIFT)) chip8->V[X] = chip8->V[Y];
                    uint8_t msb = (chip8->V[X] & 0x80) >> 7;
                    chip8->V[X] <<= 1;
                    chip8->V[15] = msb;
                    break;
                }

                default:
                    LOG_ERROR_ARGS("Unknown opcode '0x%x'", opcode);
                    exit(1);
            }
        } break;

        case 0x9: {
            // 9XY0, Skip the next instruction if VX != VY
            if (chip8->V[X] != chip8->V[Y]) {
                chip8->pc += 2;
            }
        } break;

        case 0xA: {
            // ANNN, LD I, NNN
            chip8->I = NNN;
        } break;

        case 0xB: {
            // BNNN, Jump to the address NNN plus V0
            uint16_t offset = chip8->quirks & QUIRK_JUMP_REL ? chip8->V[X] : chip8->V[0];
            chip8->pc = NNN + offset;
        } break;

        case 0xC: {
            // CXNN, Set VX to a random number with a mask of NN
            chip8->V[X] = (uint8_t)((rand() & 0xFF) & NN);
        } break;

        case 0xD: {
            // DXYN,
            // 1. Draw a sprite at position VX, VY with N bytes of sprite
            // data starting at the address stored in I.
            // 2. Set VF to 01 if any set pixels are changed to unset, and
            // 00 otherwise
            //
            // To draw the sprite we need to know:
            // 1. Starting location (x, y), obtained from opcode
            // 2. Number of columns, fixed at 8 (1 byte)
            // 3. Number of rows, obtained from opcode
            // 4. Memory to load from, obtained from register I

            uint8_t x = chip8->V[X];
            uint8_t y = chip8->V[Y];

            chip8->V[15] = 0;
            for (int yline = 0; yline < N; yline++) {
                if (y + yline >= CHIP8_HEIGHT) break;
                uint8_t row = chip8->memory[chip8->I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    uint8_t bit_value = row & (0x80 >> xline);
                    if (bit_value != 0) {
                        uint32_t bit_index = (x + xline) + (y + yline) * CHIP8_WIDTH;
                        if (chip8->gfx[bit_index] == 1) {
                            chip8->V[15] = 1; // collision
                        }
                        chip8->gfx[bit_index] ^= 1;
                    }
                }
            }
            
            chip8->draw = true;
        } break;

        case 0xE: {
            switch (opcode & 0x00FF) {
                case 0x9E: // EX9E, Skip the next instruction if the key stored in VX is pressed
                    if (chip8->key[chip8->V[X] & 0x0F] == CHIP8_KEY_DOWN) {
                        chip8->pc += 2;
                    }
                    break;

                case 0xA1: // EXA1, Skip the next instruction if the key stored in VX isn't pressed
                    if (chip8->key[chip8->V[X] & 0x0F] == CHIP8_KEY_UP) {
                        chip8->pc += 2;
                    }
                    break;

                default:
                    LOG_ERROR_ARGS("Unknown opcode '0x%x'", opcode);
                    exit(1);
            }
        } break;

        case 0xF: {
            switch (opcode & 0x00FF) {
                case 0x07: // FX07 - VX = delay_timer
                    chip8->V[X] = chip8->delay_timer;
                    break;

                case 0x0A: { // FX0A - wait keypress
                    uint8_t key_press = 0;
                    for (int i = 0; i < 16; i++) {
                        if (chip8->key[i] == CHIP8_KEY_DOWN) {
                            chip8->V[X] = i;
                            key_press = 1;
                            break;
                        }
                    }

                    if (!key_press) {
                        chip8->pc -= 2;
                        return;
                    }

                    break;
                }

                case 0x15: // FX15 - delay_timer = VX
                    chip8->delay_timer = chip8->V[X];
                    break;

                case 0x18: // FX18 - sound_timer = VX
                    chip8->sound_timer = chip8->V[X];
                    break;

                case 0x1E: // FX1E - I += VX
                    chip8->I += chip8->V[X];
                    break;

                case 0x29: // FX29 - I = font sprite for VX
                    chip8->I = (chip8->V[X] & 0x0F) * 0x5;
                    break;

                case 0x33: { // FX33 - BCD
                    uint8_t value = chip8->V[X];
                    chip8->memory[chip8->I + 0] = value / 100;
                    chip8->memory[chip8->I + 1] = (value % 100) / 10;
                    chip8->memory[chip8->I + 2] = value % 10;
                    break;
                }

                case 0x55: { // FX55 - store V0..VX in memory[I]
                    if (chip8->I + X >= 4096) {
                        LOG_ERROR("Memory access out of bounds in FX55!");
                        exit(1);
                    }

                    for (uint8_t i = 0; i <= X; i++) {
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    }

                    // If the quirk is active, we do not need to update the I
                    // register. Otherwise we update it accordingly to the original
                    // behavior.
                    if (!(chip8->quirks & QUIRK_LOAD_STORE)) {
                        chip8->I += X + 1;
                    }

                    break;
                }

                case 0x65: {  // FX65 - load V0..VX from memory[I]
                    if (chip8->I + X >= 4096) {
                        LOG_ERROR("Memory access out of bounds in FX65!");
                        exit(1);
                    }

                    for (uint8_t i = 0; i <= X; i++) {
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    }

                    // If the quirk is active, we do not need to update the I
                    // register. Otherwise we update it accordingly to the original
                    // behavior.
                    if (!(chip8->quirks & QUIRK_LOAD_STORE)) {
                        chip8->I += X + 1;
                    }

                    break;
                }

                default:
                    LOG_ERROR_ARGS("Unknown opcode: 0x%04X\n", opcode);
                    exit(1);
            }
        } break;
    }
}

#endif // CHIP8_IMPLEMENTATION
