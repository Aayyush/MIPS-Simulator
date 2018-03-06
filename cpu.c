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
#include <stdbool.h>

#define ENABLE_FORWARDING

bool pc_write = true;
bool if_id_write = true;
bool squash = false;


int stalls = 0;
int squashs = 0;

struct cpu_context cpu_ctx;
struct IF_ID_buffer if_id;
struct ID_EX_buffer id_ex;
struct EX_MEM_buffer ex_mem;
struct MEM_WB_buffer mem_wb;

int fetch()
{
	// Finding the new PC and retrieving the instruction
	add_instruction(instruction_memory[(cpu_ctx.PC - 0x400000)/4]);
	if_id.next_pc = cpu_ctx.PC + 0x4;
	update_pc(if_id.next_pc);
	return 0;
}

int decode()
{
    /*
     1. Pass the instruction from the IF/ID buffer to the ID/EX buffer.
     2. Decode Instruction and write to the ID/EX buffer.
     3. Decode Opcode and Set Control Signals. < Bring the set Control signal to the main function.
     4. Choose the destination register by using the RegDst multiplexer.
     */
    
    // TO DO: Don't write to the buffer directly. Instead use a temporary struct to pass data around and write to the buffer at the end of the stage.
    id_ex.instruction = if_id.instruction;
    decode_instructions(); // Reads the instruction and writes to the target buffer.
    
    // Initializing by finding the opcode and funct
    // Decode the opcode to find the control signals
    uint8_t opcode = shift_and_find(if_id.instruction, 0, 26);
    uint8_t funct = shift_and_find(if_id.instruction, 26, 26);
    decode_opcode(opcode, funct);
    
    // Finding the source for the destination register.
    // Replacing the read address 2 from buffer with the actual write register.
    
    // << Place hazard Unit before writing to the buffer. >>
    register_destination_multiplexer();
    hazard_detection();
    
    return 0;
}


int execute()
{
    ex_mem.instruction = id_ex.instruction;
    // Handling the syscall
    ex_mem.is_syscall = id_ex.is_syscall;
    // The operation in ALU after selecting the ALUSRC
    ex_mem.alu_result = alu_operation();
    // printf("Check alu result: %u \n", ex_mem.alu_result);
    ex_mem.alu_zero = (ex_mem.alu_result == 0) ? true: false;
    
    // Calculate the result for the branch instruction
    int32_t left_shifted_imm = id_ex.sign_extended_immediate << 2;
    ex_mem.branch_target = left_shifted_imm + id_ex.next_pc;
    
    // Carrying on other control signals
    ex_mem.branch = id_ex.branch;
    ex_mem.branch_ne = id_ex.branch_ne;
    ex_mem.mem_read = id_ex.mem_read;
    ex_mem.mem_write = id_ex.mem_write;
    ex_mem.reg_write = id_ex.reg_write;
    ex_mem.mem_to_reg = id_ex.mem_to_reg;
    ex_mem.read_data2 = id_ex.read_data2;
    ex_mem.write_register = id_ex.write_register;
    ex_mem.jump_ctrl = id_ex.jump_ctrl;
    ex_mem.jump_address = id_ex.jump_address << 2;
    ex_mem.next_pc = id_ex.next_pc;
    
    // Indicating successful completion of the function
    return 0;
}


int memory()
{
    mem_wb.is_syscall = ex_mem.is_syscall;
    mem_wb.instruction = ex_mem.instruction;
    // Checking the result of the branch statement
    if(branch_taken())
        update_pc(ex_mem.branch_target);
    
    // Check if we have to read or write to the memory
    int32_t read_data_memory = 0;
    // << Need to confirm the write address .. >>
    if (ex_mem.mem_read)
        read_data_memory = data_memory[(ex_mem.alu_result - 0x10000000) / 4];
    
    // << Need to confirm the write memory address ..>>
    if (ex_mem.mem_write)
        data_memory[(ex_mem.alu_result - 0x10000000) / 4] = ex_mem.read_data2;
    
    // Check what value to send to the write register
    mem_wb.write_register = ex_mem.write_register;
    mem_wb.write_data = (ex_mem.mem_to_reg)? read_data_memory : ex_mem.alu_result;
    
    // Passing the control signals
    mem_wb.reg_write = ex_mem.reg_write;
    
    // Check if the instruction is a jump instruction.
    check_jump();
    
    return 0;
}


int writeback()
{
    if (mem_wb.is_syscall) {
        // Copy the content of the v0 register to determine the type of operation.
        syscall(cpu_ctx.GPR[2], 4);
    } else if (mem_wb.reg_write)
    // Writing the data in the given register
        cpu_ctx.GPR[mem_wb.write_register] = mem_wb.write_data;
	return 0;
}

int32_t shift_and_find(uint32_t instruction, uint8_t left, uint8_t right){
    return (instruction << left) >> right;
}

void set_control_signals(char arr[]){
    id_ex.branch = convert_to_bool(arr[0]);
    id_ex.mem_read = convert_to_bool(arr[1]);
    id_ex.mem_to_reg = convert_to_bool(arr[2]);
    id_ex.mem_write = convert_to_bool(arr[3]);
    id_ex.reg_write = convert_to_bool(arr[4]);
    id_ex.alu_src = convert_to_bool(arr[5]);
    id_ex.branch_ne = convert_to_bool(arr[6]);
    id_ex.reg_dst = convert_to_bool(arr[7]);
}


bool convert_to_bool(char a) {
	return a != '0';
}

int32_t alu_operation(){
    int32_t src1 = forward(id_ex.read_address1);
    int32_t src2 = forward(id_ex.read_address2);
    src2 = (id_ex.alu_src) ? id_ex.sign_extended_immediate : src2;
    uint32_t u_src2 = src2;
    switch(id_ex.alu_control){
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
            return src2 << (id_ex.shamt);
        case SLT:
            return (src1 < src2) ? 1 : 0;
        case SRL:
            return u_src2 >> (id_ex.shamt);
        case SRA:
            return  src2 >> (id_ex.shamt);
        default:
            return 0;
    }
}


void check_jump(){
    switch(ex_mem.jump_ctrl){
        case NONE:
            return;
            
        case JUMP:
            update_pc(ex_mem.jump_address);
            return;
            
        case JAL:
            mem_wb.write_data = ex_mem.next_pc;
            update_pc(ex_mem.jump_address);
            mem_wb.write_register = 31;
            mem_wb.reg_write = true;
            return;
            
        case JR:
            update_pc(ex_mem.alu_result);
            return;
    }
}


int decode_rformat(uint8_t funct, char *control_signal){
    switch (funct)
    {
        case 8: //jr
            id_ex.jump_ctrl = JR;
            strcpy(control_signal, "00000000");
            id_ex.alu_control = ADD;
        case 32:
            id_ex.alu_control = ADD;
            break;
        case 33:
            id_ex.alu_control = ADD;
            break;
        case 36:
            id_ex.alu_control = AND;
            break;
        case 37:
            id_ex.alu_control = OR;
            break;
        case 0:
            id_ex.alu_control = SLL;
            break;
        case 42:
            id_ex.alu_control = SLT;
            break;
        case 2:
            id_ex.alu_control = SRL;
            break;
        case 39:
            id_ex.alu_control = NOR;
            break;
        case 3:
            id_ex.alu_control = SRA;
            break;
        case 38:
            id_ex.alu_control = XOR;
            break;
        case 40:
            id_ex.alu_control = SUB;
            break;
    }
    return 0;
}


int decode_opcode(uint8_t opcode, uint8_t funct){
    char control_signal[9];
    switch(opcode){
        case 0:
            strcpy(control_signal, "00001001");
            if(funct == 12){ // Syscall
                id_ex.is_syscall = true;
                break;
            }
            decode_rformat(funct, control_signal);
            break;
        case 2: // Jump instruction
            id_ex.jump_ctrl = JUMP;
            strcpy(control_signal, "00000000");
            break;
        case 3: // JAL instruction
            id_ex.jump_ctrl = JAL;
            strcpy(control_signal, "00000000");
        case 8: // Add immediate.
            id_ex.alu_control = ADD;
            strcpy(control_signal, "00001100");
            break;
        case 12: // And immediate.
            id_ex.alu_control = AND;
            strcpy(control_signal, "00001100");
            break;
        case 4: // Branch if equal.
            id_ex.alu_control = SUB;
            strcpy(control_signal, "10000000");
            break;
        case 5: // Branch if not equal.
            id_ex.alu_control = SUB;
            strcpy(control_signal, "10000010");
            break;
        case 13: // Or immediate.
            id_ex.alu_control = OR;
            strcpy(control_signal, "00001100");
            break;
        case 10: // Set if less than immediate.
            id_ex.alu_control = SLT;
            strcpy(control_signal, "00001100");
            break;
        case 14: // XOR immediate.
            id_ex.alu_control = XOR;
            strcpy(control_signal, "00001100");
            break;
        case 35: // Load word.
            id_ex.alu_control = SUB;
            strcpy(control_signal, "01101100");
            break;
        case 43: // Store Word.
            id_ex.alu_control = ADD;
            strcpy(control_signal, "00010100");
            break;
        case 15: // Load upper immediate.
            id_ex.alu_control = LUI;
            strcpy(control_signal, "00001100");
            break;
    }
    set_control_signals(control_signal);
    return 0;
}

int hazard_detection(){
# if defined(ENABLE_FORWARDING)
    if (ex_mem.mem_read &&
        (id_ex.read_address1 == ex_mem.write_register ||
        id_ex.read_address2 == ex_mem.write_register)){
            stall();
    }
# endif
    
# if !defined(ENABLE_FORWARDING)
    // If forwarding is not defined
    if (ex_mem.reg_write && ex_mem.write_register != 0 &&
       (id_ex.read_address1 == ex_mem.write_register ||
        id_ex.read_address2 == ex_mem.write_register)){
           stall();
    } else if (mem_wb.reg_write && ex_mem.write_register != 0 &&
       (id_ex.read_address1 == mem_wb.write_register ||
        id_ex.read_address2 == mem_wb.write_register)){
           stall();
    }
#endif
    // Check for branch instruction
    if (branch_taken()){
        printf("Squashed");
        // Squashing the instruction fetched
        squash = true;
        squashs += 2;
        // Squashing the instruction in decode stage
        id_ex.reg_write = 0;
        id_ex.mem_write = 0;
        
        id_ex.is_syscall = false;
    }
    
    // For jumps
    if (ex_mem.jump_ctrl != NONE){
        printf("Squashed");
        squash = true;
        squashs += 2;
        
        id_ex.reg_write = 0;
        id_ex.mem_write = 0;
        id_ex.is_syscall = false;
        
    }
    return 0;
    
    
}

void update_pc(uint32_t new_pc){
    if (pc_write){
        cpu_ctx.PC = new_pc;
    }
}

void add_instruction(uint32_t new_instruction){
    if (if_id_write){
        if_id.instruction = new_instruction;
    }
    if (squash){
        if_id.instruction = 0;
    }
}

void reset_write_signals(){
    pc_write = true;
    if_id_write = true;
    squash = false;
}

void stall(){
    printf("stalled");
    stalls++;
    pc_write = false;
    if_id_write = false;
    id_ex.reg_write = 0;
    id_ex.mem_write = 0;
    id_ex.is_syscall = false;
}

int32_t forward(uint32_t reg_addr){
# if defined(ENABLE_FORWARDING)
    if (ex_mem.reg_write == 1 && ex_mem.write_register == reg_addr)
        return ex_mem.alu_result;
    if (mem_wb.reg_write == 1 && mem_wb.write_register == reg_addr)
        return mem_wb.write_data;
# endif
    return cpu_ctx.GPR[reg_addr];
}

void decode_instructions(){
    // Figuring out addresses and data
    id_ex.is_syscall = false;
    id_ex.read_address1 = shift_and_find(if_id.instruction, 6, 27);
    id_ex.read_address2 = shift_and_find(if_id.instruction, 11, 27);
    id_ex.write_register = shift_and_find(if_id.instruction, 16, 27);
    id_ex.shamt = shift_and_find(if_id.instruction, 21, 27);
    id_ex.jump_address = shift_and_find(if_id.instruction, 6, 6);
    id_ex.sign_extended_immediate = shift_and_find(if_id.instruction, 16, 16);
    id_ex.read_data2 = cpu_ctx.GPR[id_ex.read_address2];
    id_ex.read_data1 = cpu_ctx.GPR[id_ex.read_address1];
    id_ex.next_pc = if_id.next_pc;
    id_ex.jump_ctrl = NONE;
}

void register_destination_multiplexer(){
    if (!id_ex.reg_dst) id_ex.write_register = shift_and_find(if_id.instruction, 11, 27);
}

int no_of_stalls(){
    return stalls;
}

int no_of_squashes(){
    return squashs;
}

bool branch_taken(){
    return (ex_mem.branch && ex_mem.alu_result == 0) ||
    (ex_mem.branch_ne && ex_mem.alu_result != 0);
}
