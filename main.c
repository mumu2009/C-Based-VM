#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

// Function to load a program into the VM's memory from a binary file
int vm_load_program_from_memory(VM *vm, int *program, int program_size) {
    for (int i = 0; i < program_size; i++) {
        vm->memory[i] = program[i];
    }
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

// Function to assemble a program from an assembly file to a binary file
int assemble_program(const char *input_filename, int *program) {
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Failed to open input file");
        return -1;
    }

    char line[256];
    int program_size = 0;

    while (fgets(line, sizeof(line), input_file)) {
        char opcode_str[16];
        int operand1, operand2;

        if (sscanf(line, "%s %d %d", opcode_str, &operand1, &operand2) != 3) {
            fprintf(stderr, "Invalid instruction format: %s", line);
            fclose(input_file);
            return -1;
        }

        Instruction opcode;
        if (strcmp(opcode_str, "LOAD") == 0) {
            opcode = LOAD;
        } else if (strcmp(opcode_str, "STORE") == 0) {
            opcode = STORE;
        } else if (strcmp(opcode_str, "ADD") == 0) {
            opcode = ADD;
        } else if (strcmp(opcode_str, "SUB") == 0) {
            opcode = SUB;
        } else if (strcmp(opcode_str, "JMP") == 0) {
            opcode = JMP;
        } else if (strcmp(opcode_str, "HALT") == 0) {
            opcode = HALT;
            operand1 = 0;
            operand2 = 0;
        } else {
            fprintf(stderr, "Unknown opcode: %s\n", opcode_str);
            fclose(input_file);
            return -1;
        }

        int instruction = (opcode << 24) | (operand1 << 16) | operand2;
        program[program_size++] = instruction;
    }

    fclose(input_file);
    return program_size;
}

// ISO 9660 Primary Volume Descriptor
typedef struct {
    uint8_t type;
    char id[5];
    uint8_t version;
    uint8_t unused1;
    char system_id[32];
    char volume_id[32];
    uint8_t unused2[8];
    uint32_t volume_space_size_le;
    uint32_t volume_space_size_be;
    uint8_t unused3[32];
    uint16_t volume_set_size_le;
    uint16_t volume_set_size_be;
    uint16_t volume_sequence_number_le;
    uint16_t volume_sequence_number_be;
    uint16_t logical_block_size_le;
    uint16_t logical_block_size_be;
    uint32_t path_table_size_le;
    uint32_t path_table_size_be;
    uint32_t path_table_lba_le;
    uint32_t path_table_lba_be;
    uint32_t optional_path_table_lba_le;
    uint32_t optional_path_table_lba_be;
    uint8_t root_directory_record[34];
    char volume_set_id[128];
    char publisher_id[128];
    char preparer_id[128];
    char application_id[128];
    char copyright_file_id[37];
    char abstract_file_id[37];
    char bibliographic_file_id[37];
    uint8_t creation_date[17];
    uint8_t modification_date[17];
    uint8_t expiration_date[17];
    uint8_t effective_date[17];
    uint8_t file_structure_version;
    uint8_t unused4;
    uint8_t application_data[512];
    uint8_t unused5[653];
} __attribute__((packed)) ISO9660_PVD;

// ISO 9660 Directory Record
typedef struct {
    uint8_t length;
    uint8_t extended_attribute_length;
    uint32_t lba_le;
    uint32_t lba_be;
    uint32_t data_length_le;
    uint32_t data_length_be;
    uint8_t date[7];
    uint8_t file_flags;
    uint8_t file_unit_size;
    uint8_t interleave_gap_size;
    uint16_t volume_sequence_number_le;
    uint16_t volume_sequence_number_be;
    uint8_t file_identifier_length;
    char file_identifier[1];
} __attribute__((packed)) ISO9660_DirectoryRecord;

// Function to read a file from an ISO 9660 image
int read_iso_file(const char *iso_filename, const char *file_path, int *buffer, int buffer_size) {
    FILE *iso_file = fopen(iso_filename, "rb");
    if (!iso_file) {
        perror("Failed to open ISO file");
        return -1;
    }

    ISO9660_PVD pvd;
    fseek(iso_file, 16 * 2048, SEEK_SET); // Primary Volume Descriptor is at sector 16
    fread(&pvd, sizeof(ISO9660_PVD), 1, iso_file);

    if (strncmp(pvd.id, "CD001", 5) != 0) {
        fprintf(stderr, "Not a valid ISO 9660 image\n");
        fclose(iso_file);
        return -1;
    }

    uint32_t root_lba = pvd.root_directory_record[2] | (pvd.root_directory_record[3] << 8) | (pvd.root_directory_record[4] << 16) | (pvd.root_directory_record[5] << 24);
    uint32_t root_size = pvd.root_directory_record[10] | (pvd.root_directory_record[11] << 8) | (pvd.root_directory_record[12] << 16) | (pvd.root_directory_record[13] << 24);

    fseek(iso_file, root_lba * pvd.logical_block_size_le, SEEK_SET);
    uint8_t *root_dir = malloc(root_size);
    if (!root_dir) {
        perror("Failed to allocate memory for root directory");
        fclose(iso_file);
        return -1;
    }

    fread(root_dir, 1, root_size, iso_file);

    ISO9660_DirectoryRecord *record = (ISO9660_DirectoryRecord *)root_dir;
    int offset = 0;
    int file_found = 0;

    while (offset < root_size) {
        if (record->length == 0) {
            break;
        }

        if (strncmp(record->file_identifier, file_path, record->file_identifier_length) == 0) {
            file_found = 1;
            break;
        }

        offset += record->length;
        record = (ISO9660_DirectoryRecord *)(root_dir + offset);
    }

    if (!file_found) {
        fprintf(stderr, "File not found in ISO image\n");
        free(root_dir);
        fclose(iso_file);
        return -1;
    }

    uint32_t file_lba = record->lba_le;
    uint32_t file_size = record->data_length_le;

    if (file_size > buffer_size) {
        fprintf(stderr, "Buffer too small to hold file\n");
        free(root_dir);
        fclose(iso_file);
        return -1;
    }

    fseek(iso_file, file_lba * pvd.logical_block_size_le, SEEK_SET);
    fread(buffer, 1, file_size, iso_file);

    free(root_dir);
    fclose(iso_file);
    return file_size;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <assembly_file> <iso_file> <program_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *assembly_filename = argv[1];
    const char *iso_filename = argv[2];
    const char *program_path = argv[3];

    int program[MEMORY_SIZE];
    int program_size = assemble_program(assembly_filename, program);
    if (program_size == -1) {
        return EXIT_FAILURE;
    }

    int iso_buffer[MEMORY_SIZE];
    int iso_buffer_size = sizeof(iso_buffer) / sizeof(iso_buffer[0]);

    int file_size = read_iso_file(iso_filename, program_path, iso_buffer, iso_buffer_size);
    if (file_size == -1) {
        return EXIT_FAILURE;
    }

    VM vm;
    vm_init(&vm);

    // Load the assembled program into the VM's memory
    program_size = vm_load_program_from_memory(&vm, program, program_size);
    if (program_size == -1) {
        return EXIT_FAILURE;
    }

    // Load the program from ISO file into the VM's memory
    int iso_program_size = file_size / sizeof(int);
    vm_load_program_from_memory(&vm, iso_buffer, iso_program_size);

    // Execute the program
    vm_execute(&vm);

    printf("Memory at address 10: %d\n", vm.memory[10]); // Should print 8

    return 0;
}