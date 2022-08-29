
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>

#define reg cpu->regfile_ //37 instructions
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
	cpu->pc_ = 0x80000000;
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

	uint8_t opcode = inst & 0x7f;		  // opcode in bits 6..0
	uint8_t funct3 = (inst >> 12) & 0x7;  // funct3 in bits 14..12
	uint8_t funct7 = (inst >> 25) & 0x7f; // funct7 in bits 31..25
	uint32_t rd = (inst >> 7) & 0x1f; // rd in bits 11..7
	uint32_t rs1 = (inst >> 15) & 0x1f; // rs1 in bits 19..15
	uint32_t rs2 = (inst >> 20) & 0x1f; // rs2 in bits 24..20
	uint32_t shamt = rs2; // for SLLI, SRLI and SRAI
	uint32_t temp;
	uint32_t addr;

//#define trace printf
#define trace(...)

	reg[0] = 0; // x0 hardwired to 0 at each cycle
	trace("\npc: %X\n", pc);
	trace("instr: %X\n", inst);
	trace("opcode: %X\n", opcode);
	trace("rd:%d rs1:%d rs2:%d U_immediate:0x%X J_immediate:0x%X B_immediate:0x%X I_immediate:0x%X S_immediate:0x%X funct3:0x%X funct7:0x%X\n",rd,rs1,rs2,U_immediate,J_immediate,B_immediate,I_immediate,S_immediate,funct3,funct7);
	// if (pc == 0x80002164)
	// 	trace("breakpoint\n");
	switch (opcode) {
		case LUI:	
			reg[rd] = U_immediate;
			pc += 4;
			trace("%s: reg[%u] = %X\n", "LUI", rd, U_immediate);
			break;
		case AUIPC:
			trace("%s: reg[%u] = %u + %X = %X\n", "AUIPC", pc, U_immediate, pc + (U_immediate));
			reg[rd] = pc + (U_immediate); pc += 4;
			break;
		case JAL:
			trace("%s: reg[%u] = %u + 4 = %X, pc = %X + %X = %X\n", "JAL", rd, pc, pc + 4, pc, J_immediate, (J_immediate) + pc);
			reg[rd] = pc + 4; pc += (J_immediate);
			break;
		case JALR:
			trace("%s: reg[%u] = %u + 4 = %X, pc = reg[%u] + %X = %X\n", "JALR", rd, pc, pc + 4, rs1, I_immediate, pc + (reg[rs1] + (I_immediate)) & 0xfffffffe);
			temp = reg[rs1];
			reg[rd] = pc + 4; pc = temp + (I_immediate);
			break;

		case I:
			switch (funct3) {
				case ADDI:
					trace("%s: reg[%u] = reg[%u] + %X = %X\n", "ADDI", rd, rs1, (I_immediate), reg[rs1] + (I_immediate));
					reg[rd] = reg[rs1] + (I_immediate);
					break;
				case SLLI:
					trace("%s: reg[%u] = reg[%u] << %X = %X\n", "SLLI", rd, rs1, shamt, reg[rs1] << shamt);
					reg[rd] = reg[rs1] << shamt;
					break;
				case SLTI:
					trace("%s: reg[%u] = reg[%u] < %X = %d\n", "SLTI", (int32_t)rs1, (int32_t)(I_immediate), (int32_t)reg[rs1] < (int32_t)(I_immediate));
					reg[rd] = (int32_t)reg[rs1] < (int32_t)(I_immediate);
					break;
				case SLTIU:
					trace("%s: reg[%u] = reg[%u] < %X = %d\n", "SLTIU", rd, rs1, I_immediate, reg[rs1] < (I_immediate));
					reg[rd] = reg[rs1] < (I_immediate);
					break;
				case XORI:
					trace("%s: reg[%u] = reg[%u] ^ %X = %d\n", "XORI", rs1, I_immediate, reg[rs1] ^ (I_immediate));
					reg[rd] = reg[rs1] ^ (I_immediate);
					break;
				case SRI:
					switch (funct7) {
						case SRLI:
							trace("%s: reg[%u] = reg[%u] >> %X = %X\n", "SRLI", rd, rs1, shamt, reg[rs1] >> shamt);
							reg[rd] = reg[rs1] >> shamt;
							break;
						case SRAI: {
							uint32_t result = reg[rs1]; 
							temp = reg[rs1] & 0x80000000; while (shamt > 0) {result = (result >> 1) | temp; shamt--;}
							reg[rd] = result;
							trace("%s: reg[%u] = reg[%u] >> %X = %X\n", "SRAI", rs1, shamt, reg[rd]);
							break;
						}
					} break;
				case ORI:	reg[rd] = reg[rs1] | (I_immediate);
					trace("%s: reg[%u] = reg[%u] | %X = %X\n", "ORI", rs1, I_immediate, reg[rd]);
					break;
				case ANDI:  reg[rd] = reg[rs1] & (I_immediate);
					trace("%s: reg[%u] = reg[%u] & %X = %X\n", "ANDI", rs1, I_immediate, reg[rd]);
					break;
			}
			pc += 4;
			break;
		
		case R:
			switch (funct3) {
				case ADD_SUB:
					switch (funct7) {
						case ADD:	reg[rd] = reg[rs1] + reg[rs2];
							trace("%s: reg[%u] = reg[%u] + reg[%u] = %X\n", "AND", rd, rs1, rs2, reg[rd]);
							break;
						case SUB:	reg[rd] = reg[rs1] - reg[rs2];
							trace("%s: reg[%u] = reg[%u] - reg[%u] = %X\n", "SUB", rd, rs1, rs2, reg[rd]);
							break;
					} break;
				case SLL:
					trace("reg[%u] = %X\n", rs2, reg[rs2]);
					trace("reg[%u](5 bits) = %X\n", rs2, reg[rs2] & 0x1f);
					reg[rd] = reg[rs1] << (reg[rs2] & 0x1f); // should be the lowest 5 bits of rs2 register
					trace("%s: reg[%u] = reg[%u] << reg[%u](5 bits) = %X\n", "SLL", rd, rs1, rs2, reg[rd]);
					break;
				case SLT:	reg[rd] = (int32_t)reg[rs1] < (int32_t)reg[rs2];
					trace("%s: reg[%u] = reg[%u] < reg[%u] = %X\n", "SLT", rd, rs1, rs2, reg[rd]);
					break;
				case SLTU:	reg[rd] = reg[rs1] < reg[rs2];
					trace("%s: reg[%u] = reg[%u] < reg[%u] = %X\n", "SLTU", rd, rs1, rs2, reg[rd]);
					break;
				case XOR:	reg[rd] = reg[rs1] ^ reg[rs2];
					trace("%s: reg[%u] = reg[%u] ^ reg[%u] =%X\n", "XOR", rd, rs1, rs2, reg[rd]);
					break;
				case SR:
					switch (funct7) {
						case SRL:	reg[rd] = reg[rs1] >> (reg[rs2] & 0x1f);
							trace("reg[%u] = %X\n", rs2, reg[rs2]);
							trace("reg[%u](5 bits) = %X\n", rs2, reg[rs2] & 0x1f);
							trace("%s: reg[%u] = reg[%u] >> reg[%u] = %X\n", "SRL", rd, rs1, rs2, reg[rd]);
							break;
						case SRA: {
							uint32_t result = reg[rs1];
							shamt = (reg[rs2] & 0x1f); temp = reg[rs1] & 0x80000000; while (shamt > 0) {result = (result >> 1) | temp; shamt--;}
							reg[rd] = result;
							trace("%s: reg[%u] = reg[%u] >> reg[%u] = %X\n", "SRA", rd, rs1, rs2, reg[rd]);
							break;
						}
					} break;
				case OR:	reg[rd] = reg[rs1] | reg[rs2];
					trace("%s: reg[%u] = reg[%u] | reg[%u] = %X\n", "OR", rd, rs1, rs2, reg[rd]);
					break;
				case AND:	reg[rd] = reg[rs1] & reg[rs2];
					trace("%s: reg[%u] = reg[%u] & reg[%u] = %X\n", "SRL", rd, rs1, rs2, reg[rd]);
					break;
			}
			pc += 4;
			break;
		
		case S:
			addr = (S_immediate) + reg[rs1];
			switch (funct3) {
				case SB:
					trace("%s: data[reg[%u] + %X] = reg[%u]\n", "SB", rs1, S_immediate, rs2);
					trace("%s: data[%X] = %X\n", "SB", addr, (data[addr] & 0xffffff00) | (reg[rs2] & 0xff));
					trace("byte to write: %X\n", (reg[rs2] & 0xff));
					trace("byte to write converted: %X\n", ((uint8_t) reg[rs2] & 0xff));
					data[addr] = reg[rs2];
					if (addr == 0x5000) putchar(data[addr] & 0x7f);
					break;
				case SH:
					trace("%s: data[reg[%u] + %X] = reg[%u]\n", "SH", rs1, S_immediate, rs2);
					trace("%s: data[%X] =  %X\n", "SH",addr, (data[addr] & 0xffff0000) | (reg[rs2] & 0xffff));
					*(uint16_t *)(data + addr) = reg[rs2];
					break;
				case SW:
					trace("%s: data[reg[%u] + %X] = reg[%u]\n", "SW", rs1, S_immediate, rs2);
					trace("%s: data[%X] = %X\n", "SW", addr, reg[rs2]);
					*(uint32_t *)(data + addr) = reg[rs2];
					break;
			}
			pc += 4;
			break;

		case L:
			addr = reg[rs1] + (I_immediate);
			switch (funct3) {
				case LB:
					trace("%s: reg[%d] = data[reg[%d] + %X]\n", "LB", rd, rs1, (I_immediate));
					trace("offset: %X\n", "LB", rd, rs1, (I_immediate));
					trace("reg[%d] + (I_immediate) = %X (all 32 bits here, no sign extension)\n", rd, addr);
					reg[rd] = (int8_t)data[addr]; break;
				case LH:
					trace("%s: reg[%d] = data[reg[%d] + %X]\n", "LH", rd, rs1, (I_immediate));
					trace("reg[%d] + (I_immediate) = %X (all 32 bits here, no sign extension)\n", rd, addr);
					reg[rd] = *(int16_t *)(data + addr); break;
				case LW:
					trace("%s: reg[%d] = data[reg[%d] + %X]\n", "LW", rd, rs1, (I_immediate));
					trace("reg[%d] + (I_immediate) = %X (all 32 bits here, no sign extension)\n", rs1, addr);
					reg[rd] = *(int32_t *)(data + addr); break;
				case LBU:
					trace("%s: reg[%d] = data[reg[%d] + %X]\n", "LBU", rd, rs1, (I_immediate));
					trace("reg[%d] + (I_immediate) = %X (all 32 bits here, no sign extension)\n", rd, addr);
					reg[rd] = data[addr]; break;
				case LHU:
					trace("%s: reg[%d] = data[reg[%d] + %X]\n", "LHU", rd, rs1, (I_immediate));
					trace("reg[%d] + (I_immediate) = %X (all 32 bits here, no sign extension)\n", rd, addr);
					reg[rd] = *(uint16_t *)(data + addr); break;
			}
			pc += 4;
			break;

		case B:
			switch (funct3) {
				case BEQ:
					trace("%s: reg[%d] = data[reg[%d] + %X]\n", "LHU", rd, rs1, (I_immediate));
					temp = (reg[rs1] == reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BNE:	temp = (reg[rs1] != reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BLT:	temp = ((int32_t)reg[rs1] < (int32_t)reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BGE:	temp = ((int32_t)reg[rs1] >= (int32_t)reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BLTU:	temp = (reg[rs1] < reg[rs2]) ? B_immediate : 4; pc += temp; break;
				case BGEU:	temp = (reg[rs1] >= reg[rs2]) ? B_immediate : 4; pc += temp; break;
			} trace("%s\n", "branch"); break;

	case NO_INSTR:
		break;
    default:
        fprintf(stderr,
                "[-] Invalid instruction: opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
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
	// const char *instr_path = "instruction_mem.bin";
	// const char *data_path = "data_mem.bin";

	// cpu_inst = CPU_init(instr_path, data_path);
	cpu_inst = CPU_init(argv[1], argv[2]);
	uint32_t y = 0;
	for (uint32_t i = 0; i < 1000000; i++)
	{ // run 70000 cycles
		y = i;
		if (CPU_execute(cpu_inst) == 0)
			break; // no more instructions to execute
		cpu_inst->regfile_[0] = 0;
		// printf("\n------------------------------------Regfile values:------------------------------------\n");
		// for (int i=0; i<32; i++) {
		// 	printf("%d:%X ", i, cpu_inst->regfile_[i]); 
		// } printf("\n");
		// if ((cpu_inst->pc_ == 0x0000267) | (cpu_inst->pc_ == 0x0000264))
		// 	printf("\n\n");
	}
	printf("%d iterations\n", y);

	printf("\n-----------------------RISC-V program terminate------------------------\nRegfile values:\n");

	// output Regfile
	for (uint32_t i = 0; i <= 31; i++)
	{
		printf("%d: %X\n", i, cpu_inst->regfile_[i]);
	}
	fflush(stdout);

	return 0;
}
