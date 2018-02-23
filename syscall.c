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

void sys_exit() {
	printf("Exiting\n");
	exit(0);
}

void syscall(uint32_t num, uint32_t out) {
	switch (num) {
	case 10:
		// Sysexit statement.
		sys_exit();
		break;
	case 1:
		// Print integer. TODO: Change the index according to the address.
        printf("%d", data_memory[out]);
		break;
	case 4:
		// Print String. Print until new line character. TODO: Change the index according to the address.
		while (data_memory[out] != '\0') {
			printf("%d", data_memory[out]);
			out++; // byte addressed for 8-bit characters.
		}
		break;
	}
}

