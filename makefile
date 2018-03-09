CC=gcc
CFLAGS= -Wall
DEPS = cpu.h memory.h syscall.h

single-cycle-simulator: single-cycle.o cpu.o memory.o syscall.o
	gcc single-cycle.o cpu.o memory.o syscall.o -o single-cycle-simulator

pipeline: pipeline.o cpu.o memory.o syscall.o
	gcc pipeline.o cpu.o memory.o syscall.o -o pipeline

test-simulator: test.o cpu.o memory.o syscall.o
	gcc test.o memory.o syscall.o cpu.o -o test-simulator

pipeline.o: pipeline.c
	gcc $(CFLAGS) -c pipeline.c -o pipeline.o

single-cycle.o: single-cycle.c
	gcc $(CFLAGS) -c single-cycle.c -o single-cycle.o

cpu.o: cpu.c cpu.h
	gcc $(CFLAGS) -c cpu.c -o cpu.o

syscall.o: syscall.c syscall.h
	gcc $(CFLAGS) -c syscall.c -o syscall.o

memory.o: memory.c memory.h
	gcc $(CFLAGS) -c memory.c -o memory.o


clean: 
	rm -f create-prog.o cpu.o memory.o syscall.o pipeline.o test.o single-cycle.o
	rm -f singlete-program single-cycle-simulator create-program pipeline test-simulator
	rm -f build-program.o program.sim

create-program: create-prog.o
	gcc create-prog.o -o create-program

create-prog.o: util/create-prog.c
	gcc $(CFLAGS) -c util/create-prog.c -o create-prog.o

build-program: create-program
	./create-program util/example-data.txt util/example-text.txt program.sim

