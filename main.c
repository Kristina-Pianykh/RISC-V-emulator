
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>

#define reg cpu->regfile_
#define pc cpu->pc_
#define data cpu->data_mem_

#define U_immediate ((uint32_t)((inst >> 12) << 12))
#define J_immediate ((uint32_t)((inst&0x80000000) ? 0xFFE00000 | (inst&0x000FF000) | (inst&0x00100000)>>9 | (inst&0x80000000)>>11 | (inst&0x7FE00000)>>20 : (inst&0x000FF000) | (inst&0x00100000)>>9 | (inst&0x80000000)>>11 | (inst&0x7FE00000)>>20))
#define B_immediate ((uint32_t)((inst&0x80000000) ? 0xFFFFE000 | (inst&0xF00)>>7 | (inst&0x7E000000)>>20 | (inst&0x80)<<4 | (inst&0x80000000)>> 19 : (inst&0xF00)>>7 | (inst&0x7E000000)>>20 | (inst&0x80)<<4 | (inst&0x80000000)>> 19))
#define I_immediate ((uint32_t)((inst&0x80000000) ? 0xFFFFF000 | inst >> 20 : inst >> 20))
#define S_immediate ((uint32_t)((inst&0x80000000) ? 0xFFFFF000 | (inst&0xFE000000)>>20 | (inst&0xF80)>>7 : (inst&0xFE000000)>>20 | (inst&0xF80)>>7))

#define NO_INSTR 0
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
		#define ADD 0x00
		#define SUB 0x20
	#define SLL     0x1
	#define SLT     0x2
	#define SLTU    0x3
	#define XOR     0x4
	#define SR      0x5
		#define SRL     0x00
		#define SRA     0x20
	#define OR      0x6
	#define AND     0x7

#define S     0x23
	#define SB      0x0
	#define SH      0x1
	#define SW		0x2

#define L     0x03
	#define LHU     0x5
	#define LB      0x0
	#define LH      0x1
	#define LW      0x2
	#define LBU     0x4
	
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
	cpu->pc_ = 0x00000000;
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
	printf("size of instruction memory: %ld Byte\n\n", sb.st_size);
	instr_mem_size = sb.st_size;
	cpu->instr_mem_ = malloc(instr_mem_size);
	fread(cpu->instr_mem_, sb.st_size, 1, input_file);
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
	printf("read data for data memory: %ld Byte\n\n", sb.st_size);

	cpu->data_mem_ = malloc(cpu->data_mem_size_);
	fread(cpu->data_mem_, sb.st_size, 1, input_file);
	fclose(input_file);
	return;
}

/**
 * Instruction fetch Instruction decode, Execute, Memory access, Write back
 */
uint32_t CPU_execute(CPU *cpu)
{

	uint32_t inst = *(uint32_t *)(cpu->instr_mem_ + (cpu->pc_ & 0xFFFFF));

	uint8_t opcode = inst & 0x7f;
	uint8_t funct3 = (inst >> 12) & 0x7;
	uint8_t funct7 = (inst >> 25) & 0x7f;
	uint32_t rd = (inst >> 7) & 0x1f;
	uint32_t rs1 = (inst >> 15) & 0x1f;
	uint32_t rs2 = (inst >> 20) & 0x1f;
	uint32_t shamt = rs2; // for SLLI, SRLI and SRAI
	uint32_t temp;
	uint32_t addr;

//#define trace printf // leave uncommented for debugging
#define trace(...)

	reg[0] = 0; // x0 is hardwired to 0
	switch (opcode) {
		case LUI:	reg[rd] = U_immediate; pc += 4; break;
		case AUIPC:	reg[rd] = pc + (U_immediate); pc += 4; break;
		case JAL:	reg[rd] = pc + 4; pc += (J_immediate); break;
		case JALR:	temp = reg[rs1]; reg[rd] = pc + 4; pc = temp + (I_immediate); break;

		case I:
			switch (funct3) {
				case ADDI:	reg[rd] = reg[rs1] + (I_immediate); break;
				case SLLI:	reg[rd] = reg[rs1] << shamt; break;
				case SLTI:	reg[rd] = (int32_t)reg[rs1] < (int32_t)(I_immediate); break;
				case SLTIU:	reg[rd] = reg[rs1] < (I_immediate); break;
				case XORI:	reg[rd] = reg[rs1] ^ (I_immediate); break;
				case SRI:
					switch (funct7) {
						case SRLI:	reg[rd] = reg[rs1] >> shamt; break;
						case SRAI: {
							uint32_t result = reg[rs1]; 
							temp = reg[rs1] & 0x80000000; while (shamt > 0) {result = (result >> 1) | temp; shamt--;}
							reg[rd] = result;
							break;
						}
					} break;
				case ORI:	reg[rd] = reg[rs1] | (I_immediate); break;
				case ANDI:  reg[rd] = reg[rs1] & (I_immediate); break;
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
				case SLL:	reg[rd] = reg[rs1] << (reg[rs2] & 0x1f); break;
				case SLT:	reg[rd] = (int32_t)reg[rs1] < (int32_t)reg[rs2]; break;
				case SLTU:	reg[rd] = reg[rs1] < reg[rs2]; break;
				case XOR:	reg[rd] = reg[rs1] ^ reg[rs2]; break;
				case SR:
					switch (funct7) {
						case SRL:	reg[rd] = reg[rs1] >> (reg[rs2] & 0x1f); break;
						case SRA: {
							uint32_t result = reg[rs1];
							shamt = (reg[rs2] & 0x1f); temp = reg[rs1] & 0x80000000; while (shamt > 0) {result = (result >> 1) | temp; shamt--;}
							reg[rd] = result;
							break;
						}
					} break;
				case OR:	reg[rd] = reg[rs1] | reg[rs2]; break;
				case AND:	reg[rd] = reg[rs1] & reg[rs2]; break;
			}
			pc += 4;
			break;
		
		case S:
			addr = (S_immediate) + reg[rs1];
			switch (funct3) {
				case SB:	data[addr] = reg[rs2]; if (addr == 0x5000) putchar(data[addr] & 0x7f); break;
				case SH:	*(uint16_t *)(data + addr) = reg[rs2]; break;
				case SW:	*(uint32_t *)(data + addr) = reg[rs2]; break;
			}
			pc += 4;
			break;

		case L:
			addr = reg[rs1] + (I_immediate);
			switch (funct3) {
				case LB:	reg[rd] = (int8_t)data[addr]; break;
				case LH:	reg[rd] = *(int16_t *)(data + addr); break;
				case LW:	reg[rd] = *(int32_t *)(data + addr); break;
				case LBU:	reg[rd] = data[addr]; break;
				case LHU:	reg[rd] = *(uint16_t *)(data + addr); break;
			}
			pc += 4;
			break;

		case B:
			switch (funct3) {
				case BEQ:	temp = (reg[rs1] == reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BNE:	temp = (reg[rs1] != reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BLT:	temp = ((int32_t)reg[rs1] < (int32_t)reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BGE:	temp = ((int32_t)reg[rs1] >= (int32_t)reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BLTU:	temp = (reg[rs1] < reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BGEU:	temp = (reg[rs1] >= reg[rs2]) ? B_immediate : 4; pc += temp; break;
			} break;

	case NO_INSTR:
		break;
    default:
        fprintf(stderr, "Invalid instruction.");
        return 0;
}
	return inst;
}

int main(int argc, char *argv[])
{
	CPU *cpu_inst;

	cpu_inst = CPU_init(argv[1], argv[2]);
	uint32_t y = 0;
	for (uint32_t i = 0; i < 1000000; i++)
	{
		y = i;
		if (CPU_execute(cpu_inst) == 0)
			break; // no more instructions to execute
		cpu_inst->regfile_[0] = 0;
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
