/* last 5 bit of Program Status Register (PSR) */
.equ Mode_USR, 0x10 @ User Mode
.equ Mode_FIQ, 0x11 @ Fast Interrupt Mode
.equ Mode_IRQ, 0x12 @ Interrupt Mode
.equ Mode_SVC, 0x13 @ Supervisor Mode
.equ Mode_ABT, 0x17 @ Abort Mode
.equ Mode_UND, 0x1B @ Undefined Mode
.equ Mode_SYS, 0x1F @ System Mode
.equ Mode_MON, 0x16 @ Monitor Mode

/* 6th and 7th bit of Program Status Register (PSR) */
.equ I_Bit,    0x80 @ IRQ interrupts disabled
.equ F_Bit,    0x40 @ FIQ interrupts disabled

.equ NS_BIT,   0x1


.global init_tz
.align 4
init_tz:


.global tz_monitor
.align 5
tz_monitor:
	@ Monitor
	nop @ Reset      - not used by Monitor
	nop @ Undef      - not used by Monitor
	b   smc_handler
	nop @ Prefetch   - can by used by Monitor
	nop @ Data abort - can by used by Monitor
	nop @ RESERVED
	nop @ IRQ        - can by used by Monitor
	nop @ FIQ        - can by used by Monitor


/* 
 * SMC Handler
 *
 * - Detect which world executed SMC
 * - Saves state to appropriate stack
 * - Restores other worlds state
 * - Switches world
 * - Performs exception return
 */

.global smc_handler
smc_handler:
	push {r0-r3}							@ R0-r3 contain args to be passed between worlds
											@ Temporarily stack, so can be used as scratch regs

	@ Which world have we come from
	@ ------------------------------
	mrc p15, 0, r0, c1, c1, 0               @ Read Secure Configuration Register data
	tst r0, #NS_BIT                         @ Is the NS bit set?
	eor r0, r0, #NS_BIT                     @ Toggle NS bit
	mcr p15, 0, r0, c1, c1, 0               @ Write Secure Configuration Register data

	@ Load save to pointer
	@ ---------------------
	ldreq  r0, =S_STACK_SP                  @ If NS bit set, was in Normal world.  So restore Secure state
	ldrne  r0, =NS_STACK_SP
	ldr    r2, [r0]

	@ Load restore from pointer
	@ --------------------------
	ldreq  r1, =NS_STACK_SP
	ldrne  r1, =S_STACK_SP
	ldr    r3, [r1]

	@ r2  <-- save to
	@ r3  <-- restore from

	@ Save general purpose registers, SPSR and LR
	@ --------------------------------------------
	stmfd   r2!, {r4-r12}                   @ Save r4 to r12
	@ ADD SUPPORT FOR SPs
	mrs     r4, spsr                        @ Also get a copy of the SPSR
	stmfd   r2!, {r4, lr}                   @ Save original SPSR and LR
	
	str     r2, [r0]                        @ Save updated pointer back, r0 and r2 now free
	
	@ Restore other world registers, SPSR and LR
	@ ---------------------------------------------
	ldmfd   r3!, {r0, lr}                   @ Get SPSR and LR from
	@ ADD SUPPORT FOR SPs
	msr     spsr_cxsf, r0                   @ Restore SPSR
	ldmfd   r3!, {r4-r12}                   @ Restore registers r4 to r12

	str r3, [r1]                            @ Save updated pointer back, r1 and r3 now free

	@ Clear local monitor
	@ --------------------
	clrex                                   @ Not strictly required in this example, as not using LDR EX/STREX
											@ However, architecturally should execute CLREX on a context switch

	@ Now restore args (r0-r3)
	@ -------------------------
	pop {r0-r3}

	
	@ Perform exception return
	@ -------------------------
	movs pc, lr

/*
 * Monitor Initialization
 *
 * This is called the first time the Secure world wishes to
 * transit to the Normal world.
 */

.global init_secure_monitor
init_secure_monitor:
	@ Install Secure Monitor
	@ -----------------------
	ldr r1, =ns_image                   @ R1 is used
	str r0, [r1]
	ldr r0, =tz_monitor                    @ Get address of Monitors vector table
	mcr p15, 0, r0, c12, c0, 1          @ Write Monitor Vector Base Address Register

	@ Save Secure state
	@ ------------------
	ldr    r0, =S_STACK_LIMIT           @ Get address of Secure state stack
	stmfd  r0!, {r4-r12}                @ Save general purpose registers
	@ ADD support for SPs
	mrs    r1, cpsr                     @ Also get a copy of the CPSR
	stmfd  r0!, {r1, lr}                @ Save CPSR and LR

	@ Switch to Monitor mode
	@ -----------------------
	cps    #Mode_MON                    @ Move to Monitor mode after saving Secure state

	@ Save Secure state stack pointer
	@ --------------------------------
	ldr r1, =S_STACK_SP                 @ Get address of global
	str r0, [r1]                        @ Save pointer


	@ Set up initial NS state stack pointer
	@ --------------------------------------
	ldr r0, =NS_STACK_SP                @ Get address of global
	ldr r1, =NS_STACK_LIMIT             @ Get top of Normal state stack (assuming FD model)
	str r1, [r0]                        @ Save pointer


	@ Set up exception return information
	@ ------------------------------------
	@IMPORT  ns_image

	ldr lr, ns_image                    @ ns_image
	msr spsr_cxsf, #Mode_SVC            @ Set SPSR to be SVC mode

	@ Switch to Normal world
	@ -----------------------
	mrc p15, 0, r4, c1, c1, 0           @ Read Secure Configuration Register data
	orr r4, #NS_BIT                     @ Set NS bit
	mcr p15, 0, r4, c1, c1, 0           @ Write Secure Configuration Register data

	@ Clear general purpose registers
	@ --------------------------------
	mov r0,  #0
	mov r1,  #0
	mov r2,  #0
	mov r3,  #0
	mov r4,  #0
	mov r5,  #0
	mov r6,  #0
	mov r7,  #0
	mov r8,  #0
	mov r9,  #0
	mov r10, #0
	mov r11, #0
	mov r12, #0

	movs pc, lr

@ ------------------------------------------------------------
@ Space reserved for secure and non-secure stacks
@ ------------------------------------------------------------
NS_STACK_BASE:
	.word     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
NS_STACK_LIMIT:

S_STACK_BASE:
	.word     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
S_STACK_LIMIT:

NS_STACK_SP:
	.word     0

S_STACK_SP:
	.word     0
	
ns_image :
	.word     0

@.end
@ ------------------------------------------------------------
@ End
@ ------------------------------------------------------------
