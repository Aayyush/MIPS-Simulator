/**
 * Gedare Bloom
 * main.c
 *
 * Drives the simulation
 */

#include "cpu.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

uint32_t buffer[2048];

int main( int argc, char *argv[] )
{
	FILE *f;
	struct IF_ID_buffer if_id;
	struct ID_EX_buffer id_ex;
	struct EX_MEM_buffer ex_mem;
	struct MEM_WB_buffer mem_wb;
	int i;

	for ( i = 0; i < 2048; i++ ) {
		buffer[i] = 0;
	}

	f = fopen(argv[1], "r");
	for ( i = 0; i < 2048; i++ ) {
		fread(&buffer[i], sizeof(uint32_t), 1, f);
		printf("%d\n", buffer[i]);
	}
	fclose(f);

	fetch( &if_id );
	decode( &if_id, &id_ex );
	execute( &id_ex, &ex_mem );
	memory( &ex_mem, &mem_wb );
	writeback( &mem_wb );

	return 0;
}
