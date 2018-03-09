/**
 * Gedare Bloom
 * Bijesh Subedi
 * Kishor Subedi
 * Aayush Gupta
 * Suraj Upreti
 * syscall.c
 *
 * Implementation of the system calls
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "memory.h"
#include "cpu.h"


void sys_exit() {
	printf("Exiting\n");
	exit(0);
}

void syscall(uint32_t num, int32_t out) {
    uint32_t curr_addr = cpu_ctx.GPR[out];
	switch (num) {
	case 10:
        // Completing the remaining stages of the cycles
        
        // Print the total number of stalls and squashes
        printf("The total number of stalls:  %d\n", no_of_stalls());
        printf("the total number of squsash: %d\n", no_of_squashes());
		// Sysexit statement.
		sys_exit();
		break;
	case 1:
		// Print integer. TODO: Change the index according to the address.
            printf("The integer is: %d \n", cpu_ctx.GPR[out]);
		break;
	case 4:
		// Print String. Print until new line character. TODO: Change the index according to the address.
		while (data_memory[curr_addr] != '\0') {
			printf("%d", data_memory[curr_addr]);
			curr_addr += 4; // byte addressed for 8-bit characters.
		}
		break;
	}
    printf("---------------------------- \n");
}

