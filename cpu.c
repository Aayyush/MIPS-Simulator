/**
 * Gedare Bloom
 * Bijesh Subedi
 * Kishor Subedi
 * Aayush Gupta
 * Suraj Upreti
 * cpu.c
 *
 * Implementation of simulated processor.
 */

#include "cpu.h"
#include "memory.h"
#include "syscall.h"
#include <string.h>
#include <stdio.h>

struct cpu_context cpu_ctx;

int fetch( struct IF_ID_buffer *out )
{
	// Finding the new PC and retrieving the instruction
	out->instruction = instruction_memory[(cpu_ctx.PC - 0x400000)/4];
	out->next_pc = cpu_ctx.PC + 0x4;
    
	return 0;
}

int decode( struct IF_ID_buffer *in, struct ID_EX_buffer *out )
{
    // Initializing by finding the opcode and funct
	uint8_t opcode = (in->instruction)>>26;
	uint8_t funct = ((in->instruction)<<26)>>26;
    
    // Figuring out addresses and data
    out->is_syscall = false;
    out->read_address1 = shift_and_find(in->instruction, 6, 27);
    out->read_address2 = shift_and_find(in->instruction, 11, 27);
    out->write_register = shift_and_find(in->instruction, 16, 27);
    out->shamt = shift_and_find(in->instruction, 21, 27);
    out->jump_address = shift_and_find(in->instruction, 6, 6);
    out->sign_extended_immediate = shift_and_find(in->instruction, 16, 16);
    out->read_data2 = cpu_ctx.GPR[out->read_address2];
    out->read_data1 = cpu_ctx.GPR[out->read_address1];
	out->next_pc = in->next_pc;
    out->jump_ctrl = NONE;
    
    // Decode the opcode to find the control signals
    decode_opcode(opcode, funct, in, out);
    
    // Finding the source for the destination register
	if (!out->reg_dst) out->write_register = out->read_address2;
    
	return 0;
}

int execute( struct ID_EX_buffer *in, struct EX_MEM_buffer *out )
{
    cpu_ctx.PC = in->next_pc;
    
    // Handling the syscall
	if (in->is_syscall) {
		// Copy the content of the v0 register to determine the type of operation.
		syscall(cpu_ctx.GPR[2], cpu_ctx.GPR[4]);
		return 0;
	}
    // The operation in ALU after selecting the ALUSRC
    out->alu_result = alu_operation(in);
	// printf("Check alu result: %u \n", out->alu_result);
	out->alu_zero = (out->alu_result == 0) ? true: false;
    
    // Calculate the result for the branch instruction
    int32_t left_shifted_imm = in->sign_extended_immediate << 2;
    out->branch_target = left_shifted_imm + in->next_pc;
    
    // Carrying on other control signals
    out->branch = in->branch;
    out->branch_ne = in->branch_ne;
    out->mem_read = in->mem_read;
    out->mem_write = in->mem_write;
    out->reg_write = in->reg_write;
    out->mem_to_reg = in->mem_to_reg;
    out->read_data2 = in->read_data2;
    out->write_register = in->write_register;
    out->jump_ctrl = in->jump_ctrl;
    out->jump_address = in->jump_address << 2;
    
    // Indicating successful completion of the function
	return 0;
}

int memory( struct EX_MEM_buffer *in, struct MEM_WB_buffer *out )
{
    // Checking the result of the branch statement
    if (in->branch && in->alu_zero)
        cpu_ctx.PC = in->branch_target;
    else if (in->branch_ne && !in->alu_zero)
        cpu_ctx.PC = in->branch_target;
    
    // Check if we have to read or write to the memory
    int32_t read_data_memory = 0;
    if (in->mem_read)
        read_data_memory = data_memory[(in->alu_result - 0x10000000) / 4];

    if (in->mem_write)
        data_memory[(in->alu_result - 0x10000000) / 4] = in->read_data2;
    
    // Check what value to send to the write register
    out->write_register = in->write_register;
    out->write_data = (in->mem_to_reg)? read_data_memory : in->alu_result;
    
    // Passing the control signals
    out->reg_write = in->reg_write;
    
    // Check if the instruction is a jump instruction
    check_jump(in, out);
    
	return 0;
}

int writeback( struct MEM_WB_buffer *in )
{
    // Writing the data in the given register
    if (in->reg_write)
        cpu_ctx.GPR[in->write_register] = in->write_data;
    
	return 0;
}

int32_t shift_and_find(uint32_t instruction, uint8_t left, uint8_t right){
    return (instruction << left) >> right;
}

void set_control_signals(char arr[], struct ID_EX_buffer *out){
	out->branch = convert_to_bool(arr[0]);
	out->mem_read = convert_to_bool(arr[1]);
	out->mem_to_reg = convert_to_bool(arr[2]);
	out->mem_write = convert_to_bool(arr[3]);
	out->reg_write = convert_to_bool(arr[4]);
	out->alu_src = convert_to_bool(arr[5]);
    out->branch_ne = convert_to_bool(arr[6]);
	out->reg_dst = convert_to_bool(arr[7]);
}

bool convert_to_bool(char a) {
	return a != '0';
}

int32_t alu_operation(struct ID_EX_buffer *in){
    int32_t src1 = in->read_data1;
	int32_t src2 = (in->alu_src)?in->sign_extended_immediate:in->read_data2;
	uint32_t u_src2 = src2;
	switch(in->alu_control){
        case ADD:
            return (src1 + src2);
        case SUB:
            return src1 - src2;
        case AND:
            return src1 & src2;
        case OR:
            return src1 | src2;
        case NOR:
            return ~(src1 | src2);
        case XOR:
            return src1 ^ src2;
        case LUI:
            return src2 << 16;
		case SLL:
			return src2 << (in->shamt);
		case SLT:
			return (src1 < src2) ? 1 : 0;
		case SRL:
			return u_src2 >> (in->shamt);
		case SRA:
			return  src2 >> (in->shamt);
        default:
            return 0;
    }
}

void check_jump(struct EX_MEM_buffer *in, struct MEM_WB_buffer *out){
    switch(in->jump_ctrl){
        case NONE:
            return;
            
        case JUMP:
            cpu_ctx.PC = in->jump_address;
            return;
            
        case JAL:
            out->write_data = cpu_ctx.PC;
            cpu_ctx.PC = in->jump_address;
            out->write_register = 31;
            out->reg_write = true;
            return;
            
        case JR:
            cpu_ctx.PC = in->alu_result;
            return;
    }
}

int decode_rformat(uint8_t funct, char *control_signal, struct ID_EX_buffer *out){
    switch (funct)
    {
        case 8: //jr
            out->jump_ctrl = JR;
            strcpy(control_signal, "00000000");
            out->alu_control = ADD;
        case 32:
            out->alu_control = ADD;
            break;
        case 33:
            out->alu_control = ADD;
            break;
        case 36:
            out->alu_control = AND;
            break;
        case 37:
            out->alu_control = OR;
            break;
        case 0:
            out->alu_control = SLL;
            break;
        case 42:
            out->alu_control = SLT;
            break;
        case 2:
            out->alu_control = SRL;
            break;
        case 39:
            out->alu_control = NOR;
            break;
        case 3:
            out->alu_control = SRA;
            break;
        case 38:
            out->alu_control = XOR;
            break;
        case 40:
            out->alu_control = SUB;
            break;
    }
    return 0;
}

int decode_opcode(uint8_t opcode, uint8_t funct, struct IF_ID_buffer *in, struct ID_EX_buffer *out){
    char control_signal[9];
    switch(opcode){
        case 0:
            strcpy(control_signal, "00001001");
            if(funct == 12){ // Syscall
                out->is_syscall = true;
                break;
            }
            decode_rformat(funct, control_signal, out);
            break;
        case 2: // Jump instruction
            out->jump_ctrl = JUMP;
            strcpy(control_signal, "00000000");
            break;
        case 3: // JAL instruction
            out->jump_ctrl = JAL;
            strcpy(control_signal, "00000000");
        case 8: // Add immediate.
            out->alu_control = ADD;
            strcpy(control_signal, "00001100");
            break;
        case 12: // And immediate.
            out->alu_control = AND;
            strcpy(control_signal, "00001100");
            break;
        case 4: // Branch if equal.
            out->alu_control = SUB;
            strcpy(control_signal, "10000000");
            break;
        case 5: // Branch if not equal.
            out->alu_control = SUB;
            strcpy(control_signal, "10000010");
            break;
        case 13: // Or immediate.
            out->alu_control = OR;
            strcpy(control_signal, "00001100");
            break;
        case 10: // Set if less than immediate.
            out->alu_control = SLT;
            strcpy(control_signal, "00001100");
            break;
        case 14: // XOR immediate.
            out->alu_control = XOR;
            strcpy(control_signal, "00001100");
            break;
        case 35: // Load word.
            out->alu_control = SUB;
            strcpy(control_signal, "01101100");
            break;
        case 43: // Store Word.
            out->alu_control = ADD;
            strcpy(control_signal, "00010100");
            break;
        case 15: // Load upper immediate.
            out->alu_control = LUI;
            strcpy(control_signal, "00001100");
            break;
    }
    set_control_signals(control_signal, out);
    return 0;
}
