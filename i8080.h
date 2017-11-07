#ifndef I8080_I8080_H_
#define I8080_I8080_H_

#include <stdio.h>
#include "types.h"

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

// memory helpers (the only four functions to use `read_byte` and `write_byte`
// function pointers)
u8 i8080_rb(i8080* const c, const u16 addr);
void i8080_wb(i8080* const c, const u16 addr, const u8 val);
u16 i8080_rw(i8080* const c, const u16 addr);
void i8080_ww(i8080* const c, const u16 addr, const u16 val);

u8 i8080_next_byte(i8080* const c);
u16 i8080_next_word(i8080* const c);

// paired registers helpers
void i8080_set_bc(i8080* const c, const u16 val);
void i8080_set_de(i8080* const c, const u16 val);
void i8080_set_hl(i8080* const c, const u16 val);
u16 i8080_get_bc(i8080* const c);
u16 i8080_get_de(i8080* const c);
u16 i8080_get_hl(i8080* const c);

// stack helpers
void i8080_push_stack(i8080* const c, const u16 val);
u16 i8080_pop_stack(i8080* const c);

// instructions
bool parity(const u8 val);
void i8080_add(i8080* const c, u8* const reg, const u8 val, const bool cy);
void i8080_sub(i8080* const c, u8* const reg, const u8 val, const bool cy);
void i8080_dad(i8080* const c, const u16 val);
u8 i8080_inr(i8080* const c, const u8 val);
u8 i8080_dcr(i8080* const c, const u8 val);
void i8080_ana(i8080* const c, const u8 val);
void i8080_xra(i8080* const c, const u8 val);
void i8080_ora(i8080* const c, const u8 val);
void i8080_cmp(i8080* const c, const u8 val);

void i8080_jmp(i8080* const c, const u16 addr);
void i8080_cond_jmp(i8080* const c, const bool condition);
void i8080_call(i8080* const c, const u16 addr);
void i8080_cond_call(i8080* const c, const bool condition);
void i8080_ret(i8080* const c);
void i8080_cond_ret(i8080* const c, const bool condition);

void i8080_push_psw(i8080* const c);
void i8080_pop_psw(i8080* const c);

void i8080_rlc(i8080* const c);
void i8080_rrc(i8080* const c);
void i8080_ral(i8080* const c);
void i8080_rar(i8080* const c);

void i8080_daa(i8080* const c);
void i8080_xchg(i8080* const c);
void i8080_xthl(i8080* const c);

// this array defines the number of cycles one opcode takes.
// if the value is == 0, that means the cycles count is handled in the opcode
// implementation
static const u8 OPCODES_CYCLES[] = {
    4, 10, 7, 6, 5, 5, 7, 4, 4, 11, 7, 6, 5, 5, 7, 4,
    4, 10, 7, 6, 5, 5, 7, 4, 4, 11, 7, 6, 5, 5, 7, 4,
    4, 10, 16, 6, 5, 5, 7, 4, 4, 11, 16, 6, 5, 5, 7, 4,
    4, 10, 13, 6, 10, 10, 10, 4, 4, 11, 13, 6, 5, 5, 7, 4,
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    0, 10, 0, 10, 0, 11, 7, 11, 0, 10, 0, 10, 0, 17, 7, 11,
    0, 10, 0, 10, 0, 11, 7, 11, 0, 10, 0, 10, 0, 17, 7, 11,
    0, 10, 0, 4, 0, 11, 7, 11, 0, 10, 0, 4, 0, 17, 7, 11,
    0, 10, 0, 4, 0, 11, 7, 11, 0, 10, 0, 4, 0, 17, 7, 11
};

#endif  // I8080_I8080_H_
