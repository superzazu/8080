// This file uses the 8080 emulator to run the test suite (roms in cpu_tests
// directory). It uses a simple array as memory.

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "i8080.h"

// memory callbacks
static u8 memory[0x10000] = {0};

static u8 rb(const u16 addr) {
    return memory[addr];
}

static void wb(const u16 addr, const u8 val) {
    memory[addr] = val;
}

static int load_file_into_memory(const char* filename, const u16 addr) {
    FILE *f;
    long file_size = 0;

    f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: can't open file '%s'\n", filename);
        return 1;
    }

    // obtain file size
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    rewind(f);

    if (file_size > 0x10000) {
        fprintf(stderr, "error: file '%s' too big to fit in memory (%ld)\n",
                filename, file_size);
        return 1;
    }

    // copy file data to buffer
    u8 buffer[file_size];
    size_t result = fread(buffer, 1, file_size, f);
    if (result != file_size) {
        fprintf(stderr, "error: while reading file '%s'\n", filename);
        return 1;
    }

    // copy buffer to memory
    for (int i = 0; i < file_size; i++) {
        memory[addr + i] = buffer[i];
    }

    return 0;
}

int main(int argc, char **argv) {
    i8080 cpu;
    cpu.read_byte = rb;
    cpu.write_byte = wb;

    time_t start, end;
    time(&start);

    i8080_init(&cpu);
    memset(memory, 0, sizeof memory);
    load_file_into_memory("cpu_tests/TST8080.COM", 0x100);
    i8080_run_testrom(&cpu);

    i8080_init(&cpu);
    memset(memory, 0, sizeof memory);
    load_file_into_memory("cpu_tests/CPUTEST.COM", 0x100);
    i8080_run_testrom(&cpu);

    i8080_init(&cpu);
    memset(memory, 0, sizeof memory);
    load_file_into_memory("cpu_tests/8080PRE.COM", 0x100);
    i8080_run_testrom(&cpu);

    i8080_init(&cpu);
    memset(memory, 0, sizeof memory);
    load_file_into_memory("cpu_tests/8080EXM.COM", 0x100);
    i8080_run_testrom(&cpu);

    time(&end);
    double elapsed = difftime(end, start);
    printf("Executed in %.2lfs.", elapsed);

    return 0;
}
