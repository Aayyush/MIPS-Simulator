/**
 * Gedare Bloom
 * memory.h
 *
 * Definitions for the memory.
 */

#include <stdint.h>

extern uint32_t instruction_memory[1024]; /* 0x400000 - 0x401000 */
extern uint32_t data_memory[1024]; /* 0x10000000 - 0x10001000 */
extern uint32_t stack_memory[1024]; /* 0x20000000 - 0x20001000 */

