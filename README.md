# mips-sim
simulator for a small subset of mips assembly in C
The input to mips_sim will be the 32-bit instruction codes for MIPS instructions as hexadecimal numbers.

MIPS functions available:
- add 
- sub 
- slt 
- mul 
- beq 
- bne 
- addi 
- ori 
- lui 
- syscall 

Syscalls available:
- print int (1)
- exit (10)
- print character (11)

Example:
cat examples/42.hex
3404002a
34020001
c
3404000a
3402000b
c
dcc mips_sim.c -o mips_sim
./mips_sim examples/42.hex
0: 0x3404002A ori $4, $0, 42
>>> $4 = 42
1: 0x34020001 ori $2, $0, 1
>>> $2 = 1
2: 0x0000000C syscall
>>> syscall 1
<<< 42
3: 0x3404000A ori $4, $0, 10
>>> $4 = 10
4: 0x3402000B ori $2, $0, 11
>>> $2 = 11
5: 0x0000000C syscall
>>> syscall 11
<<<
