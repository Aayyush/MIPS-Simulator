/**
 * Gedare Bloom
 * cpu.c
 *
 * Implementation of simulated processor.
 */

#include "cpu.h"
#include "memory.h"

struct cpu_context cpu_ctx;

int fetch( struct IF_ID_buffer *out )
{
	return 0;
}

int decode( struct IF_ID_buffer *in, struct ID_EX_buffer *out )
{
	return 0;
}

int execute( struct ID_EX_buffer *in, struct EX_MEM_buffer *out )
{
	return 0;
}

int memory( struct EX_MEM_buffer *in, struct MEM_WB_buffer *out )
{
	return 0;
}

int writeback( struct MEM_WB_buffer *in )
{
	return 0;
}


