#include "cpu.h"
#include "memory.h"
#include "syscall.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define DEBUG

int main( int argc, char *argv[] )
{
    FILE *f;
    int i;
    
    /* Initialize registers and memory to 0 */
    cpu_ctx.PC = 0x00400000;
    cpu_ctx.GPR[29] = 0x10000200;
    
    for ( i = 0; i < 32; i++ ) {
        cpu_ctx.GPR[i] = 0;
    }
    
    for ( i = 0; i < 2048; i++ ) {
        instruction_memory[i/2] = 0;
        data_memory[i] = 0;
    }


    // Printing the data memory with the index.
    
    /* Read memory from the input file */
    f = fopen(argv[1], "r");
    assert (f);
    for ( i = 0; i < 1024; i++ ) {
        fread(&data_memory[i], sizeof(uint32_t), 1, f);
#if defined(DEBUG)
        printf("%u\n", data_memory[i]);
#endif
    }
    for ( i = 0; i < 1024; i++ ) {
        fread(&instruction_memory[i], sizeof(uint32_t), 1, f);
#if defined(DEBUG)
        printf("%u\n", instruction_memory[i]);
#endif
    }
    // Adding syscalls to the instruction memory.
    // instruction_memory[1022] = 0x2002000A;
    // instruction_memory[1023] = 0xC;
    
    for (int a = 0x400000, i = 0; i < 1024; i++, a+=4){
        printf("Instruction at address %x : %x \n", a, instruction_memory[i]);
    }
    fclose(f);
    
    while(1) {
#if defined(DEBUG)
        printf("FETCH from PC=%x\n", cpu_ctx.PC);
#endif

        fetch();
        decode();
        execute();
        memory();
        writeback();
       //  if (++a >= 100) break;
    }
    return 0;
}
