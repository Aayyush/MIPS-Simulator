/**
 * Gedare Bloom
 * Bijesh Subedi
 * Kishor Subedi
 * Aayush Gupta
 * Suraj Upreti
 * cpu.h
 *
 * Definitions for the processor.
 */

#include <stdint.h>
#include <stdbool.h>

enum ALUCONTROL { ADD, AND, OR, NOR, SLT, SLL, SRL, SRA, XOR, SUB, LUI};
enum JUMP_CONTROL {NONE, JUMP, JAL, JR};

struct cpu_context {
	uint32_t PC;
	uint32_t GPR[32];
};

extern struct cpu_context cpu_ctx;

struct IF_ID_buffer {
	uint32_t instruction;
	uint32_t next_pc;
};

struct ID_EX_buffer {
    int32_t read_data1;
    int32_t read_data2;
	int16_t sign_extended_immediate;
    uint32_t read_address1;
	uint32_t read_address2;
    uint32_t jump_address;
	uint32_t write_register;
	uint32_t next_pc;
	uint8_t shamt;
	bool is_syscall;

	// Control Signals.
	bool reg_dst;
	bool branch;
	bool mem_read;
	bool mem_write;
	bool reg_write;
	bool mem_to_reg;
	bool alu_src;
    bool branch_ne;

	// Control Signal to ALU.
	enum ALUCONTROL alu_control;
    
    // Jump instructions
    enum JUMP_CONTROL jump_ctrl;
};

struct EX_MEM_buffer {
	uint32_t branch_target;
	int32_t alu_result;
	uint32_t read_data2;
	uint32_t write_register;
    uint32_t jump_address;
	bool alu_zero;
    
	// Control Signals.
	bool branch;
	bool mem_read;
	bool mem_write;
	bool reg_write;
	bool mem_to_reg;
    bool branch_ne;
    
    // Jump instructions
    enum JUMP_CONTROL jump_ctrl;
};

struct MEM_WB_buffer {
	uint32_t write_register;
    int32_t write_data;

	// Control Signals.
	bool reg_write;
};

int fetch( struct IF_ID_buffer *out );
int decode( struct IF_ID_buffer *in, struct ID_EX_buffer *out );
int execute( struct ID_EX_buffer *in, struct EX_MEM_buffer *out );
int memory( struct EX_MEM_buffer *in, struct MEM_WB_buffer *out );
int writeback( struct MEM_WB_buffer *in );

int32_t shift_and_find(uint32_t instruction, uint8_t left, uint8_t right);
int decode_opcode(uint8_t opcode, uint8_t funct, struct IF_ID_buffer *in, struct ID_EX_buffer *out);
void set_control_signals(char arr[], struct ID_EX_buffer *out);
int32_t alu_operation(struct ID_EX_buffer *in);
void check_jump(struct EX_MEM_buffer *in, struct MEM_WB_buffer *out);
bool convert_to_bool(char);

