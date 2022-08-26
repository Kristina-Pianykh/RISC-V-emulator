.section .init
.global _start
_start:
	lui x2, 0x3FF
	jal main
	sbreak
	j .

