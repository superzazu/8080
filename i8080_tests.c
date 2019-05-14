// This file uses the 8080 emulator to run the test suite (roms in cpu_tests
// directory). It uses a simple array as memory.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "i8080.h"

// memory callbacks
static u8* memory;

static u8 rb(void* userdata, const u16 addr) {
    (void) userdata;
    return memory[addr];
}

static void wb(void* userdata, const u16 addr, const u8 val) {
    (void) userdata;
    memory[addr] = val;
}

static int load_file_into_memory(const char* filename, const u16 addr) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: can't open file '%s'\n", filename);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    rewind(f);

    if (file_size > 0x10000) {
        fprintf(stderr, "error: file '%s' too big to fit in memory (%ld)\n",
                filename, file_size);
        return 1;
    }

    size_t result = fread(&memory[addr], sizeof(u8), file_size, f);
    if (result != file_size) {
        fprintf(stderr, "error: while reading file '%s'\n", filename);
        return 1;
    }

    fclose(f);
    return 0;
}

int main(void) {
    memory = malloc(0x10000 * sizeof(u8));
    if (memory == NULL) {
        return 1;
    }

    i8080 cpu;
    i8080_init(&cpu);
    cpu.read_byte = rb;
    cpu.write_byte = wb;

    time_t start = 0, end = 0;
    time(&start);

    i8080_reset(&cpu);
    memset(memory, 0, 0x10000);
    load_file_into_memory("cpu_tests/TST8080.COM", 0x100);
    i8080_run_testrom(&cpu);

    i8080_reset(&cpu);
    memset(memory, 0, 0x10000);
    load_file_into_memory("cpu_tests/CPUTEST.COM", 0x100);
    i8080_run_testrom(&cpu);

    i8080_reset(&cpu);
    memset(memory, 0, 0x10000);
    load_file_into_memory("cpu_tests/8080PRE.COM", 0x100);
    i8080_run_testrom(&cpu);

    i8080_reset(&cpu);
    memset(memory, 0, 0x10000);
    load_file_into_memory("cpu_tests/8080EXM.COM", 0x100);
    i8080_run_testrom(&cpu);

    time(&end);
    double elapsed = difftime(end, start);
    printf("Executed in %.2lfs.", elapsed);

    free(memory);

    return 0;
}
