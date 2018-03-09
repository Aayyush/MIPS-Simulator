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
extern struct IF_ID_buffer if_id;
extern struct ID_EX_buffer id_ex;
extern struct EX_MEM_buffer ex_mem;
extern struct MEM_WB_buffer mem_wb;

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
    bool branch_taken;
    
    // Control Signal to ALU.
    enum ALUCONTROL alu_control;
    
    // Jump instructions
    enum JUMP_CONTROL jump_ctrl;
    
    uint32_t instruction;
};

struct EX_MEM_buffer {
    uint32_t next_pc;
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
    bool is_syscall;
    
    // Jump instructions
    enum JUMP_CONTROL jump_ctrl;
    
    uint32_t instruction;
};

struct MEM_WB_buffer {
    uint32_t write_register;
    int32_t write_data;
    
    // Control Signals.
    bool reg_write;
    bool is_syscall;
    
    uint32_t instruction;
};

int fetch(void);
int decode(void);
int execute(void);
int memory(void);
int writeback(void);

int32_t shift_and_find(uint32_t instruction, uint8_t left, uint8_t right);
int decode_opcode(uint8_t opcode, uint8_t funct);
void set_control_signals(char arr[]);
int32_t alu_operation(void);
void check_jump(void);
bool convert_to_bool(char);

// Pipeline specific functions
int hazard_detection(void);
void update_pc(uint32_t new_pc);
void add_instruction(uint32_t new_instruction);
void reset_write_signals(void);
void stall(void);
int32_t forward(uint32_t reg_addr);
void decode_instructions(void);
void register_destination_multiplexer(void);
int no_of_stalls(void);
int no_of_squashes(void);
bool branch_taken(void);
