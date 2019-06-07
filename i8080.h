#ifndef I8080_I8080_H_
#define I8080_I8080_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;

typedef struct i8080 {
    u16 pc, sp; // program counter, stack pointer
    u8 a, b, c, d, e, h, l; // registers
    bool sf, zf, hf, pf, cf, iff; // flags: sign, zero, half-carry, parity,
                                  // carry, interrupt flip-flop

    unsigned long cyc; // cycle count
    bool halted;
    bool interrupt_pending;
    u8 interrupt_vector;
    u8 interrupt_delay;

    // memory + io interface
    void* userdata; // general purpose pointer for the user
    u8 (*read_byte)(void*, u16);
    void (*write_byte)(void*, u16, u8);
    u8 (*port_in)(void*, u8);
    void (*port_out)(void*, u8, u8);
} i8080;

void i8080_init(i8080* const c);
void i8080_step(i8080* const c);
void i8080_interrupt(i8080* const c, u8 opcode);
void i8080_debug_output(i8080* const c);

#endif  // I8080_I8080_H_
