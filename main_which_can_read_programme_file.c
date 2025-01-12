#include <stdio.h>
#include <stdlib.h>

#define MEMORY_SIZE 256
#define NUM_REGISTERS 8

typedef enum {
    LOAD = 0,
    STORE,
    ADD,
    SUB,
    JMP,
    HALT
} Instruction;

typedef struct {
    int registers[NUM_REGISTERS];
    int memory[MEMORY_SIZE];
    int pc; // Program Counter
} VM;

// Function to initialize the VM
void vm_init(VM *vm) {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        vm->registers[i] = 0;
    }
    for (int i = 0; i < MEMORY_SIZE; i++) {
        vm->memory[i] = 0;
    }
    vm->pc = 0;
}

// Function to load a program into the VM's memory from a file
int vm_load_program_from_file(VM *vm, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return -1;
    }

    int program_size = 0;
    while (program_size < MEMORY_SIZE && fread(&vm->memory[program_size], sizeof(int), 1, file) == 1) {
        program_size++;
    }

    fclose(file);
    return program_size;
}

// Function to execute the program in the VM
void vm_execute(VM *vm) {
    while (1) {
        int opcode = vm->memory[vm->pc++];
        int operand1 = vm->memory[vm->pc++];
        int operand2 = vm->memory[vm->pc++];

        switch (opcode) {
            case LOAD:
                vm->registers[operand1] = operand2;
                break;
            case STORE:
                vm->memory[operand2] = vm->registers[operand1];
                break;
            case ADD:
                vm->registers[operand1] += vm->registers[operand2];
                break;
            case SUB:
                vm->registers[operand1] -= vm->registers[operand2];
                break;
            case JMP:
                vm->pc = operand1;
                break;
            case HALT:
                return;
            default:
                fprintf(stderr, "Unknown opcode: %d\n", opcode);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    VM vm;
    vm_init(&vm);

    int program_size = vm_load_program_from_file(&vm, argv[1]);
    if (program_size == -1) {
        return EXIT_FAILURE;
    }

    vm_execute(&vm);

    printf("Memory at address 10: %d\n", vm.memory[10]); // Should print 8

    return 0;
}