// This file uses the 8080 emulator to run the test suite (roms in cpu_tests
// directory). It uses a simple array as memory.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "i8080.h"

// memory callbacks
#define MEMORY_SIZE 0x10000
static u8* memory;

static u8 rb(void* userdata, const u16 addr) {
    (void) userdata;
    return memory[addr];
}

static void wb(void* userdata, const u16 addr, const u8 val) {
    (void) userdata;
    memory[addr] = val;
}

static int load_file(const char* filename, u16 addr) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: can't open file '%s'.\n", filename);
        return 1;
    }

    // file size check:
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    rewind(f);

    if (file_size + addr >= MEMORY_SIZE) {
        fprintf(stderr, "error: file %s can't fit in memory.\n", filename);
        return 1;
    }

    // copying the bytes in memory:
    size_t result = fread(&memory[addr], sizeof(u8), file_size, f);
    if (result != file_size) {
        fprintf(stderr, "error: while reading file '%s'\n", filename);
        return 1;
    }

    fclose(f);
    return 0;
}

// runs a program, handling CALL 5 call to output test results to standard
// output
static void run_test(i8080* const c, const char* filename) {
    i8080_init(c);
    c->read_byte = rb;
    c->write_byte = wb;
    memset(memory, 0, MEMORY_SIZE);

    if (load_file(filename, 0x100) != 0) {
        return;
    }
    printf("*** TEST: %s\n", filename);

    c->pc = 0x100;
    wb(c, 5, 0xC9); // inject RET at 0x5 to handle "CALL 5"

    while (1) {
        u16 cur_pc = c->pc;

        if (c->pc == 5) {
            // prints characters stored in memory at (DE) until '$' is found
            if (c->c == 9) {
                u16 i = (c->d << 8) | c->e;
                do {
                    printf("%c", rb(c, i));
                    i += 1;
                } while (rb(c, i) != '$');
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
            printf("\nJumped to 0x0000 from 0x%04X (%d cycles)\n\n",
                cur_pc, c->cyc);
            break;
        }
    }
}

int main(void) {
    memory = malloc(MEMORY_SIZE * sizeof(u8));
    if (memory == NULL) {
        return 1;
    }

    i8080 cpu;
    run_test(&cpu, "cpu_tests/TST8080.COM");
    run_test(&cpu, "cpu_tests/CPUTEST.COM");
    run_test(&cpu, "cpu_tests/8080PRE.COM");
    run_test(&cpu, "cpu_tests/8080EXM.COM");

    free(memory);

    return 0;
}
