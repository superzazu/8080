#include "i8080.h"

// memory helpers (the only four functions to use `read_byte` and `write_byte`
// function pointers)
static u8 i8080_rb(i8080* const c, const u16 addr);
static void i8080_wb(i8080* const c, const u16 addr, const u8 val);
static u16 i8080_rw(i8080* const c, const u16 addr);
static void i8080_ww(i8080* const c, const u16 addr, const u16 val);

static u8 i8080_next_byte(i8080* const c);
static u16 i8080_next_word(i8080* const c);

// paired registers helpers
static void i8080_set_bc(i8080* const c, const u16 val);
static void i8080_set_de(i8080* const c, const u16 val);
static void i8080_set_hl(i8080* const c, const u16 val);
static u16 i8080_get_bc(i8080* const c);
static u16 i8080_get_de(i8080* const c);
static u16 i8080_get_hl(i8080* const c);

// stack helpers
static void i8080_push_stack(i8080* const c, const u16 val);
static u16 i8080_pop_stack(i8080* const c);

// instructions
static bool parity(const u8 val);
static void i8080_add(i8080* const c, u8* const reg, const u8 val,
                      const bool cy);
static void i8080_sub(i8080* const c, u8* const reg, const u8 val,
                      const bool cy);
static void i8080_dad(i8080* const c, const u16 val);
static u8 i8080_inr(i8080* const c, const u8 val);
static u8 i8080_dcr(i8080* const c, const u8 val);
static void i8080_ana(i8080* const c, const u8 val);
static void i8080_xra(i8080* const c, const u8 val);
static void i8080_ora(i8080* const c, const u8 val);
static void i8080_cmp(i8080* const c, const u8 val);

static void i8080_jmp(i8080* const c, const u16 addr);
static void i8080_cond_jmp(i8080* const c, const bool condition);
static void i8080_call(i8080* const c, const u16 addr);
static void i8080_cond_call(i8080* const c, const bool condition);
static void i8080_ret(i8080* const c);
static void i8080_cond_ret(i8080* const c, const bool condition);

static void i8080_push_psw(i8080* const c);
static void i8080_pop_psw(i8080* const c);

static void i8080_rlc(i8080* const c);
static void i8080_rrc(i8080* const c);
static void i8080_ral(i8080* const c);
static void i8080_rar(i8080* const c);

static void i8080_daa(i8080* const c);
static void i8080_xchg(i8080* const c);
static void i8080_xthl(i8080* const c);

// this array defines the number of cycles one opcode takes.
// note that there are some special cases: conditional RETs and CALLs
// add +6 cycles if the condition is met
static const u8 OPCODES_CYCLES[] = {
//  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  // 0
    4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  // 1
    4,  10, 16, 5,  5,  5,  7,  4,  4,  10, 16, 5,  5,  5,  7,  4,  // 2
    4,  10, 13, 5,  10, 10, 10, 4,  4,  10, 13, 5,  5,  5,  7,  4,  // 3
    5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  // 4
    5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  // 5
    5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  // 6
    7,  7,  7,  7,  7,  7,  7,  7,  5,  5,  5,  5,  5,  5,  7,  5,  // 7
    4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  // 8
    4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  // 9
    4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  // A
    4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  // B
    5,  10, 10, 10, 11, 11, 7,  11, 5,  10, 10, 10, 11, 11, 7,  11, // C
    5,  10, 10, 10, 11, 11, 7,  11, 5,  10, 10, 10, 11, 11, 7,  11, // D
    5,  10, 10, 18, 11, 11, 7,  11, 5,  5,  10, 5,  11, 11, 7,  11, // E
    5,  10, 10, 4,  11, 11, 7,  11, 5,  5,  10, 4,  11, 11, 7,  11  // F
};

static const char* DISASSEMBLE_TABLE[] = {
    "nop", "lxi b,#", "stax b", "inx b", "inr b", "dcr b", "mvi b,#", "rlc",
    "ill", "dad b", "ldax b", "dcx b", "inr c", "dcr c", "mvi c,#", "rrc",
    "ill", "lxi d,#", "stax d", "inx d", "inr d", "dcr d", "mvi d,#", "ral",
    "ill", "dad d", "ldax d", "dcx d", "inr e", "dcr e", "mvi e,#", "rar",
    "ill", "lxi h,#", "shld", "inx h", "inr h", "dcr h", "mvi h,#", "daa",
    "ill", "dad h", "lhld", "dcx h", "inr l", "dcr l", "mvi l,#", "cma",
    "ill", "lxi sp,#","sta $", "inx sp", "inr M", "dcr M", "mvi M,#", "stc",
    "ill", "dad sp", "lda $", "dcx sp", "inr a", "dcr a", "mvi a,#", "cmc",
    "mov b,b", "mov b,c", "mov b,d", "mov b,e", "mov b,h", "mov b,l",
    "mov b,M", "mov b,a", "mov c,b", "mov c,c", "mov c,d", "mov c,e",
    "mov c,h", "mov c,l", "mov c,M", "mov c,a", "mov d,b", "mov d,c",
    "mov d,d", "mov d,e", "mov d,h", "mov d,l", "mov d,M", "mov d,a",
    "mov e,b", "mov e,c", "mov e,d", "mov e,e", "mov e,h", "mov e,l",
    "mov e,M", "mov e,a", "mov h,b", "mov h,c", "mov h,d", "mov h,e",
    "mov h,h", "mov h,l", "mov h,M", "mov h,a", "mov l,b", "mov l,c",
    "mov l,d", "mov l,e", "mov l,h", "mov l,l", "mov l,M", "mov l,a",
    "mov M,b", "mov M,c", "mov M,d", "mov M,e", "mov M,h", "mov M,l", "hlt",
    "mov M,a", "mov a,b", "mov a,c", "mov a,d", "mov a,e", "mov a,h",
    "mov a,l", "mov a,M", "mov a,a", "add b", "add c", "add d", "add e",
    "add h", "add l", "add M", "add a", "adc b", "adc c", "adc d", "adc e",
    "adc h", "adc l", "adc M", "adc a", "sub b", "sub c", "sub d", "sub e",
    "sub h", "sub l", "sub M", "sub a", "sbb b", "sbb c", "sbb d", "sbb e",
    "sbb h", "sbb l", "sbb M", "sbb a", "ana b", "ana c", "ana d", "ana e",
    "ana h", "ana l", "ana M", "ana a", "xra b", "xra c", "xra d", "xra e",
    "xra h", "xra l", "xra M", "xra a", "ora b", "ora c", "ora d", "ora e",
    "ora h", "ora l", "ora M", "ora a", "cmp b", "cmp c", "cmp d", "cmp e",
    "cmp h", "cmp l", "cmp M", "cmp a", "rnz", "pop b", "jnz $", "jmp $",
    "cnz $", "push b", "adi #", "rst 0", "rz", "ret", "jz $", "ill", "cz $",
    "call $", "aci #", "rst 1", "rnc", "pop d", "jnc $", "out p", "cnc $",
    "push d", "sui #", "rst 2", "rc", "ill", "jc $", "in p", "cc $", "ill",
    "sbi #", "rst 3", "rpo", "pop h", "jpo $", "xthl", "cpo $", "push h",
    "ani #", "rst 4", "rpe", "pchl", "jpe $", "xchg", "cpe $", "ill", "xri #",
    "rst 5", "rp", "pop psw", "jp $", "di", "cp $", "push psw","ori #",
    "rst 6", "rm", "sphl", "jm $", "ei", "cm $", "ill", "cpi #", "rst 7"
};


// initialises the emulator with default values
void i8080_init(i8080* const c) {
    i8080_reset(c);
    c->userdata = NULL;
    c->read_byte = NULL;
    c->write_byte = NULL;
}

void i8080_reset(i8080* const c) {
    c->pc = 0;
    c->sp = 0;

    c->a = 0;
    c->b = 0;
    c->c = 0;
    c->d = 0;
    c->e = 0;
    c->h = 0;
    c->l = 0;

    c->s = false;
    c->z = false;
    c->hc = false;
    c->p = false;
    c->cy = false;
    c->iff = false;

    c->cyc = 0;
}

// executes one opcode stored at the address pointed by the program counter
void i8080_step(i8080* const c) {
    const u8 opcode = i8080_next_byte(c);
    c->cyc += OPCODES_CYCLES[opcode];

    switch (opcode) {
    // 8 bit transfer instructions
    case 0x7F: c->a = c->a; break; // MOV A,A
    case 0x78: c->a = c->b; break; // MOV A,B
    case 0x79: c->a = c->c; break; // MOV A,C
    case 0x7A: c->a = c->d; break; // MOV A,D
    case 0x7B: c->a = c->e; break; // MOV A,E
    case 0x7C: c->a = c->h; break; // MOV A,H
    case 0x7D: c->a = c->l; break; // MOV A,L
    case 0x7E: c->a = i8080_rb(c, i8080_get_hl(c)); break; // MOV A,M

    case 0x0A: c->a = i8080_rb(c, i8080_get_bc(c)); break; // LDAX B
    case 0x1A: c->a = i8080_rb(c, i8080_get_de(c)); break; // LDAX D
    case 0x3A: c->a = i8080_rb(c, i8080_next_word(c)); break; // LDA word

    case 0x47: c->b = c->a; break; // MOV B,A
    case 0x40: c->b = c->b; break; // MOV B,B
    case 0x41: c->b = c->c; break; // MOV B,C
    case 0x42: c->b = c->d; break; // MOV B,D
    case 0x43: c->b = c->e; break; // MOV B,E
    case 0x44: c->b = c->h; break; // MOV B,H
    case 0x45: c->b = c->l; break; // MOV B,L
    case 0x46: c->b = i8080_rb(c, i8080_get_hl(c)); break; // MOV B,M

    case 0x4F: c->c = c->a; break; // MOV C,A
    case 0x48: c->c = c->b; break; // MOV C,B
    case 0x49: c->c = c->c; break; // MOV C,C
    case 0x4A: c->c = c->d; break; // MOV C,D
    case 0x4B: c->c = c->e; break; // MOV C,E
    case 0x4C: c->c = c->h; break; // MOV C,H
    case 0x4D: c->c = c->l; break; // MOV C,L
    case 0x4E: c->c = i8080_rb(c, i8080_get_hl(c)); break; // MOV C,M

    case 0x57: c->d = c->a; break; // MOV D,A
    case 0x50: c->d = c->b; break; // MOV D,B
    case 0x51: c->d = c->c; break; // MOV D,C
    case 0x52: c->d = c->d; break; // MOV D,D
    case 0x53: c->d = c->e; break; // MOV D,E
    case 0x54: c->d = c->h; break; // MOV D,H
    case 0x55: c->d = c->l; break; // MOV D,L
    case 0x56: c->d = i8080_rb(c, i8080_get_hl(c)); break; // MOV D,M

    case 0x5F: c->e = c->a; break; // MOV E,A
    case 0x58: c->e = c->b; break; // MOV E,B
    case 0x59: c->e = c->c; break; // MOV E,C
    case 0x5A: c->e = c->d; break; // MOV E,D
    case 0x5B: c->e = c->e; break; // MOV E,E
    case 0x5C: c->e = c->h; break; // MOV E,H
    case 0x5D: c->e = c->l; break; // MOV E,L
    case 0x5E: c->e = i8080_rb(c, i8080_get_hl(c)); break; // MOV E,M

    case 0x67: c->h = c->a; break; // MOV H,A
    case 0x60: c->h = c->b; break; // MOV H,B
    case 0x61: c->h = c->c; break; // MOV H,C
    case 0x62: c->h = c->d; break; // MOV H,D
    case 0x63: c->h = c->e; break; // MOV H,E
    case 0x64: c->h = c->h; break; // MOV H,H
    case 0x65: c->h = c->l; break; // MOV H,L
    case 0x66: c->h = i8080_rb(c, i8080_get_hl(c)); break; // MOV H,M

    case 0x6F: c->l = c->a; break; // MOV L,A
    case 0x68: c->l = c->b; break; // MOV L,B
    case 0x69: c->l = c->c; break; // MOV L,C
    case 0x6A: c->l = c->d; break; // MOV L,D
    case 0x6B: c->l = c->e; break; // MOV L,E
    case 0x6C: c->l = c->h; break; // MOV L,H
    case 0x6D: c->l = c->l; break; // MOV L,L
    case 0x6E: c->l = i8080_rb(c, i8080_get_hl(c)); break; // MOV L,M

    case 0x77: i8080_wb(c, i8080_get_hl(c), c->a); break; // MOV M,A
    case 0x70: i8080_wb(c, i8080_get_hl(c), c->b); break; // MOV M,B
    case 0x71: i8080_wb(c, i8080_get_hl(c), c->c); break; // MOV M,C
    case 0x72: i8080_wb(c, i8080_get_hl(c), c->d); break; // MOV M,D
    case 0x73: i8080_wb(c, i8080_get_hl(c), c->e); break; // MOV M,E
    case 0x74: i8080_wb(c, i8080_get_hl(c), c->h); break; // MOV M,H
    case 0x75: i8080_wb(c, i8080_get_hl(c), c->l); break; // MOV M,L

    case 0x3E: c->a = i8080_next_byte(c); break; // MVI A,byte
    case 0x06: c->b = i8080_next_byte(c); break; // MVI B,byte
    case 0x0E: c->c = i8080_next_byte(c); break; // MVI C,byte
    case 0x16: c->d = i8080_next_byte(c); break; // MVI D,byte
    case 0x1E: c->e = i8080_next_byte(c); break; // MVI E,byte
    case 0x26: c->h = i8080_next_byte(c); break; // MVI H,byte
    case 0x2E: c->l = i8080_next_byte(c); break; // MVI L,byte
    case 0x36:
        i8080_wb(c, i8080_get_hl(c), i8080_next_byte(c));
    break; // MVI M,byte

    case 0x02: i8080_wb(c, i8080_get_bc(c), c->a); break; // STAX B
    case 0x12: i8080_wb(c, i8080_get_de(c), c->a); break; // STAX D
    case 0x32: i8080_wb(c, i8080_next_word(c), c->a); break; // STA word

    // 16 bit transfer instructions
    case 0x01: i8080_set_bc(c, i8080_next_word(c)); break; // LXI B,word
    case 0x11: i8080_set_de(c, i8080_next_word(c)); break; // LXI D,word
    case 0x21: i8080_set_hl(c, i8080_next_word(c)); break; // LXI H,word
    case 0x31: c->sp = i8080_next_word(c); break; // LXI SP,word
    case 0x2A: i8080_set_hl(c, i8080_rw(c, i8080_next_word(c))); break; // LHLD
    case 0x22: i8080_ww(c, i8080_next_word(c), i8080_get_hl(c)); break; // SHLD
    case 0xF9: c->sp = i8080_get_hl(c); break; // SPHL

    // register exchange instructions
    case 0xEB: i8080_xchg(c); break; // XCHG
    case 0xE3: i8080_xthl(c); break; // XTHL

    // add byte instructions
    case 0x87: i8080_add(c, &c->a, c->a, 0); break; // ADD A
    case 0x80: i8080_add(c, &c->a, c->b, 0); break; // ADD B
    case 0x81: i8080_add(c, &c->a, c->c, 0); break; // ADD C
    case 0x82: i8080_add(c, &c->a, c->d, 0); break; // ADD D
    case 0x83: i8080_add(c, &c->a, c->e, 0); break; // ADD E
    case 0x84: i8080_add(c, &c->a, c->h, 0); break; // ADD H
    case 0x85: i8080_add(c, &c->a, c->l, 0); break; // ADD L
    case 0x86:
        i8080_add(c, &c->a, i8080_rb(c, i8080_get_hl(c)), 0);
    break; // ADD M
    case 0xC6: i8080_add(c, &c->a, i8080_next_byte(c), 0); break; // ADI byte

    // add byte with carry-in instructions
    case 0x8F: i8080_add(c, &c->a, c->a, c->cy); break; // ADC A
    case 0x88: i8080_add(c, &c->a, c->b, c->cy); break; // ADC B
    case 0x89: i8080_add(c, &c->a, c->c, c->cy); break; // ADC C
    case 0x8A: i8080_add(c, &c->a, c->d, c->cy); break; // ADC D
    case 0x8B: i8080_add(c, &c->a, c->e, c->cy); break; // ADC E
    case 0x8C: i8080_add(c, &c->a, c->h, c->cy); break; // ADC H
    case 0x8D: i8080_add(c, &c->a, c->l, c->cy); break; // ADC L
    case 0x8E:
        i8080_add(c, &c->a, i8080_rb(c, i8080_get_hl(c)), c->cy);
    break; // ADC M
    case 0xCE:
        i8080_add(c, &c->a, i8080_next_byte(c), c->cy);
    break; // ACI byte

    // substract byte instructions
    case 0x97: i8080_sub(c, &c->a, c->a, 0); break; // SUB A
    case 0x90: i8080_sub(c, &c->a, c->b, 0); break; // SUB B
    case 0x91: i8080_sub(c, &c->a, c->c, 0); break; // SUB C
    case 0x92: i8080_sub(c, &c->a, c->d, 0); break; // SUB D
    case 0x93: i8080_sub(c, &c->a, c->e, 0); break; // SUB E
    case 0x94: i8080_sub(c, &c->a, c->h, 0); break; // SUB H
    case 0x95: i8080_sub(c, &c->a, c->l, 0); break; // SUB L
    case 0x96:
        i8080_sub(c, &c->a, i8080_rb(c, i8080_get_hl(c)), 0);
    break; // SUB M
    case 0xD6: i8080_sub(c, &c->a, i8080_next_byte(c), 0); break; // SUI byte

    // substract byte with borrow-in instructions
    case 0x9F: i8080_sub(c, &c->a, c->a, c->cy); break; // SBB A
    case 0x98: i8080_sub(c, &c->a, c->b, c->cy); break; // SBB B
    case 0x99: i8080_sub(c, &c->a, c->c, c->cy); break; // SBB C
    case 0x9A: i8080_sub(c, &c->a, c->d, c->cy); break; // SBB D
    case 0x9B: i8080_sub(c, &c->a, c->e, c->cy); break; // SBB E
    case 0x9C: i8080_sub(c, &c->a, c->h, c->cy); break; // SBB H
    case 0x9D: i8080_sub(c, &c->a, c->l, c->cy); break; // SBB L
    case 0x9E:
        i8080_sub(c, &c->a, i8080_rb(c, i8080_get_hl(c)), c->cy);
    break; // SBB M
    case 0xDE:
        i8080_sub(c, &c->a, i8080_next_byte(c), c->cy);
    break; // SBI byte

    // double byte add instructions
    case 0x09: i8080_dad(c, i8080_get_bc(c)); break; // DAD B
    case 0x19: i8080_dad(c, i8080_get_de(c)); break; // DAD D
    case 0x29: i8080_dad(c, i8080_get_hl(c)); break; // DAD H
    case 0x39: i8080_dad(c, c->sp); break; // DAD SP

    // control instructions
    case 0xF3: c->iff = 0; break; // DI
    case 0xFB: c->iff = 1; break; // EI
    case 0x00: break; // NOP
    case 0x76: c->pc--; break; // HLT

    // increment byte instructions
    case 0x3C: c->a = i8080_inr(c, c->a); break; // INR A
    case 0x04: c->b = i8080_inr(c, c->b); break; // INR B
    case 0x0C: c->c = i8080_inr(c, c->c); break; // INR C
    case 0x14: c->d = i8080_inr(c, c->d); break; // INR D
    case 0x1C: c->e = i8080_inr(c, c->e); break; // INR E
    case 0x24: c->h = i8080_inr(c, c->h); break; // INR H
    case 0x2C: c->l = i8080_inr(c, c->l); break; // INR L
    case 0x34:
        i8080_wb(c, i8080_get_hl(c),
                 i8080_inr(c, i8080_rb(c, i8080_get_hl(c))));
    break; // INR M

    // decrement byte instructions
    case 0x3D: c->a = i8080_dcr(c, c->a); break; // DCR A
    case 0x05: c->b = i8080_dcr(c, c->b); break; // DCR B
    case 0x0D: c->c = i8080_dcr(c, c->c); break; // DCR C
    case 0x15: c->d = i8080_dcr(c, c->d); break; // DCR D
    case 0x1D: c->e = i8080_dcr(c, c->e); break; // DCR E
    case 0x25: c->h = i8080_dcr(c, c->h); break; // DCR H
    case 0x2D: c->l = i8080_dcr(c, c->l); break; // DCR L
    case 0x35:
        i8080_wb(c, i8080_get_hl(c),
                 i8080_dcr(c, i8080_rb(c, i8080_get_hl(c))));
    break; // DCR M

    // increment register pair instructions
    case 0x03: i8080_set_bc(c, i8080_get_bc(c) + 1); break; // INX B
    case 0x13: i8080_set_de(c, i8080_get_de(c) + 1); break; // INX D
    case 0x23: i8080_set_hl(c, i8080_get_hl(c) + 1); break; // INX H
    case 0x33: c->sp += 1; break; // INX SP

    // decrement register pair instructions
    case 0x0B: i8080_set_bc(c, i8080_get_bc(c) - 1); break; // DCX B
    case 0x1B: i8080_set_de(c, i8080_get_de(c) - 1); break; // DCX D
    case 0x2B: i8080_set_hl(c, i8080_get_hl(c) - 1); break; // DCX H
    case 0x3B: c->sp -= 1; break; // DCX SP

    // special accumulator and flag instructions
    case 0x27: i8080_daa(c); break; // DAA
    case 0x2F: c->a ^= 0xFF; break; // CMA
    case 0x37: c->cy = 1; break; // STC
    case 0x3F: c->cy = !c->cy; break; // CMC

    // rotate instructions
    case 0x07: i8080_rlc(c); break; // RLC (rotate left)
    case 0x0F: i8080_rrc(c); break; // RRC (rotate right)
    case 0x17: i8080_ral(c); break; // RAL
    case 0x1F: i8080_rar(c); break; // RAR

    // logical byte instructions
    case 0xA7: i8080_ana(c, c->a); break; // ANA A
    case 0xA0: i8080_ana(c, c->b); break; // ANA B
    case 0xA1: i8080_ana(c, c->c); break; // ANA C
    case 0xA2: i8080_ana(c, c->d); break; // ANA D
    case 0xA3: i8080_ana(c, c->e); break; // ANA E
    case 0xA4: i8080_ana(c, c->h); break; // ANA H
    case 0xA5: i8080_ana(c, c->l); break; // ANA L
    case 0xA6: i8080_ana(c, i8080_rb(c, i8080_get_hl(c))); break; // ANA M
    case 0xE6: i8080_ana(c, i8080_next_byte(c)); break; // ANI byte

    case 0xAF: i8080_xra(c, c->a); break; // XRA A
    case 0xA8: i8080_xra(c, c->b); break; // XRA B
    case 0xA9: i8080_xra(c, c->c); break; // XRA C
    case 0xAA: i8080_xra(c, c->d); break; // XRA D
    case 0xAB: i8080_xra(c, c->e); break; // XRA E
    case 0xAC: i8080_xra(c, c->h); break; // XRA H
    case 0xAD: i8080_xra(c, c->l); break; // XRA L
    case 0xAE: i8080_xra(c, i8080_rb(c, i8080_get_hl(c))); break; // XRA M
    case 0xEE: i8080_xra(c, i8080_next_byte(c)); break; // XRI byte

    case 0xB7: i8080_ora(c, c->a); break; // ORA A
    case 0xB0: i8080_ora(c, c->b); break; // ORA B
    case 0xB1: i8080_ora(c, c->c); break; // ORA C
    case 0xB2: i8080_ora(c, c->d); break; // ORA D
    case 0xB3: i8080_ora(c, c->e); break; // ORA E
    case 0xB4: i8080_ora(c, c->h); break; // ORA H
    case 0xB5: i8080_ora(c, c->l); break; // ORA L
    case 0xB6: i8080_ora(c, i8080_rb(c, i8080_get_hl(c))); break; // ORA M
    case 0xF6: i8080_ora(c, i8080_next_byte(c)); break; // ORI byte

    case 0xBF: i8080_cmp(c, c->a); break; // CMP A
    case 0xB8: i8080_cmp(c, c->b); break; // CMP B
    case 0xB9: i8080_cmp(c, c->c); break; // CMP C
    case 0xBA: i8080_cmp(c, c->d); break; // CMP D
    case 0xBB: i8080_cmp(c, c->e); break; // CMP E
    case 0xBC: i8080_cmp(c, c->h); break; // CMP H
    case 0xBD: i8080_cmp(c, c->l); break; // CMP L
    case 0xBE: i8080_cmp(c, i8080_rb(c, i8080_get_hl(c))); break; // CMP M
    case 0xFE: i8080_cmp(c, i8080_next_byte(c)); break; // CPI byte

    // branch control/program counter load instructions
    case 0xC3: i8080_jmp(c, i8080_next_word(c)); break; // JMP
    case 0xC2: i8080_cond_jmp(c, c->z == 0); break; // JNZ
    case 0xCA: i8080_cond_jmp(c, c->z == 1); break; // JZ
    case 0xD2: i8080_cond_jmp(c, c->cy == 0); break; // JNC
    case 0xDA: i8080_cond_jmp(c, c->cy == 1); break; // JC
    case 0xE2: i8080_cond_jmp(c, c->p == 0); break; // JPO
    case 0xEA: i8080_cond_jmp(c, c->p == 1); break; // JPE
    case 0xF2: i8080_cond_jmp(c, c->s == 0); break; // JP
    case 0xFA: i8080_cond_jmp(c, c->s == 1); break; // JM

    case 0xE9: c->pc = i8080_get_hl(c); break; // PCHL
    case 0xCD: i8080_call(c, i8080_next_word(c)); break; // CALL

    case 0xC4: i8080_cond_call(c, c->z == 0); break; // CNZ
    case 0xCC: i8080_cond_call(c, c->z == 1); break; // CZ
    case 0xD4: i8080_cond_call(c, c->cy == 0); break; // CNC
    case 0xDC: i8080_cond_call(c, c->cy == 1); break; // CC
    case 0xE4: i8080_cond_call(c, c->p == 0); break; // CPO
    case 0xEC: i8080_cond_call(c, c->p == 1); break; // CPE
    case 0xF4: i8080_cond_call(c, c->s == 0); break; // CP
    case 0xFC: i8080_cond_call(c, c->s == 1); break; // CM

    case 0xC9: i8080_ret(c); break; // RET
    case 0xC0: i8080_cond_ret(c, c->z == 0); break; // RNZ
    case 0xC8: i8080_cond_ret(c, c->z == 1); break; // RZ
    case 0xD0: i8080_cond_ret(c, c->cy == 0); break; // RNC
    case 0xD8: i8080_cond_ret(c, c->cy == 1); break; // RC
    case 0xE0: i8080_cond_ret(c, c->p == 0); break; // RPO
    case 0xE8: i8080_cond_ret(c, c->p == 1); break; // RPE
    case 0xF0: i8080_cond_ret(c, c->s == 0); break; // RP
    case 0xF8: i8080_cond_ret(c, c->s == 1); break; // RM

    case 0xC7: i8080_call(c, 0x00); break; // RST 0
    case 0xCF: i8080_call(c, 0x08); break; // RST 1
    case 0xD7: i8080_call(c, 0x10); break; // RST 2
    case 0xDF: i8080_call(c, 0x18); break; // RST 3
    case 0xE7: i8080_call(c, 0x20); break; // RST 4
    case 0xEF: i8080_call(c, 0x28); break; // RST 5
    case 0xF7: i8080_call(c, 0x30); break; // RST 6
    case 0xFF: i8080_call(c, 0x38); break; // RST 7

    // stack operation instructions
    case 0xC5: i8080_push_stack(c, i8080_get_bc(c)); break; // PUSH B
    case 0xD5: i8080_push_stack(c, i8080_get_de(c)); break; // PUSH D
    case 0xE5: i8080_push_stack(c, i8080_get_hl(c)); break; // PUSH H
    case 0xF5: i8080_push_psw(c); break; // PUSH PSW
    case 0xC1: i8080_set_bc(c, i8080_pop_stack(c)); break; // POP B
    case 0xD1: i8080_set_de(c, i8080_pop_stack(c)); break; // POP D
    case 0xE1: i8080_set_hl(c, i8080_pop_stack(c)); break; // POP H
    case 0xF1: i8080_pop_psw(c); break; // POP PSW

    // input/output instructions
    case 0xDB: break; // IN
    case 0xD3: break; // OUT

    // undocumented nops
    case 0x08: break;
    case 0x10: break;
    case 0x18: break;
    case 0x20: break;
    case 0x28: break;
    case 0x30: break;
    case 0x38: break;

    // undocumented RET
    case 0xD9: i8080_ret(c); break;

    default:
        fprintf(stderr, "error: unknown opcode 0x%02X\n", opcode);
        break;
    }
}

// services an interrupt (a simple CALL) if the iff is true
void i8080_interrupt(i8080* const c, const u16 addr) {
    if (c->iff) {
        c->iff = 0;
        i8080_call(c, addr);
        c->cyc += 11;
    }
}

// outputs a debug trace of the emulator state to the standard output,
// including registers and flags
void i8080_debug_output(i8080* const c) {
    char flags[] = "......";

    if (c->z) flags[0] = 'z';
    if (c->s) flags[1] = 's';
    if (c->p) flags[2] = 'p';
    if (c->iff) flags[3] = 'i';
    if (c->cy) flags[4] = 'c';
    if (c->hc) flags[5] = 'a';

    // registers + flags
    printf("af\tbc\tde\thl\tpc\tsp\tflags\tcycles\n");
    printf("%02X__\t%04X\t%04X\t%04X\t%04X\t%04X\t%s\t%i\n",
           c->a, i8080_get_bc(c), i8080_get_de(c), i8080_get_hl(c), c->pc,
           c->sp, flags, c->cyc);

    // current address in memory
    printf("%04X: ", c->pc);

    // current opcode + next two
    printf("%02X %02X %02X", i8080_rb(c, c->pc), i8080_rb(c, c->pc + 1),
           i8080_rb(c, c->pc + 2));

    // disassembly of the current opcode
    printf(" - %s", DISASSEMBLE_TABLE[i8080_rb(c, c->pc)]);

    printf("\n================================");
    printf("==============================\n");
}

// runs the emulator in test mode, allowing it to output characters
// to the standard output to see the results of the test
// warning: before calling this function, you must have initialized the struct
// beforehand and loaded a rom at 0x100
void i8080_run_testrom(i8080* const c) {
    c->pc = 0x100; // the test roms all start at 0x100
    i8080_wb(c, 5, 0xC9); // inject RET at 0x5 to handle "CALL 5", needed
                          // for the test roms

    printf("*******************\n");
    while (1) {
        const u16 cur_pc = c->pc;

        if (i8080_rb(c, c->pc) == 0x76) { // RET
            printf("HLT at %04X\n", c->pc);
        }

        if (c->pc == 5) {
            // prints characters stored in memory at (DE)
            // until character '$' (0x24 in ASCII) is found
            if (c->c == 9) {
                u16 i = i8080_get_de(c);
                do {
                    printf("%c", i8080_rb(c, i));
                    i += 1;
                } while (i8080_rb(c, i) != 0x24);
            }
            // prints a single character stored in register E
            if (c->c == 2) {
                printf("%c", c->e);
            }
        }

        // uncomment following line to have a debug output of machine state
        // warning: will output multiple GB of data for the whole test suite
        // i8080_debug_output(c);

        i8080_step(c);

        if (c->pc == 0) {
            printf("\nJumped to 0x0000 from 0x%04X\n\n", cur_pc);
            break;
        }
    }
}

// memory helpers (the only four to use `read_byte` and `write_byte` function
// pointers)

// reads a byte from memory
u8 i8080_rb(i8080* const c, const u16 addr) {
    return c->read_byte(c->userdata, addr);
}

// writes a byte to memory
void i8080_wb(i8080* const c, const u16 addr, const u8 val) {
    c->write_byte(c->userdata, addr, val);
}

// reads a word from memory
u16 i8080_rw(i8080* const c, const u16 addr) {
    return c->read_byte(c->userdata, addr + 1) << 8 |
           c->read_byte(c->userdata, addr);
}

// writes a word to memory
void i8080_ww(i8080* const c, const u16 addr, const u16 val) {
    c->write_byte(c->userdata, addr, val & 0xFF);
    c->write_byte(c->userdata, addr + 1, val >> 8);
}

// returns the next byte in memory (and updates the program counter)
u8 i8080_next_byte(i8080* const c) {
    return i8080_rb(c, c->pc++);
}

// returns the next word in memory (and updates the program counter)
u16 i8080_next_word(i8080* const c) {
    const u16 result = i8080_rw(c, c->pc);
    c->pc += 2;
    return result;
}

// paired registers helpers (setters and getters)
void i8080_set_bc(i8080* const c, const u16 val) {
    c->b = val >> 8;
    c->c = val & 0xFF;
}

void i8080_set_de(i8080* const c, const u16 val) {
    c->d = val >> 8;
    c->e = val & 0xFF;
}

void i8080_set_hl(i8080* const c, const u16 val) {
    c->h = val >> 8;
    c->l = val & 0xFF;
}

u16 i8080_get_bc(i8080* const c) {
    return (c->b << 8) | c->c;
}

u16 i8080_get_de(i8080* const c) {
    return (c->d << 8) | c->e;
}

u16 i8080_get_hl(i8080* const c) {
    return (c->h << 8) | c->l;
}

// stack helpers

// pushes a value into the stack and updates the stack pointer
void i8080_push_stack(i8080* const c, const u16 val) {
    c->sp = (c->sp - 2) & 0xFFFF;
    i8080_ww(c, c->sp, val);
}

// pops a value from the stack and updates the stack pointer
u16 i8080_pop_stack(i8080* const c) {
    const u16 val = i8080_rw(c, c->sp);
    c->sp = (c->sp + 2) & 0xFFFF;
    return val;
}

// opcodes
bool parity(const u8 val) {
    // checks the parity of a byte. returns:
    // - false if number of 1 bits in `val` is odd,
    // - true if number of 1 bits in `val` is even
    u8 nb_one_bits = 0;
    for (int i = 0; i < 8; i++) {
        nb_one_bits += ((val >> i) & 1);
    }

    return (nb_one_bits & 1) == 0;
}

// adds a value (+ an optional bool) to a register
void i8080_add(i8080* const c, u8* const reg, const u8 val, const bool cy) {
    const s16 result = *reg + val + cy;
    c->z = (result & 0xFF) == 0;
    c->s = (result & 0b10000000) != 0;
    c->cy = (result & 0b100000000) != 0;
    c->hc = (*reg ^ result ^ val) & 0x10;
    c->p = parity(result & 0xFF);
    *reg = result & 0xFF;
}

// substracts a byte (+ an optional bool) from a register
void i8080_sub(i8080* const c, u8* const reg, const u8 val, const bool cy) {
    const s16 result = *reg - val - cy;
    c->z = (result & 0xFF) == 0;
    c->s = (result & 0b10000000) != 0;
    c->cy = (result & 0b100000000) != 0;
    c->hc = ~(*reg ^ result ^ val) & 0x10;
    c->p = parity(result & 0xFF);
    *reg = result & 0xFF;
}

// adds a word to HL
void i8080_dad(i8080* const c, const u16 val) {
    const u32 result = i8080_get_hl(c) + val;
    i8080_set_hl(c, result & 0xFFFF);
    c->cy = (result & 0x10000) != 0;
}

// increments a byte
u8 i8080_inr(i8080* const c, const u8 val) {
    const u8 result = val + 1;
    c->hc = (result & 0x0F) == 0;
    c->z = (result) == 0;
    c->s = (result & 0b10000000) != 0;
    c->p = parity(result);
    return result;
}

// decrements a byte
u8 i8080_dcr(i8080* const c, const u8 val) {
    const u8 result = val - 1;
    c->hc = !((result & 0x0F) == 0x0F);
    c->z = result == 0;
    c->s = (result & 0b10000000) != 0;
    c->p = parity(result);
    return result;
}

// executes a logic "and" between register A and a byte, then stores the
// result in register A
void i8080_ana(i8080* const c, const u8 val) {
    const u8 result = c->a & val;
    c->cy = 0;
    c->hc = ((c->a | val) & 0x08) != 0;
    c->z = result == 0;
    c->s = (result & 0x80) != 0;
    c->p = parity(result);
    c->a = result;
}

// executes a logic "xor" between register A and a byte, then stores the
// result in register A
void i8080_xra(i8080* const c, const u8 val) {
    c->a ^= val;
    c->cy = 0;
    c->hc = 0;
    c->z = c->a == 0;
    c->s = (c->a & 0x80) != 0;
    c->p = parity(c->a);
}

// executes a logic "or" between register A and a byte, then stores the
// result in register A
void i8080_ora(i8080* const c, const u8 val) {
    c->a |= val;
    c->cy = 0;
    c->hc = 0;
    c->z = c->a == 0;
    c->s = (c->a & 0x80) != 0;
    c->p = parity(c->a);
}

// compares the register A to another byte
void i8080_cmp(i8080* const c, const u8 val) {
    const s16 result = c->a - val;
    c->cy = (result & 0b100000000) != 0;
    c->hc = ~(c->a ^ result ^ val) & 0x10;
    c->z = (result & 0xFF) == 0;
    c->s = (result & 0x80) != 0;
    c->p = parity(result);
}

// sets the program counter to a given address
void i8080_jmp(i8080* const c, const u16 addr) {
    c->pc = addr;
}

// jumps to next address pointed by the next word in memory if a condition
// is met
void i8080_cond_jmp(i8080* const c, const bool condition) {
    const u16 addr = i8080_next_word(c);
    if (condition) {
        c->pc = addr;
    }
}

// pushes the current pc to the stack, then jumps to an address
void i8080_call(i8080* const c, const u16 addr) {
    i8080_push_stack(c, c->pc);
    i8080_jmp(c, addr);
}

// calls to next word in memory if a condition is met
void i8080_cond_call(i8080* const c, const bool condition) {
    const u16 addr = i8080_next_word(c);
    if (condition) {
        i8080_call(c, addr);
        c->cyc += 6;
    }
}

// returns from subroutine
void i8080_ret(i8080* const c) {
    c->pc = i8080_pop_stack(c);
}

// returns from subroutine if a condition is met
void i8080_cond_ret(i8080* const c, const bool condition) {
    if (condition) {
        i8080_ret(c);
        c->cyc += 6;
    }
}

// pushes register A and the flags into the stack
void i8080_push_psw(i8080* const c) {
    u8 psw = 0;
    if (c->s) psw |= 0b10000000;
    if (c->z) psw |= 0b01000000;
    if (c->hc) psw |= 0b00010000;
    if (c->p) psw |= 0b00000100;
    if (c->cy) psw |= 0b00000001;

    psw |= 0b00000010; // bit 1 is always 1
    psw &= ~0b00001000; // bit 3 is always 0
    psw &= ~0b00100000; // bit 5 is always 0

    i8080_push_stack(c, c->a << 8 | psw);
}

// pops register A and the flags from the stack
void i8080_pop_psw(i8080* const c) {
    const u16 af = i8080_pop_stack(c);
    c->a = af >> 8;
    u8 psw = af & 0xFF;

    c->s = psw & 0b10000000 ? 1 : 0;
    c->z = psw & 0b01000000 ? 1 : 0;
    c->hc = psw & 0b00010000 ? 1 : 0;
    c->p = psw & 0b00000100 ? 1 : 0;
    c->cy = psw & 0b00000001 ? 1 : 0;
}

// rotate register A left
void i8080_rlc(i8080* const c) {
    c->cy = c->a >> 7;
    c->a = (c->a << 1) | c->cy;
}

// rotate register A right
void i8080_rrc(i8080* const c) {
    c->cy = c->a & 1;
    c->a = (c->a >> 1) | (c->cy << 7);
}

// rotate register A left with the carry flag
void i8080_ral(i8080* const c) {
    const bool cy = c->cy;
    c->cy = c->a >> 7;
    c->a = (c->a << 1) | cy;
}

// rotate register A right with the carry flag
void i8080_rar(i8080* const c) {
    const bool cy = c->cy;
    c->cy = c->a & 1;
    c->a = (c->a >> 1) | (cy << 7);
}

// adjusts register A to form two 4bit binary coded decimal digits.
// example: we want to add 93 and 8 (decimal operation):
//     MOV A, 0x93
//     ADI 0x08
//     ; now, A = 0x9B (0b10011011)
//     DAA
//     ; now, A = 0x01 (because 93 + 8 = 101)
//     ; and carry flag is set
void i8080_daa(i8080* const c) {
    bool cy = c->cy;
    u8 value_to_add = 0;

    const u8 lsb = c->a & 0x0F;
    const u8 msb = c->a >> 4;

    if (c->hc || lsb > 9) {
        value_to_add += 0x06;
    }
    if (c->cy || msb > 9 || (msb >= 9 && lsb > 9)) {
        value_to_add += 0x60;
        cy = 1;
    }
    i8080_add(c, &c->a, value_to_add, 0);
    c->p = parity(c->a);
    c->cy = cy;
}

// switches the value of registers DE and HL
void i8080_xchg(i8080* const c) {
    const u16 de = i8080_get_de(c);
    i8080_set_de(c, i8080_get_hl(c));
    i8080_set_hl(c, de);
}

// switches the value of a word at (sp) and HL
void i8080_xthl(i8080* const c) {
    const u16 val = i8080_rw(c, c->sp);
    i8080_ww(c, c->sp, i8080_get_hl(c));
    i8080_set_hl(c, val);
}
