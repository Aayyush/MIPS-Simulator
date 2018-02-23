#include "cpu.h"
#include "memory.h"
#include "syscall.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void test_fetch(struct IF_ID_buffer *out){
    printf("\n\nAfter fetch\n");
    printf("Instruction(dec): %d\n", out->instruction);
    printf("Instruction(hex): %x\n", out->instruction);
    printf("Next PC: %x\n", out->next_pc);
}

void test_decode( struct ID_EX_buffer *out ){
    printf("\n\nAfter Decode\n");
    printf("Read Address 1: %d\n", out->read_address1 );
    printf("Read Address 2: %d\n", out->read_address2 );
    printf("Read Data 1: %d\n", out->read_data1 );
    printf("Read Data 2: %d\n", out->read_data2 );
    printf("Desti. Address: %d\n", out->write_register );
    printf("Singed Extended %d\n", out->sign_extended_immediate);
    printf("RegDst        : %d\n", out->reg_dst );
    printf("Branch        : %d\n", out->branch );
    printf("MemRead       : %d\n", out->mem_read );
    printf("MemWrite      : %d\n", out->mem_write );
    printf("RegWrite      : %d\n", out->reg_write );
    printf("MemToReg      : %d\n", out->mem_to_reg );
    printf("AluSrc        : %d\n", out->alu_src );
    printf("ALUControl    : %d\n", out->alu_control );
	printf("Shamt         : %d\n", out->shamt);
}

void test_execute( struct EX_MEM_buffer *out ){
    printf("\n\nAfter Execute\n");
    printf("Branch Targer:%x\n", out->branch_target);
    printf("Alu Result   :%d\n", out->alu_result);
    printf("Read Data2   :%d\n", out->read_data2);
    printf("Write Registe:%d\n", out->write_register);
    printf("Branch       :%d\n", out->branch);
    printf("Mem Read     :%d\n", out->mem_read);
    printf("Mem Write    :%d\n", out->mem_write);
    printf("Reg Write    :%d\n", out->reg_write);
    printf("Mem to Reg   :%d\n", out->mem_to_reg);
    printf("Jump Addr.   :%x\n", out->jump_address);
}

void test_memory(struct MEM_WB_buffer *out){
    printf("\n\nAfter Memory\n");
	printf("Next PC: %u \n", cpu_ctx.PC);
    printf("Write Registe:%d\n", out->write_register);
    printf("Write Data   :%d\n", out->write_data);
    printf("RegWrite     :%d\n", out->reg_write);
}

void print_registers(){
    printf("\n\nREGISTERS\n\n");
    for (int i = 0; i < 32; i++){
        printf("%d:\t%d\n", i, cpu_ctx.GPR[i]);
    }
    return;
}

int main(){
    // Initializing buffers
    struct IF_ID_buffer if_id;
	struct ID_EX_buffer id_ex;
	struct EX_MEM_buffer ex_mem;
	struct MEM_WB_buffer mem_wb;
	int i ;

	/* Initialize registers and memory to 0 */
	cpu_ctx.PC = 0x00400000;
    cpu_ctx.GPR[29] = 0x10000800;
    
	for ( i = 0; i < 32; i++ ) {
		cpu_ctx.GPR[i] = 0;
	}
	for ( i = 0; i < 2048; i++ ) {
		instruction_memory[i/2] = 0;
		data_memory[i] = 0;
	}
    
    cpu_ctx.GPR[8] = -4;
    
    uint32_t instructions[] = {
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x0c100009,
        0x00000000,
        0x3402000a,
        0x0000000c,
        0x34090001,
        0x340a0004,
        0x3c011001,
        0xac290000,
        0x3c011001,
        0x002a0821,
        0xac290000,
        0x34080008,
        0x210bfff8,
        0x210cfffc,
        0x3c011001,
        0x002b0821,
        0x8c290000,
        0x3c011001,
        0x002c0821,
        0x8c2a0000,
        0x012a6820,
        0x3c011001,
        0x00280821,
        0xac2d0000,
        0x21080004,
        0x290100a0,
        0x1420fff2,
        0x21050000,
        0x34020001,
        0x0000000c,
        0x03e00008
    };

    for (int i = 0; i < 36; i++){
        instruction_memory[i] = instructions[i];
    }
    
	for (int i = 0; i < 36; i++) {
        printf("__________________________________________\n");
		fetch(&if_id);
        printf("Instruction %d -> %x\n", i+1, if_id.instruction);
		test_fetch(&if_id);
		decode(&if_id, &id_ex);
		test_decode(&id_ex);
		execute(&id_ex, &ex_mem);
		test_execute(&ex_mem);
		memory(&ex_mem, &mem_wb);
		test_memory(&mem_wb);
        writeback(&mem_wb);
        //Testing the PC
        printf("\nGPR[%d]: %d\n", mem_wb.write_register, cpu_ctx.GPR[mem_wb.write_register]);
        printf("\nPC:%x\n\n", cpu_ctx.PC);
	}
    
    //print_registers();

    return 0;
}
