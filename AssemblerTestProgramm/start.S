.section .text
.global _start
_start:
	lui x2, 0x3FF ;
	#x2: 3FF000
	#                    Arithmetische Befehle
	addi x0, x2, 122
	#x0 = 0
	addi x3, x0, -4
	#x3 = FFFFFFFC
	addi x4, x3, 0x41
	#x4 = 3D
	add x5, x3, x4
	#x5 = 39
	sub x6, x4, x5
	#x6 = 4
	sub x3, x3, x6
	# x3 = FFFFFFF8
	#                    Logische Befehle
	ori x7, x0, 0x43A
	ori x8, x0, 0x339
	or x9, x7, x8
	#x9 = 73B
	and x10, x8, x7
	#x10 = 38
	xor x11, x8, x7
	#x11 = 703
	#                    Shifts
	addi x12, x0, 4
	slli x12, x12, 6
	#x12 = 100
	srli x13, x12, 3
	#x13 = 20
	srli x14, x3, 2
	#x14 = 3FFFFFFE
	srai x15, x3, 2
	#x15 = FFFFFFFE
	sll x16, x12, x6
	#x16 = 1000
	srl x17, x12, x6
	#x17 = 10
	addi x18, x0, -122
	sra x18, x18, x12
	#x18 = FFFFFF86
	#                   Data Transfer
	sw x18, 4(x0)
	sh x11, 16(x0)
	lh x19, 4(x0)
	#x19 = FFFFFF86
	lhu x20, 4(x0)
	#x20 = FF86
	lb x21, 4(x0)
	#x21 = FFFFFF86
	lbu x22, 4(x0)
	#x22 = 86
	lh x23, 16(x0)
	#x23 =703
	lb x24, 16(x0)
	#x24 = 3
	lui x25, 5
	#x25 = 5000
	addi x26, x0,  'H'
	sb x26, 0(x25)
	addi x26, x0, 'U'
	sb x26, 0(x25)
	addi x26, x0, '\n'
	sb x26, 0(x25)
	# Es sollte "HU" auf der Standardausgabe erscheinen
	#                   Verzweigungen
	addi x27, x0, 6
	bne_ruecksprung:
	addi x27, x27, -1
	addi x28, x28, 1
	bne x27, x0, bne_ruecksprung
	#x28 = 6
	addi x27, x0, 6
	blt_ruecksprung:
	addi x27, x27, -1
	addi x29, x29, 1
	blt x0, x27, blt_ruecksprung
	#x29 = 6	
	addi x27, x0, -5
	bge_ruecksprung:
	addi x27, x27, 1
	addi x30, x30, 1
	bge x0, x27, bge_ruecksprung
	#x30 = 6
	addi x27, x0, 0
    	bgeu_ruecksprung:
    	addi x27, x27, -1
    	addi x31, x31, 1
    	bgeu x27, x3, bgeu_ruecksprung
    	#x31 = 9
    	addi x27, x0, 0xe3
    	addi x1, x0, 0
   	bltu_ruecksprung:
    	addi x27, x27, -1
    	addi x1, x1, 1
    	bltu x27, x3, bltu_ruecksprung
    	#x1 = E4
#Regfile Zustand nach der Ausführung:
#x0: 0
#x1: E4
#x2: 3FF000
#x3: FFFFFFF8
#x4: 3D
#x5: 39
#x6: 4
#x7: 43A
#x8: 339
#x9: 73B
#x10: 38
#x11: 703
#x12: 100
#x13: 20
#x14: 3FFFFFFE
#x15: FFFFFFFE
#x16: 1000
#x17: 10
#x18: FFFFFF86
#x19: FFFFFF86
#x20: FF86
#x21: FFFFFF86
#x22: 86
#x23: 703
#x24: 3
#x25: 5000
#x26: A
#x27: FFFFFFFF
#x28: 6
#x29: 6
#x30: 6
#x31: 9

#---------------Wenn die Registerwerte mit ihrer Implementierung übereinstimmen, dann die nächsten beiden Zeilen kommentieren oder löschen um weitere Instruktionen auszuführen.
    end_of_program:
	jal x0, end_of_program
#-----------------------------------------------------------------------------------
    #-----------------------------------ab hier alle Register wieder auf 0
    add x1, x0, x0
    add x2, x0, x0
    add x3, x0, x0
    add x4, x0, x0
    add x5, x0, x0
    add x6, x0, x0
    add x7, x0, x0
    add x8, x0, x0
    add x9, x0, x0
    add x10, x0, x0
    add x11, x0, x0
    add x12, x0, x0
    add x13, x0, x0
    add x14, x0, x0
    add x15, x0, x0
    add x16, x0, x0
    add x17, x0, x0
    add x18, x0, x0
    add x19, x0, x0
    add x20, x0, x0
    add x21, x0, x0
    add x22, x0, x0
    add x23, x0, x0
    add x24, x0, x0
    add x25, x0, x0
    add x26, x0, x0
    add x27, x0, x0
    add x28, x0, x0
    add x29, x0, x0
    add x30, x0, x0
    add x31, x0, x0

	#                          Vergleiche
	addi x3, x0, 5
	addi x4, x0, 6
	slt x5, x3, x4
	#x5 = 1
	slt x6, x4, x3
	#x6 = 0
	lui x3, 0xFFFFF
	#x3 = 0xFFFFF000
	sltu x7, x3, x4
	#x7 =0
	sltu x8, x4, x3
	#x8 = 1
	slt x9, x3, x4
	#x9 = 1
	slt x10, x4, x3
	#x10 = 0
	addi x14, x0, 0x20
	#						   auipc
	auipc x11, 0xAFFE
	#x11 = AFFE188
    funct:
    addi x12, x12, 1
    beq x12, x14, ende
    #							jal
    jal x13, funct
    #x13: 198
    ende:
	#x12 = 20
	jal x1, funct2
    inf:
	addi x31, x31, 1
	jal x0, inf
	funct2:
	lui x25, 5
	#x25 = 5000
	addi x27, x0, 62
	addi x26, x0,  0x3F
	#schreibt @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|} auf die Standardausgabe
	ausgabeRet:
	addi x27, x27, -1
	addi x26, x26, 1
	sb x26, 0(x25)
	bne x27, x0, ausgabeRet
	jalr x0, x1, 0
	#Sprung nach inf anschließend wird x31 mit jeden neuem Zyklus inkrementiert.
	end_of_program_:
	jal end_of_program_

#	Regfile Zustand nach der Ausführung:
#x0: 0
#x1: 19C
#x2: 0
#x3: FFFFF000
#x4: 6
#x5: 1
#x6: 0
#x7: 0
#x8: 1
#x9: 1
#x10: 0
#x11: AFFE188
#x12: 20
#x13: 198
#x14: 20
#x15: 0
#x16: 0
#x17: 0
#x18: 0
#x19: 0
#x20: 0
#x21: 0
#x22: 0
#x23: 0
#x24: 0
#x25: 5000
#x26: 7D
#x27: 0
#x28: 0
#x29: 0
#x30: 0
#x31: C0FA

