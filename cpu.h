/**
 * Gedare Bloom
 * cpu.h
 *
 * Definitions for the processor.
 */

#include <stdint.h>

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

};

struct EX_MEM_buffer {

};

struct MEM_WB_buffer {

};

int fetch( struct IF_ID_buffer *out );
int decode( struct IF_ID_buffer *in, struct ID_EX_buffer *out );
int execute( struct ID_EX_buffer *in, struct EX_MEM_buffer *out );
int memory( struct EX_MEM_buffer *in, struct MEM_WB_buffer *out );
int writeback( struct MEM_WB_buffer *in );

