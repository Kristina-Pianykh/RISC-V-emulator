.PHONY: all clean


%.bin: %.elf
	riscv32-unknown-elf-objcopy -O binary -j .init -j .text build/$< build/instruction_mem.bin
	riscv32-unknown-elf-objcopy -O binary -j .rodata -j .data -j .sdata -j sbss build/$< build/data_mem.bin

OBJECTS := build/start.o

all: rv_assembler.elf rv_assembler.bin

rv_assembler.elf: $(OBJECTS)
	riscv32-unknown-elf-gcc -o build/rv_assembler.elf -v -march=rv32i -nostartfiles -Tlinker_script.ld -Wl,--Map,build/rv_assembler.map $(OBJECTS)
	riscv32-unknown-elf-size build/rv_assembler.elf

clean:
	-$(RM) $(OBJECTS)
	-$(RM) build/rv_assembler.elf build/r.bin build/rv_assembler.map build/instruction_mem.bin build/data_mem.bin

build/start.o: start.S
	riscv32-unknown-elf-gcc -c -o $@ -march=rv32i -Wall -O0  -ffreestanding -fno-builtin -std=gnu99 -Wall -Werror=implicit-function-declaration $<
