#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 256
#define NUM_REGISTERS 8
#define NETWORK_BUFFER_SIZE 256

typedef enum {
    LOAD = 0,
    STORE,
    ADD,
    SUB,
    JMP,
    HALT,
    SEND,
    RECEIVE
} Instruction;

typedef struct {
    int registers[NUM_REGISTERS];
    int memory[MEMORY_SIZE];
    int pc; // Program Counter
} VM;

typedef struct {
    char buffer[NETWORK_BUFFER_SIZE];
    int buffer_size;
} NetworkInterface;

// Function declarations
void vm_init(VM *vm);
int vm_load_program_from_memory(VM *vm, int *program, int program_size);
void vm_execute(VM *vm, NetworkInterface *net);
void network_init(NetworkInterface *net);
void network_send(NetworkInterface *net, const char *data, int size);
int network_receive(NetworkInterface *net, char *buffer, int size);

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

// Function to load a program into the VM's memory from a binary file
int vm_load_program_from_memory(VM *vm, int *program, int program_size) {
    for (int i = 0; i < program_size; i++) {
        vm->memory[i] = program[i];
    }
    return program_size;
}

// Function to execute the program in the VM
void vm_execute(VM *vm, NetworkInterface *net) {
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
            case SEND:
                network_send(net, (char *)&vm->registers[operand1], operand2);
                break;
            case RECEIVE:
                vm->registers[operand1] = network_receive(net, (char *)&vm->registers[operand2], operand2);
                break;
            default:
                fprintf(stderr, "Unknown opcode: %d\n", opcode);
                exit(EXIT_FAILURE);
        }
    }
}

// Function to initialize the network interface
void network_init(NetworkInterface *net) {
    net->buffer_size = 0;
    memset(net->buffer, 0, NETWORK_BUFFER_SIZE);
}

// Function to send data over the network interface
void network_send(NetworkInterface *net, const char *data, int size) {
    if (size > NETWORK_BUFFER_SIZE) {
        fprintf(stderr, "Data size exceeds network buffer size\n");
        return;
    }
    memcpy(net->buffer, data, size);
    net->buffer_size = size;
    printf("Sent data: %.*s\n", size, data);
}

// Function to receive data over the network interface
int network_receive(NetworkInterface *net, char *buffer, int size) {
    if (net->buffer_size == 0) {
        return 0;
    }
    if (size < net->buffer_size) {
        fprintf(stderr, "Buffer size too small to receive data\n");
        return -1;
    }
    memcpy(buffer, net->buffer, net->buffer_size);
    int received_size = net->buffer_size;
    net->buffer_size = 0;
    printf("Received data: %.*s\n", received_size, buffer);
    return received_size;
}

// Function to simulate kernel entry point
void _start(VM *vm, NetworkInterface *net) {
    // Example program: Load 5 into R0, Load 3 into R1, Add R0 and R1, Store result in memory at address 10, Halt
    int program[] = {
        LOAD, 0, 5,  // Load 5 into R0
        LOAD, 1, 3,  // Load 3 into R1
        ADD, 0, 1,   // R0 = R0 + R1
        STORE, 0, 10, // Store R0 into memory at address 10
        HALT        // Halt the VM
    };
    int program_size = sizeof(program) / sizeof(program[0]);

    // Load the assembled program into the VM's memory
    program_size = vm_load_program_from_memory(vm, program, program_size);
    if (program_size == -1) {
        exit(EXIT_FAILURE);
    }

    // Execute the program
    vm_execute(vm, net);

    // Print result (for debugging purposes)
    printf("Memory at address 10: %d\n", vm->memory[10]); // Should print 8
    printf("Register R3: %d\n", vm->registers[3]); // Should print received data
}