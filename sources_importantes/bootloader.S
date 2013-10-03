.text
.code 32
.align 4

vectors:
	b _start @ Reset vector
	b . @ Undefined instruction
	b . @ Software interrupt
	b . @ Prefetch abort exception
	b . @ Data abort exception
	b . @ Reserved vector, not used
	b irq_handler @ Normal interrupt
	b . @ Fast interrupt

_start:
        bl _lowlevel_init

	ldr r0, =bss_size
	ldr r1, =_start_bss

	ldr r3, =data_size
	ldr r4, =_start_data

	ldr r5, =data_flash

	mov r2,#0

	b end_loop
loop_bss:
	str r2, [r1]
	add r1, r1, #4
	sub r0, r0, #4
end_loop:
	cmp r0,#0
	bne loop_bss

	b end_loop_data
loop_data:
	str r5, [r4]
	add r4, r4, #4
	add r5, r5, #4
	sub r3, r3, #4
end_loop_data:
	cmp r3,#0
	bne loop_data

	mrs r0, CPSR @ Copie CPSR dans r0
	bic r0, r0, #0x1f @ Met à 0 les 5 bits M
	orr r0, r0, #0x12 @ et change vers le mode IRQ
	msr CPSR_c, r0 @ Recharge les bits de contrôle
	nop @ de CPSR
	ldr sp, =start_stack
	mrs r0, CPSR @ Copie CPSR dans r0
	bic r0, r0, #0x1f @ Met à 0 les 5 bits M
	orr r0, r0, #0x13 @ et change vers le mode superviseur
	msr CPSR_c, r0 @ Recharge les bits de contrôle
	nop @ de CPSR
	ldr sp, =start_stack
	add sp, sp, #256

	b main
.end



