#ifndef I8080_I8080_H_
#define I8080_I8080_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;

typedef struct i8080 {
    u16 pc, sp; // program counter, stack pointer
    u8 a, b, c, d, e, h, l; // registers
    bool s, z, hc, p, cy, iff; // flags: sign, zero, half-carry, parity, carry,
                               //        interrupt flip-flop

    u32 cyc; // cycles count

    // function pointers to access memory
    u8 (*read_byte)(u16);
    void (*write_byte)(u16, u8);
} i8080;

void i8080_init(i8080* const c);
void i8080_step(i8080* const c);
void i8080_debug_output(i8080* const c);
void i8080_run_testrom(i8080* const c);

#endif  // I8080_I8080_H_
