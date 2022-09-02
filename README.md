# RISC-V emulator implementation in C

The specifications of the RISC-V instruction set architecture (ISA) can be found [here](https://riscv.org/technical/specifications/). It's a simple ISA, containing just 37 instructions. The `0x5000` address serves the purpose of the memory mapped I/O, which means that a byte written to this address is written to stdout.

## Structure

- `risc-v_emu` is the executable to run
- `main.c` contains all the source code
- `main_debugging.c` is the source code with printing statements for debugging (which are easily "turned on/off" by commenting out the respective `trace` macro)
- `/AssemblerTestProgramm`, `/Beispielprojekt`, `/ProgrammEins` and `/ProgrammPrimzahlen` contain the binaries needed to run the emulator.

## Running the program

Run the program with the following command from the project root:

```
./risc-v_emu <instructions> <data>
```

where the 1st argument <instructions> is the path to a binary with instructions and the 2nd argument <data> is the path to a binary with data to write to and read from.
