
.global reloc_begin
reloc_begin:
.global norm_begin
norm_begin:
	mov r0, #0
	ldr r1, =3273
#ldr r1, =3000
#ldr r2, =0x70000100
	ldr pc, =0x70008000
	ldr pc, =0x80008000
.global reloc_end
reloc_end:
