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
