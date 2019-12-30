	.file	1 "branch.c"
	.section .mdebug.abi64
	.previous
	.nan	legacy
	.gnu_attribute 4, 1
	.abicalls
	.globl	result
	.section	.bss,"aw",@nobits
	.align	2
	.type	result, @object
	.size	result, 4
result:
	.space	4
	.text
	.align	2
	.globl	work_4
	.set	nomips16
	.set	nomicromips
	.ent	work_4
	.type	work_4, @function
work_4:
	.frame	$fp,48,$31		# vars= 16, regs= 3/0, args= 0, gp= 0
	.mask	0xd0000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	daddiu	$sp,$sp,-48
	gssq	$31,$fp,32($sp)
	sd	$28,24($sp)
	move	$fp,$sp
	lui	$28,%hi(%neg(%gp_rel(work_4)))
	daddu	$28,$28,$25
	daddiu	$28,$28,%lo(%neg(%gp_rel(work_4)))
	ld	$2,%got_disp(result)($28)
	lw	$2,0($2)
	addiu	$2,$2,1
	move	$3,$2
	ld	$2,%got_disp(result)($28)
	sw	$3,0($2)
	sw	$0,0($fp)
	sw	$0,4($fp)
	lw	$2,0($fp)
	bne	$2,$0,.L2
	nop

	ld	$2,%got_disp(work_3)($28)
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,work_3
1:	jalr	$25
	nop

	b	.L3
	nop

.L2:
	lw	$2,4($fp)
	addiu	$2,$2,-1
	sw	$2,4($fp)
.L3:
	ld	$2,%got_disp(result)($28)
	lw	$2,0($2)
	move	$sp,$fp
	gslq	$31,$fp,32($sp)
	ld	$28,24($sp)
	daddiu	$sp,$sp,48
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	work_4
	.size	work_4, .-work_4
	.align	2
	.globl	work_1
	.set	nomips16
	.set	nomicromips
	.ent	work_1
	.type	work_1, @function
work_1:
	.frame	$fp,16,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	daddiu	$sp,$sp,-16
	sd	$fp,8($sp)
	move	$fp,$sp
	nop
	move	$sp,$fp
	ld	$fp,8($sp)
	daddiu	$sp,$sp,16
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	work_1
	.size	work_1, .-work_1
	.align	2
	.globl	work_2
	.set	nomips16
	.set	nomicromips
	.ent	work_2
	.type	work_2, @function
work_2:
	.frame	$fp,48,$31		# vars= 16, regs= 3/0, args= 0, gp= 0
	.mask	0xd0000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	daddiu	$sp,$sp,-48
	gssq	$31,$fp,32($sp)
	sd	$28,24($sp)
	move	$fp,$sp
	lui	$28,%hi(%neg(%gp_rel(work_2)))
	daddu	$28,$28,$25
	daddiu	$28,$28,%lo(%neg(%gp_rel(work_2)))
	ld	$2,%got_disp(result)($28)
	lw	$2,0($2)
	addiu	$2,$2,1
	move	$3,$2
	ld	$2,%got_disp(result)($28)
	sw	$3,0($2)
	sw	$0,0($fp)
	sw	$0,4($fp)
	lw	$2,0($fp)
	bne	$2,$0,.L8
	nop

	ld	$2,%got_disp(work_1)($28)
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,work_1
1:	jalr	$25
	nop

	b	.L9
	nop

.L8:
	lw	$2,4($fp)
	addiu	$2,$2,-1
	sw	$2,4($fp)
.L9:
	ld	$2,%got_disp(result)($28)
	lw	$2,0($2)
	move	$sp,$fp
	gslq	$31,$fp,32($sp)
	ld	$28,24($sp)
	daddiu	$sp,$sp,48
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	work_2
	.size	work_2, .-work_2
	.align	2
	.globl	work_3
	.set	nomips16
	.set	nomicromips
	.ent	work_3
	.type	work_3, @function
work_3:
	.frame	$fp,48,$31		# vars= 16, regs= 3/0, args= 0, gp= 0
	.mask	0xd0000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	daddiu	$sp,$sp,-48
	gssq	$31,$fp,32($sp)
	sd	$28,24($sp)
	move	$fp,$sp
	lui	$28,%hi(%neg(%gp_rel(work_3)))
	daddu	$28,$28,$25
	daddiu	$28,$28,%lo(%neg(%gp_rel(work_3)))
	ld	$2,%got_disp(result)($28)
	lw	$2,0($2)
	addiu	$2,$2,1
	move	$3,$2
	ld	$2,%got_disp(result)($28)
	sw	$3,0($2)
	sw	$0,0($fp)
	sw	$0,4($fp)
	lw	$2,0($fp)
	bne	$2,$0,.L12
	nop

	ld	$2,%got_disp(work_2)($28)
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,work_2
1:	jalr	$25
	nop

	b	.L13
	nop

.L12:
	lw	$2,4($fp)
	addiu	$2,$2,-1
	sw	$2,4($fp)
.L13:
	ld	$2,%got_disp(result)($28)
	lw	$2,0($2)
	move	$sp,$fp
	gslq	$31,$fp,32($sp)
	ld	$28,24($sp)
	daddiu	$sp,$sp,48
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	work_3
	.size	work_3, .-work_3
	.align	2
	.globl	main
	.set	nomips16
	.set	nomicromips
	.ent	main
	.type	main, @function
main:
	.frame	$fp,48,$31		# vars= 16, regs= 3/0, args= 0, gp= 0
	.mask	0xd0000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	daddiu	$sp,$sp,-48
	gssq	$31,$fp,32($sp)
	sd	$28,24($sp)
	move	$fp,$sp
	lui	$28,%hi(%neg(%gp_rel(main)))
	daddu	$28,$28,$25
	daddiu	$28,$28,%lo(%neg(%gp_rel(main)))
	sw	$0,0($fp)
	sw	$0,0($fp)
	b	.L16
	nop

.L17:
	ld	$2,%got_disp(work_4)($28)
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,work_4
1:	jalr	$25
	nop

	lw	$2,0($fp)
	addiu	$2,$2,1
	sw	$2,0($fp)
.L16:
	lw	$3,0($fp)
	li	$2,50000			# 0xc350
	slt	$2,$3,$2
	bne	$2,$0,.L17
	nop

	move	$2,$0
	move	$sp,$fp
	gslq	$31,$fp,32($sp)
	ld	$28,24($sp)
	daddiu	$sp,$sp,48
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	main
	.size	main, .-main
	.ident	"GCC: (GNU) 4.9.4 20160726 (Red Hat 4.9.4-14)"
