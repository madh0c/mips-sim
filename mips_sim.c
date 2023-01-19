// COMP1521 20T3 Assignment 1: mips_sim -- a MIPS simulator
// by Jeffery Pan (z5310210)


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define MAX_LINE_LENGTH 256
#define INSTRUCTIONS_GROW 64

#define N_BITS 32
#define N_REGS 32

// miscellaneous
#define LAST_11_BITS 2047
#define SYSCALL_REG 2

#define PRINT_INT 1
#define PRINT_CHAR 11
#define SYSCALL_EXIT 10

// instruction IDs
#define ADD 32
#define SUB 34
#define SLT 42
#define MUL 28
#define BEQ 4
#define BNE 5
#define ADDI 8
#define ORI 13
#define LUI 15
#define SYSCALL 12

// register position inside info_array
#define S_REG 0
#define T_REG 1
#define D_REG 2
#define IMM 3

void main_assembly(int n_instructions,
                          uint32_t instructions[n_instructions],
                          int trace_mode);
char *process_arguments(int argc, char *argv[], int *trace_mode);
uint32_t *read_instructions(char *filename, int *n_instructions_p);
uint32_t *instructions_realloc(uint32_t *instructions, int n_instructions);


// ADD YOUR FUNCTION PROTOTYPES HERE
char* execute_instructions(int *pc_p, uint32_t current_hex, int info_array[4], int registers[N_REGS]);
void print_trace(int pc, int n_instructions, char *command, int info_array[4], int registers[N_REGS]);
void insert_info(uint32_t current_hex, int info_array[4]);
int syscall_output(int registers[N_REGS]);


// YOU SHOULD NOT NEED TO CHANGE MAIN

int main(int argc, char *argv[]) {
    int trace_mode;
    char *filename = process_arguments(argc, argv, &trace_mode);

    int n_instructions;
    uint32_t *instructions = read_instructions(filename, &n_instructions);

    main_assembly(n_instructions, instructions, trace_mode);

    free(instructions);
    return 0;
}


// simulate execution of  instruction codes in  instructions array
// output from syscall instruction & any error messages are printed
//
// if trace_mode != 0:
//     information is printed about each instruction as it executed
//
// execution stops if it reaches the end of the array

// main loop that simulates line-by-line execution
void main_assembly(int n_instructions,
                          uint32_t instructions[n_instructions],
                          int trace_mode) {
    
    // assembly array
    int assembly[N_REGS] = {0};
    
    // initialise program counter and a pointer to it
    int pc = 0;
    int  *pc_p = &pc;
    
    // initialise array that holds information about each instruction
    int info_array[4]; 
    
    // loop
    while (pc < n_instructions) { 
        // reset $0 to 0
        assembly[0] = 0; 

        // extract registers / imm from hex and insert them into an array              
        insert_info(instructions[pc], info_array); 

        // a copy of pc to avoid accessing illegal indexes in the case of bad branches 
        int pc_c = pc;

        // execution
        char *current_instruct = execute_instructions(pc_p, instructions[pc], info_array, assembly);       
        
        // print when there's trace mode is on
        if (trace_mode) {
            // print current pc and hex
            printf(
                "%d: 0x%08X ", 
                pc_c, 
                instructions[pc_c]
            );

            print_trace(pc, n_instructions, current_instruct, info_array, assembly);   

        } 

        // trace mode off i.e. print only the output of syscall and/or error
        else if (strcmp(current_instruct, "syscall") == 0) {
            // print integer
            if (assembly[SYSCALL_REG] == 1) {
                printf("%d", syscall_output(assembly));
            } else
            // print char
            if (assembly[SYSCALL_REG] == 11) {
                printf("%c", syscall_output(assembly));
            } else 
            // exit if invalid
            {
                syscall_output(assembly);
            }
        }

        // bad instruction
        if (strcmp(current_instruct, "invalid") == 0) {
            printf("invalid instruction code\n");
            exit(0);
        }        

        pc++;      

        // bad branch        
        if (pc < 0 || pc > n_instructions) {
            printf("Illegal branch to address before instructions: PC = %d\n", pc);
            
            exit(0);
        }         
    }

    return;
}



// FUNCTIONS 
// converts the hex code to an instruction, executes it and returns it as a string
char *execute_instructions(int *pc_p, uint32_t current_hex, int info_array[4], int registers[N_REGS]) {
    char *command = "invalid";
    // identify and execute the instructions
    // add, sub, slt, syscall
    if ((current_hex >> (N_BITS - 6)) == 0) {
        if ((current_hex & LAST_11_BITS) == ADD) {
            // $d = $s + $t
            registers[info_array[D_REG]] = registers[info_array[S_REG]] + registers[info_array[T_REG]];
            command = "add";
        }

        if ((current_hex & LAST_11_BITS) == SUB) {
            // $d = $s - $t
            registers[info_array[D_REG]] = registers[info_array[S_REG]] - registers[info_array[T_REG]];
            command = "sub";
        }
        
        if ((current_hex & LAST_11_BITS) == SLT) {
            // $d = 1 iff $s < $t, else $d = 0
            if (registers[info_array[S_REG]] < registers[info_array[T_REG]]) {
                registers[info_array[D_REG]] = 1;
            } else {
                registers[info_array[D_REG]] = 0;
            }
            command = "slt";
        }

        if ((current_hex & LAST_11_BITS) == SYSCALL) {
            // execution handled by main_assembly
            command = "syscall";
        }
        
    }     
    // the other 6 instructions
    else {
        if ((current_hex >> (N_BITS - 6)) == MUL) {    
            // $d = $s * $t        
            registers[info_array[D_REG]] = registers[info_array[S_REG]] * registers[info_array[T_REG]];
            command = "mul";
        }

        if ((current_hex >> (N_BITS - 6)) == BEQ) {      
            // branches if $s = $t
            if (registers[info_array[S_REG]] == registers[info_array[T_REG]]) {
                // converts to signed 16 bit integer
                __int16_t offset = info_array[IMM];     
                
                // use pointer to pc to change original pc value
                // -1 cancels out the pc++ in main_assembly
                *pc_p = *pc_p + offset - 1;                
            }
            command = "beq";
        }

        if ((current_hex >> (N_BITS - 6)) == BNE) {
            // branches if $s != $t
            if (registers[info_array[S_REG]] != registers[info_array[T_REG]]) {
                __int16_t offset = info_array[IMM];                
                *pc_p = *pc_p + offset - 1;
            }
            command = "bne";            
            return command;
        }

        if ((current_hex >> (N_BITS - 6)) == ADDI) {
            __int16_t I = info_array[IMM];
            // $t = $s + I
            registers[info_array[T_REG]] = registers[info_array[S_REG]] + I;
            command = "addi";
        }

        if ((current_hex >> (N_BITS - 6)) == ORI) {
            // $t = $s | I
            registers[info_array[T_REG]] = registers[info_array[S_REG]] | info_array[IMM];
            command = "ori";
        }

        if ((current_hex >> (N_BITS - 6)) == LUI) {
            __int16_t I = info_array[IMM];
            // $t = I << 16
            registers[info_array[T_REG]] = I << 16;
            command = "lui";
        }
    }

    return command;
}

// prints instruction, relevant registers and output
void print_trace(int pc, int n_instructions, char *command, int info_array[4], int registers[N_REGS]) {
    // invalid instruction 
    if (strcmp(command, "invalid") == 0) {
        printf("invalid instruction code\n");
        exit(0);
    }

    // print the current instruction
    printf("%s ", command);

    // print relevant registers and outputs    
    if (
        strcmp(command, "add") == 0 
        || strcmp(command, "sub") == 0
        || strcmp(command, "slt") == 0
        || strcmp(command, "mul") == 0
    ) {
        // print registers in order: d, s, t
        printf(" $%d, $%d, $%d\n", info_array[D_REG], info_array[S_REG], info_array[T_REG]);
        
        // output
        printf(">>> $%d = %d", info_array[D_REG], registers[info_array[D_REG]]);
    }

    // print registers in order: s, t and IMM value
    if (strcmp(command, "beq") == 0) {
        __int16_t branch = info_array[IMM];
        
        printf (" $%d, $%d, %d\n", info_array[S_REG], info_array[T_REG], branch);        
        
        if (registers[info_array[S_REG]] == registers[info_array[T_REG]]) {
            printf(">>> branch taken to PC = %d", pc + 1);
            
            // bad branch
            if ((pc + 1) < 0 || (pc + 1) > n_instructions) {
                printf("\nIllegal branch to address before instructions: PC = %d\n", pc + 1);
                
                exit(0);
            }
        } else {
            printf(">>> branch not taken");
        }
    }    

    if (strcmp(command, "bne") == 0) {
        __int16_t branch = info_array[IMM];
        printf (" $%d, $%d, %d\n", info_array[S_REG], info_array[T_REG], branch);        
        
        if (registers[info_array[S_REG]] != registers[info_array[T_REG]]) {
            printf(">>> branch taken to PC = %d", pc + 1);
            
            // bad branch
            if ((pc + 1) < 0 || (pc + 1) > n_instructions) {
                printf("\nIllegal branch to address before instructions: PC = %d\n", pc + 1);
                
                exit(0);
            }

        } else {
            printf(">>> branch not taken");
        }
    }

    // print registers in order: t, s and IMM value 
    if (
        strcmp(command, "addi") == 0 
        || strcmp(command, "ori") == 0
    ) {
        __int16_t I = info_array[IMM];
        if (strcmp(command, "ori") == 0) {
            printf(" "); // there is one less space after addi in reference
        }     
        printf ("$%d, $%d, %d\n", info_array[T_REG], info_array[S_REG], I);
        printf(">>> $%d = %d", info_array[T_REG], registers[info_array[T_REG]]);
    }

    // print registers in order: t, IMM value
    if (strcmp(command, "lui") == 0) {
        printf (" $%d, %d\n", info_array[T_REG], info_array[IMM]);
        printf(">>> $%d = %d", info_array[T_REG], registers[info_array[T_REG]]);
    }   

    // print corresponding syscall and its output to terminal
    if (strcmp(command, "syscall") == 0) {
        printf("\n>>> syscall %d\n", registers[SYSCALL_REG]);
        int output = syscall_output(registers);       

        // print int
        if (registers[SYSCALL_REG] == PRINT_INT) {
            printf("<<< %d", output);
        }

        // print char
        if (registers[SYSCALL_REG] == PRINT_CHAR) {
            printf("<<< %c", output);
        }
    }    
    printf("\n");

    return;
}

// extract register numbers / immediate values and store them into info_array
// in the following order: $s, $t, $d, I
void insert_info(uint32_t current_hex, int info_array[4]) {    
    // extract $s and place it in 0th pos of array
    info_array[S_REG] = (current_hex >> 21) & 0x1F;

    // extract $t and place it in 1st pos of array
    info_array[T_REG] = (current_hex >> 16) & 0x1F;

    // extract $d and place it in 2nd pos of array
    info_array[D_REG] = (current_hex >> 11) & 0x1F;
    
    // extract I and place it in 3rd pos of array
    info_array[IMM] = current_hex & 0xFFFF;
    
    return;
}

// if $v0 = 1 || $v0 = 11, syscall_output will return the string
// containing the int or char. else if $v0 = 10, exit the program
// if invalid, print error msg then exit program
int syscall_output(int registers[N_REGS]) {
    int output = 0; // set to 0 to silence uninitialized variable warnings
    if (registers[SYSCALL_REG] == PRINT_CHAR || registers[SYSCALL_REG] == PRINT_INT) {
        output = registers[4];
    } else if (registers[SYSCALL_REG] == SYSCALL_EXIT) {
        exit(0);
    } else {
        printf("Unknown system call: %d\n", registers[SYSCALL_REG]);
        exit(0);
    }

    return output;
}


// YOU DO NOT NEED TO CHANGE CODE BELOW HERE


// check_arguments is given command-line arguments
// it sets *trace_mode to 0 if -r is specified
//          *trace_mode is set to 1 otherwise
// the filename specified in command-line arguments is returned

char *process_arguments(int argc, char *argv[], int *trace_mode) {
    if (
        argc < 2 ||
        argc > 3 ||
        (argc == 2 && strcmp(argv[1], "-r") == 0) ||
        (argc == 3 && strcmp(argv[1], "-r") != 0)) {
        fprintf(stderr, "Usage: %s [-r] <file>\n", argv[0]);
        exit(1);
    }
    *trace_mode = (argc == 2);
    return argv[argc - 1];
}


// read hexadecimal numbers from filename one per line
// numbers are return in a malloc'ed array
// *n_instructions is set to size of the array

uint32_t *read_instructions(char *filename, int *n_instructions_p) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "%s: '%s'\n", strerror(errno), filename);
        exit(1);
    }

    uint32_t *instructions = NULL;
    int n_instructions = 0;
    char line[MAX_LINE_LENGTH + 1];
    while (fgets(line, sizeof line, f) != NULL) {

        // grow instructions array in steps of INSTRUCTIONS_GROW elements
        if (n_instructions % INSTRUCTIONS_GROW == 0) {
            instructions = instructions_realloc(instructions, n_instructions + INSTRUCTIONS_GROW);
        }

        char *endptr;
        instructions[n_instructions] = strtol(line, &endptr, 16);
        if (*endptr != '\n' && *endptr != '\r' && *endptr != '\0') {
            fprintf(stderr, "%s:line %d: invalid hexadecimal number: %s",
                    filename, n_instructions + 1, line);
            exit(1);
        }
        n_instructions++;
    }
    fclose(f);
    *n_instructions_p = n_instructions;
    // shrink instructions array to correct size
    instructions = instructions_realloc(instructions, n_instructions);
    return instructions;
}


// instructions_realloc is wrapper for realloc
// it calls realloc to grow/shrink the instructions array
// to the speicfied size
// it exits if realloc fails
// otherwise it returns the new instructions array
uint32_t *instructions_realloc(uint32_t *instructions, int n_instructions) {
    instructions = realloc(instructions, n_instructions * sizeof *instructions);
    if (instructions == NULL) {
        fprintf(stderr, "out of memory");
        exit(1);
    }
    return instructions;
}