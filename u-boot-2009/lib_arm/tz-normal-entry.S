
.global _reloc_begin
_reloc_begin:
.global norm_begin
norm_begin:
	@ Switch to Normal world
	@ -----------------------
#mrc p15, 0, r4, c1, c1, 0           @ Read Secure Configuration Register data
	#orr r4, #NS_BIT                     @ Set NS bit
#orr r4, #0x1                        @ Set NS bit
#mcr p15, 0, r4, c1, c1, 0           @ Write Secure Configuration Register data

	@ Clear general purpose registers
	@ --------------------------------
#mov r0,  #0
#mov r1,  #0
#mov r2,  #0
#mov r3,  #0
#mov r4,  #0
#mov r5,  #0
#mov r6,  #0
#mov r7,  #0
#mov r8,  #0
#mov r9,  #0
#mov r10, #0
#mov r11, #0
#mov r12, #0	

	mov r0, #0
	ldr r1, =3273
	ldr r2, =0x70000100
	ldr pc, =0x70008000

.global _reloc_end
_reloc_end:
