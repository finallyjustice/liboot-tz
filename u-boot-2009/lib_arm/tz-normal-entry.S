
.global _reloc_begin
_reloc_begin:
.global norm_begin
norm_begin:
	mov r0, #0
	ldr r1, =3273
	ldr r2, =0x70000100
	ldr pc, =0x70008000

.global _reloc_end
_reloc_end:
