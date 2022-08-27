
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>

#define reg cpu->regfile_ //37 instructions
#define pc cpu->pc_

#define LUI   0x37
#define AUIPC 0x17
#define JAL   0x6F
#define JALR  0x67

#define I     0x13
    #define ADDI    0x0
	#define SLTI    0x2
    #define SLTIU   0x3
    #define XORI    0x4
    #define ORI     0x6
    #define ANDI    0x7
    #define SLLI    0x1
    #define SRI     0x5
        #define SRLI    0x00
        #define SRAI    0x20

#define R     0x33
	#define ADD_SUB 0x0
		#define ADD 0x0
		#define SUB 0x2
	#define SLL     0x1
	#define SLT     0x2
	#define SLTU    0x3
	#define XOR     0x4
	#define SR      0x5
		#define SRL     0x0
		#define SRA     0x2
	#define OR      0x6
	#define AND     0x7

#define S     0x23
	#define SB      0x0
	#define SH      0x1
	#define SW      0x2

#define L     0x03
	#define LB      0x0
	#define LH      0x1
	#define LW      0x2
	#define LBU     0x4
	#define LHU     0x5

#define B     0x63
	#define BEQ     0x0
	#define BNE     0x1
	#define BLT     0x4
	#define BGE     0x5
	#define BLTU    0x6
	#define BGEU    0x7

typedef struct
{
	size_t data_mem_size_;
	uint32_t regfile_[32];
	uint32_t pc_;
	uint8_t *instr_mem_;
	uint8_t *data_mem_;
} CPU;

void CPU_open_instruction_mem(CPU *cpu, const char *filename);
void CPU_load_data_mem(CPU *cpu, const char *filename);

CPU *CPU_init(const char *path_to_inst_mem, const char *path_to_data_mem)
{
	CPU *cpu = (CPU *)malloc(sizeof(CPU));
	cpu->data_mem_size_ = 0x400000;
	cpu->pc_ = 0x0;
	CPU_open_instruction_mem(cpu, path_to_inst_mem);
	CPU_load_data_mem(cpu, path_to_data_mem);
	return cpu;
}

void CPU_open_instruction_mem(CPU *cpu, const char *filename)
{
	uint32_t instr_mem_size;
	FILE *input_file = fopen(filename, "r");
	if (!input_file)
	{
		printf("no input\n");
		exit(EXIT_FAILURE);
	}
	struct stat sb;
	if (stat(filename, &sb) == -1)
	{
		printf("error stat\n");
		perror("stat");
		exit(EXIT_FAILURE);
	}
	printf("size of instruction memory: %d Byte\n\n", sb.st_size);
	instr_mem_size = sb.st_size;
	cpu->instr_mem_ = malloc(instr_mem_size);
	fread(cpu->instr_mem_, sb.st_size, 1, input_file);
	for (int i = 0; i < 100; i++)
	{
		printf("0x%X ", (int)cpu->instr_mem_[i]);
	}
	fclose(input_file);
	return;
}

void CPU_load_data_mem(CPU *cpu, const char *filename)
{
	FILE *input_file = fopen(filename, "r");
	if (!input_file)
	{
		printf("no input\n");
		exit(EXIT_FAILURE);
	}
	struct stat sb;
	if (stat(filename, &sb) == -1)
	{
		printf("error stat\n");
		perror("stat");
		exit(EXIT_FAILURE);
	}
	printf("read data for data memory: %d Byte\n\n", sb.st_size);

	cpu->data_mem_ = malloc(cpu->data_mem_size_);
	fread(cpu->data_mem_, sb.st_size, 1, input_file);
	fclose(input_file);
	return;
}

/*instruction decoding*/
uint32_t imm_I(uint32_t inst)
{
	// imm[11:0] = inst[31:20]
	return ((int32_t) (inst & 0xfff00000)) >> 20;
}
uint32_t imm_S(uint32_t inst)
{
	// imm[11:5] = inst[31:25], imm[4:0] = inst[11:7]
	return ((int32_t)(inst & 0xfe000000) >> 20) | ((inst >> 7) & 0x1f);
}
int32_t imm_B(uint32_t inst)
{
	// imm[12|10:5|4:1|11] = inst[31|30:25|11:8|7]
	return ((int32_t)(inst & 0x80000000) >> 19)
				| ((inst & 0x80) << 4) // imm[11]
        		| ((inst >> 20) & 0x7e0) // imm[10:5]
        		| ((inst >> 7) & 0x1e);
}
uint32_t imm_U(uint32_t inst)
{
	// imm[31:12] = inst[31:12]
	return (int32_t)(inst & 0xfffff000);
}
uint32_t imm_J(uint32_t inst)
{
	// imm[20|10:1|11|19:12] = inst[31|30:21|20|19:12]
	return (uint32_t)((int32_t)(inst & 0x80000000) >> 11) //imm[20]
        		| (inst & 0xff000) // imm[19:12]
        		| ((inst >> 9) & 0x800) // imm[11]
        		| ((inst >> 20) & 0x7fe); // imm[10:1]
}
uint32_t shamt(uint32_t inst)
{
	// inst[24:20] = imm[4:0]
	return (uint32_t)(imm_I(inst) & 0x1f); //rust: 0x3f
}

/**
 * Instruction fetch Instruction decode, Execute, Memory access, Write back
 */
uint32_t CPU_execute(CPU *cpu)
{

	uint32_t inst = *(uint32_t *)(cpu->instr_mem_ + (cpu->pc_ & 0xFFFFF));

	uint8_t opcode = inst & 0x7f;		  // opcode in bits 6..0
	uint8_t funct3 = (inst >> 12) & 0x7;  // funct3 in bits 14..12
	uint8_t funct7 = (inst >> 25) & 0x7f; // funct7 in bits 31..25
	uint32_t rd = (inst >> 7) & 0x1f; // rd in bits 11..7
	uint32_t rs1 = (inst >> 15) & 0x1f; // rs1 in bits 19..15
	uint32_t rs2 = (inst >> 20) & 0x1f; // rs2 in bits 24..20

	printf("inst: %X\n", inst);
	printf("opcode: %X\n", opcode);
	printf("funct3: %X\n", funct3);
	printf("funct7: %X\n", funct7);
	printf("rd: %X\n", rd);
	printf("rs1: %X\n", rs1);
	printf("rs2: %X\n", rs2);

	reg[0] = 0; // x0 hardwired to 0 at each cycle

	switch (opcode) {
		case LUI:	reg[rd] = imm_U(inst); pc += 4; break;
		case AUIPC:	reg[rd] = pc + imm_U(inst); pc += 4; break;
		case JAL:	reg[rd] = pc + 4; pc += (int32_t)imm_J(inst); break;
		case JALR:	reg[rd] = pc + 4; pc += reg[rs1] + (int32_t)imm_I(inst); break; //why int32_t cast to rs2 only?

		case I:
			switch (funct3) {
				case ADDI:  reg[rd] = reg[rs1] + imm_I(inst); break;
				case SLLI:  reg[rd] = reg[rs1] << imm_I(inst); break;
				case SLTI:  reg[rd] = reg[rs1] < imm_I(inst); break; // signed?
				case SLTIU: reg[rd] = reg[rs1] < imm_I(inst); break;
				case XORI:  reg[rd] = reg[rs1] ^ imm_I(inst); break;
				case SRI:
					switch (funct7) {
						case SRLI:  reg[rd] = reg[rs1] >> imm_I(inst); break;
						case SRAI:  reg[rd] = reg[rs1] >> imm_I(inst); break; //no difference?
					} break;
				case ORI:	reg[rd] = reg[rs1] | imm_I(inst); break;
				case ANDI:  reg[rd] = reg[rs1] & imm_I(inst); break;
			}
			pc += 4;
			break;
		
		case R:
			switch (funct3) {
				case ADD_SUB:
					switch (funct7) {
						case ADD:	reg[rd] = reg[rs1] + reg[rs2]; break;
						case SUB:	reg[rd] = reg[rs1] - reg[rs2]; break;
					} break;
				case SLL:	reg[rd] = reg[rs1] << reg[rs2]; break;
				case SLT:	reg[rd] = (int32_t)reg[rs1] < (int32_t)reg[rs2]; break;
				case SLTU:	reg[rd] = reg[rs1] < reg[rs2]; break;
				case XOR:	reg[rd] = reg[rs1] ^ reg[rs2]; break;
				case SR:
					switch (funct7) {
						case SRL:	reg[rd] = reg[rs1] >> reg[rs2]; break;
						case SRA:	reg[rd] = (int32_t)reg[rs1] >> reg[rs2]; break; //no int3_t casting on rs2?
					} break;
				case OR:	reg[rd] = reg[rs1] | reg[rs2]; break;
				case AND:	reg[rd] = reg[rs1] & reg[rs2]; break;
			}
			pc += 4;
			break;
		
		case S:
			switch (funct3) {
				case SB: // TODO: to implement
				case SH: // TODO: to implement
				case SW: // TODO: to implement
			}
			pc += 4;
			break;

		case L:
			switch (funct3) {
				case LB: // TODO: to implement
				case LH: // TODO: to implement
				case LW: // TODO: to implement
				case LBU: // TODO: to implement
				case LHU: // TODO: to implement
			}
			pc += 4;
			break;

		case B:
			switch (funct3) {
				case BEQ:	pc += (reg[rs1] == reg[rs2]) ? (int32_t)imm_B(inst) : 4; break;
				case BNE:	pc += (reg[rs1] != reg[rs2]) ? (int32_t)imm_B(inst) : 4; break;
				case BLT:	pc += (reg[rs1] < reg[rs2]) ? (int32_t)imm_B(inst) : 4; break;
				case BGE:	pc += (reg[rs1] >= reg[rs2]) ? (int32_t)imm_B(inst) : 4; break;
				case BLTU:	pc += (reg[rs1] < reg[rs2]) ? (int32_t)imm_B(inst) : 4; break; // unsigned BLT?
				case BGEU:	pc += (reg[rs1] >= reg[rs2]) ? (int32_t)imm_B(inst) : 4; break; // unsigned BGE?
			} break;

    default:
        fprintf(stderr,
                "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                , opcode, funct3, funct7);
        return 0;
        /*exit(1);*/
}
	return inst;
}

int main(int argc, char *argv[])
{
	printf("C Praktikum\nHU Risc-V  Emulator 2022\n");

	CPU *cpu_inst;
	const char *instr_path = "instruction_mem.bin";
	const char *data_path = "data_mem.bin";

	cpu_inst = CPU_init(instr_path, data_path);
	// cpu_inst = CPU_init(argv[1], argv[2]);
	for (uint32_t i = 0; i < 1000000; i++)
	{ // run 70000 cycles
		uint32_t inst = CPU_execute(cpu_inst);
		if (inst == 0)
			break; // no more instructions to execute
	}

	printf("\n-----------------------RISC-V program terminate------------------------\nRegfile values:\n");

	// output Regfile
	for (uint32_t i = 0; i <= 31; i++)
	{
		printf("%d: %X\n", i, cpu_inst->regfile_[i]);
	}
	fflush(stdout);

	return 0;
}
