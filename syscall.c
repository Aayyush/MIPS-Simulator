/**
 * Gedare Bloom
 * syscall.c
 *
 * Implementation of the system calls
 */

#include <stdlib.h>
#include <stdio.h>

int syscall(int num, ...) {
	if ( num == 10 ) sys_exit();
}

int sys_exit() {
	printf("Exiting\n");
	exit(0);
}
