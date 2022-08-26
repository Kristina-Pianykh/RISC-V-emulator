ENTRY(_start)
MEMORY
{
    iram (rx) : ORIGIN = 0x80000000, LENGTH = 0x7000
    dram (rw): ORIGIN = 0x00000000, LENGTH = 0x4000
}
SECTIONS
{
    .init : {*(.init*) } > iram
    .text : { *(.text*) } > iram
    .rodata : { *(.rodata*) } > dram
    .data : { *(.data*) } > dram
    .bss : { *(.sbss*) } > dram
}
__stack_top = 0x3FF;
