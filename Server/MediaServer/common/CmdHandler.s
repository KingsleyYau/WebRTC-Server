	.file	"CmdHandler.cpp"
	.section	.text._ZN9KRunnableD2Ev,"axG",@progbits,_ZN9KRunnableD5Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN9KRunnableD2Ev
	.type	_ZN9KRunnableD2Ev, @function
_ZN9KRunnableD2Ev:
.LFB372:
	.cfi_startproc
	movq	$_ZTV9KRunnable+16, (%rdi)
	ret
	.cfi_endproc
.LFE372:
	.size	_ZN9KRunnableD2Ev, .-_ZN9KRunnableD2Ev
	.weak	_ZN9KRunnableD1Ev
	.set	_ZN9KRunnableD1Ev,_ZN9KRunnableD2Ev
	.section	.text._ZN11CmdRunnableD2Ev,"axG",@progbits,_ZN11CmdRunnableD5Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN11CmdRunnableD2Ev
	.type	_ZN11CmdRunnableD2Ev, @function
_ZN11CmdRunnableD2Ev:
.LFB1845:
	.cfi_startproc
	movq	$0, 8(%rdi)
	movq	$_ZTV9KRunnable+16, (%rdi)
	ret
	.cfi_endproc
.LFE1845:
	.size	_ZN11CmdRunnableD2Ev, .-_ZN11CmdRunnableD2Ev
	.weak	_ZN11CmdRunnableD1Ev
	.set	_ZN11CmdRunnableD1Ev,_ZN11CmdRunnableD2Ev
	.section	.text._ZN9KRunnableD0Ev,"axG",@progbits,_ZN9KRunnableD0Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN9KRunnableD0Ev
	.type	_ZN9KRunnableD0Ev, @function
_ZN9KRunnableD0Ev:
.LFB374:
	.cfi_startproc
	movq	$_ZTV9KRunnable+16, (%rdi)
	jmp	_ZdlPv
	.cfi_endproc
.LFE374:
	.size	_ZN9KRunnableD0Ev, .-_ZN9KRunnableD0Ev
	.section	.text._ZN11CmdRunnableD0Ev,"axG",@progbits,_ZN11CmdRunnableD0Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN11CmdRunnableD0Ev
	.type	_ZN11CmdRunnableD0Ev, @function
_ZN11CmdRunnableD0Ev:
.LFB1847:
	.cfi_startproc
	movq	$0, 8(%rdi)
	movq	$_ZTV9KRunnable+16, (%rdi)
	jmp	_ZdlPv
	.cfi_endproc
.LFE1847:
	.size	_ZN11CmdRunnableD0Ev, .-_ZN11CmdRunnableD0Ev
	.section	.text._ZN9KSafeListIP7CmdItemED2Ev,"axG",@progbits,_ZN9KSafeListIP7CmdItemED5Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN9KSafeListIP7CmdItemED2Ev
	.type	_ZN9KSafeListIP7CmdItemED2Ev, @function
_ZN9KSafeListIP7CmdItemED2Ev:
.LFB1900:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rdi, %rbp
	leaq	24(%rdi), %rdi
	addq	$8, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	movq	$_ZTV9KSafeListIP7CmdItemE+16, -24(%rdi)
	call	pthread_rwlock_destroy
	movq	0(%rbp), %rdi
	cmpq	%rbp, %rdi
	jne	.L9
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L12:
	movq	%rbx, %rdi
.L9:
	movq	(%rdi), %rbx
	call	_ZdlPv
	cmpq	%rbp, %rbx
	jne	.L12
.L5:
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1900:
	.size	_ZN9KSafeListIP7CmdItemED2Ev, .-_ZN9KSafeListIP7CmdItemED2Ev
	.weak	_ZN9KSafeListIP7CmdItemED1Ev
	.set	_ZN9KSafeListIP7CmdItemED1Ev,_ZN9KSafeListIP7CmdItemED2Ev
	.section	.text._ZN9KSafeListIP7CmdItemED0Ev,"axG",@progbits,_ZN9KSafeListIP7CmdItemED0Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN9KSafeListIP7CmdItemED0Ev
	.type	_ZN9KSafeListIP7CmdItemED0Ev, @function
_ZN9KSafeListIP7CmdItemED0Ev:
.LFB1902:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movq	%rdi, %r12
	leaq	24(%rdi), %rdi
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	leaq	8(%r12), %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	$_ZTV9KSafeListIP7CmdItemE+16, -24(%rdi)
	call	pthread_rwlock_destroy
	movq	8(%r12), %rdi
	cmpq	%rbp, %rdi
	jne	.L17
	jmp	.L15
	.p2align 4,,10
	.p2align 3
.L19:
	movq	%rbx, %rdi
.L17:
	movq	(%rdi), %rbx
	call	_ZdlPv
	cmpq	%rbp, %rbx
	jne	.L19
.L15:
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	movq	%r12, %rdi
	popq	%r12
	.cfi_def_cfa_offset 8
	jmp	_ZdlPv
	.cfi_endproc
.LFE1902:
	.size	_ZN9KSafeListIP7CmdItemED0Ev, .-_ZN9KSafeListIP7CmdItemED0Ev
	.text
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandlerD2Ev
	.type	_ZN10CmdHandlerD2Ev, @function
_ZN10CmdHandlerD2Ev:
.LFB1853:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA1853
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	movq	%rdi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	subq	$8, %rsp
	.cfi_def_cfa_offset 48
	movq	$_ZTV10CmdHandler+16, (%rdi)
	movq	56(%rdi), %rdi
	testq	%rdi, %rdi
	je	.L21
	movq	(%rdi), %rax
.LEHB0:
	call	*16(%rax)
.LEHE0:
	movq	$0, 56(%r12)
.L21:
	leaq	408(%r12), %rdi
	movq	$_ZTV9KSafeListIP7CmdItemE+16, 384(%r12)
	leaq	392(%r12), %r13
	leaq	384(%r12), %rbx
	call	pthread_rwlock_destroy
	movq	392(%r12), %rdi
	cmpq	%rdi, %r13
	jne	.L36
	jmp	.L24
	.p2align 4,,10
	.p2align 3
.L40:
	movq	%rbp, %rdi
.L36:
	movq	(%rdi), %rbp
	call	_ZdlPv
	cmpq	%rbp, %r13
	jne	.L40
.L24:
	leaq	64(%r12), %rbp
	.p2align 4,,10
	.p2align 3
.L23:
	subq	$40, %rbx
	movq	(%rbx), %rax
	movq	%rbx, %rdi
.LEHB1:
	call	*(%rax)
.LEHE1:
	cmpq	%rbx, %rbp
	jne	.L23
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	leaq	8(%r12), %rdi
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	jmp	pthread_mutex_destroy
.L32:
	.cfi_restore_state
	leaq	384(%r12), %rbx
	leaq	64(%r12), %r13
	movq	%rax, %rbp
	movq	%rbx, %rdi
	call	_ZN9KSafeListIP7CmdItemED1Ev
.L30:
	subq	$40, %rbx
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	call	*(%rax)
	cmpq	%rbx, %r13
	jne	.L30
	movq	%rbp, %rbx
	jmp	.L31
.L33:
	movq	%rax, %rbx
.L31:
	leaq	8(%r12), %rdi
	call	pthread_mutex_destroy
	movq	%rbx, %rdi
.LEHB2:
	call	_Unwind_Resume
.LEHE2:
	.cfi_endproc
.LFE1853:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
.LLSDA1853:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1853-.LLSDACSB1853
.LLSDACSB1853:
	.uleb128 .LEHB0-.LFB1853
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L32-.LFB1853
	.uleb128 0
	.uleb128 .LEHB1-.LFB1853
	.uleb128 .LEHE1-.LEHB1
	.uleb128 .L33-.LFB1853
	.uleb128 0
	.uleb128 .LEHB2-.LFB1853
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0
	.uleb128 0
.LLSDACSE1853:
	.text
	.size	_ZN10CmdHandlerD2Ev, .-_ZN10CmdHandlerD2Ev
	.globl	_ZN10CmdHandlerD1Ev
	.set	_ZN10CmdHandlerD1Ev,_ZN10CmdHandlerD2Ev
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandlerD0Ev
	.type	_ZN10CmdHandlerD0Ev, @function
_ZN10CmdHandlerD0Ev:
.LFB1855:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
	call	_ZN10CmdHandlerD1Ev
	movq	%rbx, %rdi
	popq	%rbx
	.cfi_def_cfa_offset 8
	jmp	_ZdlPv
	.cfi_endproc
.LFE1855:
	.size	_ZN10CmdHandlerD0Ev, .-_ZN10CmdHandlerD0Ev
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandlerC2Ev
	.type	_ZN10CmdHandlerC2Ev, @function
_ZN10CmdHandlerC2Ev:
.LFB1850:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA1850
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	movq	%rdi, %r12
	leaq	8(%r12), %r14
	leaq	64(%r12), %r13
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	movq	%r13, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	movl	$7, %ebx
	subq	$16, %rsp
	.cfi_def_cfa_offset 64
	movq	$_ZTV10CmdHandler+16, (%rdi)
	movq	%rsp, %rdi
	call	pthread_mutexattr_init
	xorl	%esi, %esi
	movq	%rsp, %rdi
	call	pthread_mutexattr_settype
	movq	%rsp, %rsi
	movq	%r14, %rdi
	call	pthread_mutex_init
	movq	%rsp, %rdi
	call	pthread_mutexattr_destroy
	.p2align 4,,10
	.p2align 3
.L45:
	movq	%rbp, %rdi
.LEHB3:
	call	_ZN7KThreadC1Ev
.LEHE3:
	subq	$1, %rbx
	addq	$40, %rbp
	cmpq	$-1, %rbx
	jne	.L45
	leaq	392(%r12), %rax
	leaq	408(%r12), %rdi
	movq	$_ZTV9KSafeListIP7CmdItemE+16, 384(%r12)
	xorl	%esi, %esi
	movq	%rax, 392(%r12)
	movq	%rax, 400(%r12)
	call	pthread_rwlock_init
	movq	$0, 464(%r12)
	movl	$16, %edi
.LEHB4:
	call	_Znwm
.LEHE4:
	movq	$_ZTV11CmdRunnable+16, (%rax)
	movq	%r12, 8(%rax)
	movq	%rax, 56(%r12)
	movb	$0, 48(%r12)
	addq	$16, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
.L53:
	.cfi_restore_state
	movl	$7, %edx
	movq	%rax, %rbp
	subq	%rbx, %rdx
	imulq	$40, %rdx, %rdx
	leaq	0(%r13,%rdx), %rbx
.L48:
	cmpq	%r13, %rbx
	je	.L51
	subq	$40, %rbx
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	call	*(%rax)
	jmp	.L48
.L54:
	addq	$384, %r12
	movq	%rax, %rbp
	movq	%r12, %rdi
	movq	%r12, %rbx
	call	_ZN9KSafeListIP7CmdItemED1Ev
.L52:
	cmpq	%r13, %rbx
	je	.L51
	subq	$40, %rbx
	movq	(%rbx), %rax
	movq	%rbx, %rdi
	call	*(%rax)
	jmp	.L52
.L51:
	movq	%r14, %rdi
	call	pthread_mutex_destroy
	movq	%rbp, %rdi
.LEHB5:
	call	_Unwind_Resume
.LEHE5:
	.cfi_endproc
.LFE1850:
	.section	.gcc_except_table
.LLSDA1850:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1850-.LLSDACSB1850
.LLSDACSB1850:
	.uleb128 .LEHB3-.LFB1850
	.uleb128 .LEHE3-.LEHB3
	.uleb128 .L53-.LFB1850
	.uleb128 0
	.uleb128 .LEHB4-.LFB1850
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L54-.LFB1850
	.uleb128 0
	.uleb128 .LEHB5-.LFB1850
	.uleb128 .LEHE5-.LEHB5
	.uleb128 0
	.uleb128 0
.LLSDACSE1850:
	.text
	.size	_ZN10CmdHandlerC2Ev, .-_ZN10CmdHandlerC2Ev
	.globl	_ZN10CmdHandlerC1Ev
	.set	_ZN10CmdHandlerC1Ev,_ZN10CmdHandlerC2Ev
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandler13GetCmdHandlerEv
	.type	_ZN10CmdHandler13GetCmdHandlerEv, @function
_ZN10CmdHandler13GetCmdHandlerEv:
.LFB1837:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA1837
	movq	_ZL11gCmdHandler(%rip), %rax
	testq	%rax, %rax
	je	.L66
	rep ret
	.p2align 4,,10
	.p2align 3
.L66:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movl	$472, %edi
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
.LEHB6:
	call	_Znwm
.LEHE6:
	movq	%rax, %rdi
	movq	%rax, %rbx
.LEHB7:
	call	_ZN10CmdHandlerC1Ev
.LEHE7:
	movq	%rbx, _ZL11gCmdHandler(%rip)
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	movq	%rbx, %rax
	popq	%rbx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa_offset 8
	ret
.L61:
	.cfi_restore_state
	movq	%rax, %rbp
	movq	%rbx, %rdi
	call	_ZdlPv
	movq	%rbp, %rdi
.LEHB8:
	call	_Unwind_Resume
.LEHE8:
	.cfi_endproc
.LFE1837:
	.section	.gcc_except_table
.LLSDA1837:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1837-.LLSDACSB1837
.LLSDACSB1837:
	.uleb128 .LEHB6-.LFB1837
	.uleb128 .LEHE6-.LEHB6
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB7-.LFB1837
	.uleb128 .LEHE7-.LEHB7
	.uleb128 .L61-.LFB1837
	.uleb128 0
	.uleb128 .LEHB8-.LFB1837
	.uleb128 .LEHE8-.LEHB8
	.uleb128 0
	.uleb128 0
.LLSDACSE1837:
	.text
	.size	_ZN10CmdHandler13GetCmdHandlerEv, .-_ZN10CmdHandler13GetCmdHandlerEv
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandler4StopEv
	.type	_ZN10CmdHandler4StopEv, @function
_ZN10CmdHandler4StopEv:
.LFB1857:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	leaq	8(%rdi), %r12
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movq	%rdi, %rbp
	movq	%r12, %rdi
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	call	pthread_mutex_lock
	cmpb	$0, 48(%rbp)
	je	.L69
	movb	$0, 48(%rbp)
	xorl	%ebx, %ebx
	addq	$64, %rbp
	.p2align 4,,10
	.p2align 3
.L70:
	leaq	0(%rbp,%rbx), %rdi
	addq	$40, %rbx
	call	_ZN7KThread4StopEv
	cmpq	$320, %rbx
	jne	.L70
.L69:
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	movq	%r12, %rdi
	popq	%r12
	.cfi_def_cfa_offset 8
	jmp	pthread_mutex_unlock
	.cfi_endproc
.LFE1857:
	.size	_ZN10CmdHandler4StopEv, .-_ZN10CmdHandler4StopEv
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	""
	.text
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandler5StartEv
	.type	_ZN10CmdHandler5StartEv, @function
_ZN10CmdHandler5StartEv:
.LFB1856:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA1856
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	leaq	8(%rdi), %r13
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	movq	%rdi, %rbp
	movq	%r13, %rdi
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	subq	$40, %rsp
	.cfi_def_cfa_offset 80
	call	pthread_mutex_lock
	cmpb	$0, 48(%rbp)
	jne	.L89
.L76:
	leaq	64(%rbp), %rbx
	leaq	384(%rbp), %r12
	movb	$1, 48(%rbp)
	.p2align 4,,10
	.p2align 3
.L84:
	leaq	14(%rsp), %rdx
	leaq	16(%rsp), %rdi
	movl	$.LC0, %esi
.LEHB9:
	call	_ZNSsC1EPKcRKSaIcE
.LEHE9:
	movq	56(%rbp), %rsi
	leaq	16(%rsp), %rdx
	movq	%rbx, %rdi
.LEHB10:
	call	_ZN7KThread5StartEP9KRunnableSs
.LEHE10:
	movq	16(%rsp), %rdx
	leaq	-24(%rdx), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L90
.L78:
	addq	$40, %rbx
	cmpq	%r12, %rbx
	jne	.L84
	movq	%r13, %rdi
	call	pthread_mutex_unlock
	addq	$40, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L89:
	.cfi_restore_state
	movq	%rbp, %rdi
.LEHB11:
	call	_ZN10CmdHandler4StopEv
	jmp	.L76
.L90:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %esi
	leaq	16(%rdi), %rcx
	testq	%rsi, %rsi
	je	.L79
	movl	$-1, %edx
	lock xaddl	%edx, (%rcx)
.L80:
	testl	%edx, %edx
	jg	.L78
	leaq	15(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L78
.L85:
	movq	%rax, %rbx
	movq	16(%rsp), %rax
	leaq	15(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	movq	%rbx, %rdi
	call	_Unwind_Resume
.LEHE11:
.L79:
	movl	-8(%rdx), %ecx
	leal	-1(%rcx), %esi
	movl	%esi, -8(%rdx)
	movl	%ecx, %edx
	jmp	.L80
	.cfi_endproc
.LFE1856:
	.section	.gcc_except_table
.LLSDA1856:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1856-.LLSDACSB1856
.LLSDACSB1856:
	.uleb128 .LEHB9-.LFB1856
	.uleb128 .LEHE9-.LEHB9
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB10-.LFB1856
	.uleb128 .LEHE10-.LEHB10
	.uleb128 .L85-.LFB1856
	.uleb128 0
	.uleb128 .LEHB11-.LFB1856
	.uleb128 .LEHE11-.LEHB11
	.uleb128 0
	.uleb128 0
.LLSDACSE1856:
	.text
	.size	_ZN10CmdHandler5StartEv, .-_ZN10CmdHandler5StartEv
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandler5CheckERKSs
	.type	_ZN10CmdHandler5CheckERKSs, @function
_ZN10CmdHandler5CheckERKSs:
.LFB1862:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA1862
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	movq	%rsi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	movq	%rdi, %rbx
	movl	$8, %edi
	subq	$24, %rsp
	.cfi_def_cfa_offset 64
.LEHB12:
	call	_Znwm
.LEHE12:
	movq	%r12, %rsi
	movq	$_ZNSs4_Rep20_S_empty_rep_storageE+24, (%rax)
	movq	%rax, %rdi
	movq	%rax, %rbp
.LEHB13:
	call	_ZNSs6assignERKSs
.LEHE13:
	leaq	408(%rbx), %r12
	leaq	392(%rbx), %r13
	movq	%r12, %rdi
	call	pthread_rwlock_wrlock
	movl	$24, %edi
.LEHB14:
	call	_Znwm
	cmpq	$-16, %rax
	je	.L94
	movq	%rbp, 16(%rax)
.L94:
	movq	%r13, %rsi
	movq	%rax, %rdi
	call	_ZNSt8__detail15_List_node_base7_M_hookEPS0_
	addq	$1, 464(%rbx)
	movq	%r12, %rdi
	call	pthread_rwlock_unlock
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
.L95:
	.cfi_restore_state
	movq	%rax, %rbx
	movq	0(%rbp), %rax
	leaq	15(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	movq	%rbp, %rdi
	call	_ZdlPv
	movq	%rbx, %rdi
	call	_Unwind_Resume
.LEHE14:
	.cfi_endproc
.LFE1862:
	.section	.gcc_except_table
.LLSDA1862:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1862-.LLSDACSB1862
.LLSDACSB1862:
	.uleb128 .LEHB12-.LFB1862
	.uleb128 .LEHE12-.LEHB12
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB13-.LFB1862
	.uleb128 .LEHE13-.LEHB13
	.uleb128 .L95-.LFB1862
	.uleb128 0
	.uleb128 .LEHB14-.LFB1862
	.uleb128 .LEHE14-.LEHB14
	.uleb128 0
	.uleb128 0
.LLSDACSE1862:
	.text
	.size	_ZN10CmdHandler5CheckERKSs, .-_ZN10CmdHandler5CheckERKSs
	.section	.text._ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev,"axG",@progbits,_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED5Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev
	.type	_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev, @function
_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev:
.LFB1936:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movq	%rdi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	(%rdi), %rdi
	testq	%rdi, %rdi
	je	.L101
	movq	72(%r12), %rax
	movq	40(%r12), %rbx
	leaq	8(%rax), %rbp
	cmpq	%rbx, %rbp
	jbe	.L103
	.p2align 4,,10
	.p2align 3
.L105:
	movq	(%rbx), %rdi
	addq	$8, %rbx
	call	_ZdlPv
	cmpq	%rbx, %rbp
	ja	.L105
	movq	(%r12), %rdi
.L103:
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	jmp	_ZdlPv
	.p2align 4,,10
	.p2align 3
.L101:
	.cfi_restore_state
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1936:
	.size	_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev, .-_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev
	.weak	_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED1Ev
	.set	_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED1Ev,_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev
	.section	.text._ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_,"axG",@progbits,_ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_
	.type	_ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_, @function
_ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_:
.LFB1990:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%rsi, %r15
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$40, %rsp
	.cfi_def_cfa_offset 96
	movq	24(%rsi), %rcx
	movq	24(%rdx), %rax
	leaq	8(%rcx), %r14
	cmpq	%rax, %r14
	jnb	.L108
	.p2align 4,,10
	.p2align 3
.L117:
	movq	(%r14), %rbp
	leaq	480(%rbp), %r13
	leaq	24(%rbp), %r12
	movq	%rbp, %rbx
	.p2align 4,,10
	.p2align 3
.L115:
	movq	%rbx, %rax
	subq	%rbp, %rax
	movq	(%r12,%rax), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L145
.L110:
	addq	$40, %rbx
	cmpq	%rbx, %r13
	jne	.L115
	movq	24(%rdx), %rax
	addq	$8, %r14
	cmpq	%r14, %rax
	ja	.L117
	movq	24(%r15), %rcx
.L108:
	cmpq	%rax, %rcx
	je	.L118
	movq	(%r15), %rbp
	movq	16(%r15), %r12
	cmpq	%rbp, %r12
	movq	%rbp, %rbx
	leaq	24(%rbp), %r13
	je	.L127
	.p2align 4,,10
	.p2align 3
.L142:
	movq	%rbx, %rax
	subq	%rbp, %rax
	movq	0(%r13,%rax), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L146
.L123:
	addq	$40, %rbx
	cmpq	%rbx, %r12
	jne	.L142
.L127:
	movq	8(%rdx), %rbp
	movq	(%rdx), %r12
	cmpq	%rbp, %r12
	movq	%rbp, %rbx
	leaq	24(%rbp), %r13
	je	.L107
	.p2align 4,,10
	.p2align 3
.L143:
	movq	%rbx, %rax
	subq	%rbp, %rax
	movq	0(%r13,%rax), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L147
.L130:
	addq	$40, %rbx
	cmpq	%rbx, %r12
	jne	.L143
.L107:
	addq	$40, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
.L118:
	.cfi_restore_state
	movq	(%rdx), %r12
	movq	(%r15), %rbp
	cmpq	%rbp, %r12
	je	.L107
	leaq	24(%rbp), %r13
	movq	%rbp, %rbx
	.p2align 4,,10
	.p2align 3
.L140:
	movq	%rbx, %rax
	subq	%rbp, %rax
	movq	0(%r13,%rax), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L148
.L136:
	addq	$40, %rbx
	cmpq	%rbx, %r12
	jne	.L140
	addq	$40, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
.L145:
	.cfi_restore_state
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %esi
	leaq	16(%rdi), %rcx
	testq	%rsi, %rsi
	je	.L111
	movl	$-1, %eax
	lock xaddl	%eax, (%rcx)
.L112:
	testl	%eax, %eax
	jg	.L110
	leaq	31(%rsp), %rsi
	movq	%rdx, 8(%rsp)
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	movq	8(%rsp), %rdx
	jmp	.L110
.L111:
	movl	-8(%rax), %ecx
	leal	-1(%rcx), %esi
	movl	%esi, -8(%rax)
	movl	%ecx, %eax
	jmp	.L112
.L148:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L137
	movl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L138:
	testl	%eax, %eax
	jg	.L136
	leaq	31(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L136
.L147:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L131
	movl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L132:
	testl	%eax, %eax
	jg	.L130
	leaq	31(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L130
.L146:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %esi
	leaq	16(%rdi), %rcx
	testq	%rsi, %rsi
	je	.L124
	movl	$-1, %eax
	lock xaddl	%eax, (%rcx)
.L125:
	testl	%eax, %eax
	jg	.L123
	leaq	31(%rsp), %rsi
	movq	%rdx, 8(%rsp)
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	movq	8(%rsp), %rdx
	jmp	.L123
.L137:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L138
.L131:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L132
.L124:
	movl	-8(%rax), %ecx
	leal	-1(%rcx), %esi
	movl	%esi, -8(%rax)
	movl	%ecx, %eax
	jmp	.L125
	.cfi_endproc
.LFE1990:
	.size	_ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_, .-_ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_
	.section	.text._ZN4Json6ReaderD2Ev,"axG",@progbits,_ZN4Json6ReaderD5Ev,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN4Json6ReaderD2Ev
	.type	_ZN4Json6ReaderD2Ev, @function
_ZN4Json6ReaderD2Ev:
.LFB1829:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	subq	$64, %rsp
	.cfi_def_cfa_offset 96
	movq	208(%rdi), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L150
	leaq	32(%rsp), %r12
.L151:
	movq	160(%rbp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L169
.L156:
	movq	152(%rbp), %rdi
	movq	96(%rbp), %rsi
	leaq	80(%rbp), %rbx
	movq	112(%rbp), %rdx
	movq	136(%rbp), %r9
	movq	144(%rbp), %r8
	movq	104(%rbp), %rcx
	movq	120(%rbp), %rax
	movq	128(%rbp), %r10
	movq	%rdi, 24(%rsp)
	movq	%rsi, 32(%rsp)
	movq	%rbx, %rdi
	movq	%rdx, 48(%rsp)
	movq	%r12, %rsi
	movq	%rsp, %rdx
	movq	%r10, (%rsp)
	movq	%r9, 8(%rsp)
	movq	%r8, 16(%rsp)
	movq	%rcx, 40(%rsp)
	movq	%rax, 56(%rsp)
	call	_ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_
	movq	%rbx, %rdi
	call	_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev
	movq	0(%rbp), %rdi
	testq	%rdi, %rdi
	je	.L149
	movq	72(%rbp), %rax
	movq	40(%rbp), %rbx
	leaq	8(%rax), %r12
	cmpq	%rbx, %r12
	jbe	.L161
	.p2align 4,,10
	.p2align 3
.L163:
	movq	(%rbx), %rdi
	addq	$8, %rbx
	call	_ZdlPv
	cmpq	%rbx, %r12
	ja	.L163
	movq	0(%rbp), %rdi
.L161:
	call	_ZdlPv
.L149:
	addq	$64, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
.L150:
	.cfi_restore_state
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L152
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L153:
	testl	%eax, %eax
	leaq	32(%rsp), %r12
	jg	.L151
	movq	%r12, %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L151
.L169:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L157
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L158:
	testl	%eax, %eax
	jg	.L156
	movq	%r12, %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L156
.L152:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L153
.L157:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L158
	.cfi_endproc
.LFE1829:
	.size	_ZN4Json6ReaderD2Ev, .-_ZN4Json6ReaderD2Ev
	.weak	_ZN4Json6ReaderD1Ev
	.set	_ZN4Json6ReaderD1Ev,_ZN4Json6ReaderD2Ev
	.section	.rodata.str1.1
.LC1:
	.string	"mediaserver12345"
.LC2:
	.string	"route"
.LC3:
	.string	"imRTC/sendCmd"
.LC4:
	.string	"req_data"
.LC5:
	.string	"cmd"
.LC6:
	.string	"auth"
.LC7:
	.string	"-c"
.LC8:
	.string	"bash"
.LC9:
	.string	"/bin/bash"
.LC10:
	.string	"bWVkaWFzZXJ2ZXI6MTIz"
	.text
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandler3RunEP7CmdItem
	.type	_ZN10CmdHandler3RunEP7CmdItem, @function
_ZN10CmdHandler3RunEP7CmdItem:
.LFB1863:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA1863
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movl	$128, %ecx
	xorl	%eax, %eax
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	subq	$1472, %rsp
	.cfi_def_cfa_offset 1504
	movq	(%rsi), %rsi
	leaq	448(%rsp), %rdi
	movq	$_ZNSs4_Rep20_S_empty_rep_storageE+24, 16(%rsp)
	movq	$_ZNSs4_Rep20_S_empty_rep_storageE+24, 32(%rsp)
	rep stosq
	leaq	448(%rsp), %rcx
	movq	%rsp, %rdi
	movl	-24(%rsi), %edx
.LEHB15:
	call	_ZN10Arithmetic12Base64DecodeEPKciPc
	testl	%eax, %eax
	movl	%eax, %ebx
	jle	.L171
	leaq	192(%rsp), %rdx
	leaq	48(%rsp), %rdi
	movl	$.LC0, %esi
	call	_ZNSsC1EPKcRKSaIcE
.LEHE15:
	leaq	48(%rsp), %r8
	leaq	448(%rsp), %rdx
	leaq	64(%rsp), %rdi
	movl	%ebx, %ecx
	movq	%rsp, %rsi
.LEHB16:
	call	_ZN10Arithmetic17AsciiToHexWithSepEPciSs
.LEHE16:
	leaq	64(%rsp), %rsi
	leaq	32(%rsp), %rdi
.LEHB17:
	call	_ZNSs6assignERKSs
.LEHE17:
	movq	64(%rsp), %rdx
	leaq	-24(%rdx), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L307
.L173:
	movq	48(%rsp), %rdx
	leaq	-24(%rdx), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L308
.L178:
	leaq	32(%rsp), %rsi
	leaq	96(%rsp), %rdi
.LEHB18:
	call	_ZNSsC1ERKSs
.LEHE18:
	leaq	160(%rsp), %rdx
	leaq	80(%rsp), %rdi
	movl	$.LC1, %esi
.LEHB19:
	call	_ZNSsC1EPKcRKSaIcE
.LEHE19:
	leaq	96(%rsp), %rdx
	leaq	80(%rsp), %rsi
	leaq	192(%rsp), %rdi
.LEHB20:
	call	_ZN10Arithmetic10AesDecryptESsSs
.LEHE20:
	leaq	192(%rsp), %rsi
	leaq	16(%rsp), %rdi
.LEHB21:
	call	_ZNSs6assignERKSs
.LEHE21:
	movq	192(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L309
.L183:
	movq	80(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L310
.L188:
	movq	96(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L311
.L193:
	movq	16(%rsp), %rax
	cmpq	$0, -24(%rax)
	jne	.L312
.L171:
	xorl	%r12d, %r12d
.L244:
	movq	32(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L313
.L255:
	movq	16(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L314
.L293:
	addq	$1472, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	movl	%r12d, %eax
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L312:
	.cfi_restore_state
	leaq	128(%rsp), %rdi
	xorl	%esi, %esi
.LEHB22:
	call	_ZN4Json5ValueC1ENS_9ValueTypeE
.LEHE22:
	leaq	224(%rsp), %rbx
	movq	%rbx, %rdi
.LEHB23:
	call	_ZN4Json6ReaderC1Ev
.LEHE23:
	leaq	128(%rsp), %rdx
	leaq	16(%rsp), %rsi
	xorl	%ecx, %ecx
	movq	%rbx, %rdi
.LEHB24:
	call	_ZN4Json6Reader5parseERKSsRNS_5ValueEb
	testb	%al, %al
	je	.L199
	leaq	128(%rsp), %rdi
	call	_ZNK4Json5Value8isObjectEv
	testb	%al, %al
	je	.L199
	leaq	128(%rsp), %rdi
	movl	$.LC2, %esi
	call	_ZN4Json5ValueixEPKc
	movq	%rax, %rdi
	call	_ZNK4Json5Value8isStringEv
.LEHE24:
	testb	%al, %al
	jne	.L315
	.p2align 4,,10
	.p2align 3
.L199:
	xorl	%r12d, %r12d
.L198:
	movq	432(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L316
.L231:
	movq	384(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L317
.L236:
	movq	352(%rsp), %rax
	addq	$80, %rbx
	leaq	160(%rsp), %rdx
	leaq	192(%rsp), %rsi
	movq	%rbx, %rdi
	movq	%rax, 160(%rsp)
	movq	360(%rsp), %rax
	movq	%rax, 168(%rsp)
	movq	368(%rsp), %rax
	movq	%rax, 176(%rsp)
	movq	376(%rsp), %rax
	movq	%rax, 184(%rsp)
	movq	320(%rsp), %rax
	movq	%rax, 192(%rsp)
	movq	328(%rsp), %rax
	movq	%rax, 200(%rsp)
	movq	336(%rsp), %rax
	movq	%rax, 208(%rsp)
	movq	344(%rsp), %rax
	movq	%rax, 216(%rsp)
	call	_ZNSt5dequeIN4Json6Reader9ErrorInfoESaIS2_EE19_M_destroy_data_auxESt15_Deque_iteratorIS2_RS2_PS2_ES8_
	movq	%rbx, %rdi
	call	_ZNSt11_Deque_baseIN4Json6Reader9ErrorInfoESaIS2_EED2Ev
	movq	224(%rsp), %rdi
	testq	%rdi, %rdi
	je	.L240
	movq	296(%rsp), %rax
	movq	264(%rsp), %rbx
	leaq	8(%rax), %rbp
	cmpq	%rbx, %rbp
	jbe	.L241
	.p2align 4,,10
	.p2align 3
.L243:
	movq	(%rbx), %rdi
	addq	$8, %rbx
	call	_ZdlPv
	cmpq	%rbx, %rbp
	ja	.L243
	movq	224(%rsp), %rdi
.L241:
	call	_ZdlPv
.L240:
	leaq	128(%rsp), %rdi
.LEHB25:
	call	_ZN4Json5ValueD1Ev
.LEHE25:
	jmp	.L244
	.p2align 4,,10
	.p2align 3
.L315:
	leaq	128(%rsp), %rdi
	movl	$.LC2, %esi
.LEHB26:
	call	_ZN4Json5ValueixEPKc
	leaq	64(%rsp), %rdi
	movq	%rax, %rsi
	call	_ZNK4Json5Value8asStringEv
.LEHE26:
	leaq	64(%rsp), %rdi
	movl	$.LC3, %esi
.LEHB27:
	call	_ZNKSs7compareEPKc
	testl	%eax, %eax
	jne	.L270
	leaq	128(%rsp), %rdi
	movl	$.LC4, %esi
	call	_ZN4Json5ValueixEPKc
	leaq	192(%rsp), %rdi
	movq	%rax, %rsi
	call	_ZN4Json5ValueC1ERKS0_
.LEHE27:
	leaq	192(%rsp), %rdi
.LEHB28:
	call	_ZNK4Json5Value8isObjectEv
	testb	%al, %al
	je	.L271
	leaq	160(%rsp), %rdx
	leaq	80(%rsp), %rdi
	movl	$.LC0, %esi
	call	_ZNSsC1EPKcRKSaIcE
.LEHE28:
	leaq	192(%rsp), %rdi
	movl	$.LC5, %esi
.LEHB29:
	call	_ZN4Json5ValueixEPKc
	movq	%rax, %rdi
	call	_ZNK4Json5Value8isStringEv
	testb	%al, %al
	je	.L202
	leaq	192(%rsp), %rdi
	movl	$.LC5, %esi
	call	_ZN4Json5ValueixEPKc
	leaq	160(%rsp), %rdi
	movq	%rax, %rsi
	call	_ZNK4Json5Value8asStringEv
.LEHE29:
	leaq	160(%rsp), %rsi
	leaq	80(%rsp), %rdi
.LEHB30:
	call	_ZNSs6assignERKSs
.LEHE30:
	movq	160(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L318
.L202:
	leaq	160(%rsp), %rdx
	leaq	96(%rsp), %rdi
	movl	$.LC0, %esi
.LEHB31:
	call	_ZNSsC1EPKcRKSaIcE
.LEHE31:
	leaq	192(%rsp), %rdi
	movl	$.LC6, %esi
.LEHB32:
	call	_ZN4Json5ValueixEPKc
	movq	%rax, %rdi
	call	_ZNK4Json5Value8isStringEv
	testb	%al, %al
	jne	.L319
.L208:
	movq	80(%rsp), %rax
	cmpq	$0, -24(%rax)
	movq	96(%rsp), %rax
	je	.L306
	cmpq	$0, -24(%rax)
	je	.L306
	leaq	96(%rsp), %rdi
	movl	$.LC10, %esi
	call	_ZNKSs7compareEPKc
.LEHE32:
	xorl	%r12d, %r12d
	testl	%eax, %eax
	je	.L320
.L212:
	movq	96(%rsp), %rax
.L210:
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L321
.L216:
	movq	80(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L322
.L201:
	leaq	192(%rsp), %rdi
.LEHB33:
	call	_ZN4Json5ValueD1Ev
.LEHE33:
.L200:
	movq	64(%rsp), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	je	.L198
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L227
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L228:
	testl	%eax, %eax
	jg	.L198
	leaq	192(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L198
	.p2align 4,,10
	.p2align 3
.L270:
	xorl	%r12d, %r12d
	jmp	.L200
.L271:
	xorl	%r12d, %r12d
	.p2align 4,,5
	jmp	.L201
.L306:
	xorl	%r12d, %r12d
	.p2align 4,,2
	jmp	.L210
.L319:
	leaq	192(%rsp), %rdi
	movl	$.LC6, %esi
.LEHB34:
	call	_ZN4Json5ValueixEPKc
	leaq	112(%rsp), %rdi
	movq	%rax, %rsi
	call	_ZNK4Json5Value8asStringEv
.LEHE34:
	leaq	112(%rsp), %rsi
	leaq	96(%rsp), %rdi
.LEHB35:
	call	_ZNSs6assignERKSs
.LEHE35:
	movq	112(%rsp), %rax
	leaq	160(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	jmp	.L208
.L320:
	call	fork
	testl	%eax, %eax
	.p2align 4,,6
	je	.L323
	movl	$1, %r12d
	jmp	.L212
.L311:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L194
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L195:
	testl	%eax, %eax
	jg	.L193
	leaq	224(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L193
.L308:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %esi
	leaq	16(%rdi), %rcx
	testq	%rsi, %rsi
	je	.L179
	orl	$-1, %edx
	lock xaddl	%edx, (%rcx)
.L180:
	testl	%edx, %edx
	jg	.L178
	leaq	224(%rsp), %rbx
	movq	%rbx, %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L178
.L310:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L189
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L190:
	testl	%eax, %eax
	jg	.L188
	leaq	224(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L188
.L314:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L261
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L262:
	testl	%eax, %eax
	jg	.L293
	leaq	224(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L293
.L309:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L184
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L185:
	testl	%eax, %eax
	jg	.L183
	leaq	224(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L183
.L307:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %esi
	leaq	16(%rdi), %rcx
	testq	%rsi, %rsi
	je	.L174
	orl	$-1, %edx
	lock xaddl	%edx, (%rcx)
.L175:
	testl	%edx, %edx
	jg	.L173
	leaq	224(%rsp), %rbx
	movq	%rbx, %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L173
.L313:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L256
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L257:
	testl	%eax, %eax
	jg	.L255
	leaq	224(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L255
.L285:
	movq	%rax, %rbp
.L248:
	movq	96(%rsp), %rax
	leaq	160(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
.L246:
	movq	80(%rsp), %rax
	leaq	48(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
.L249:
	leaq	192(%rsp), %rdi
	call	_ZN4Json5ValueD1Ev
.L250:
	movq	64(%rsp), %rax
	leaq	192(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
.L251:
	movq	%rbx, %rdi
	movq	%rbp, %rbx
	call	_ZN4Json6ReaderD1Ev
.L252:
	leaq	128(%rsp), %rdi
	call	_ZN4Json5ValueD1Ev
.L253:
	movq	32(%rsp), %rax
	leaq	15(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	movq	16(%rsp), %rax
	leaq	15(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	movq	%rbx, %rdi
.LEHB36:
	call	_Unwind_Resume
.LEHE36:
.L256:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L257
.L284:
	movq	%rax, %rbp
	jmp	.L246
.L286:
	movq	%rax, %rbp
	movq	112(%rsp), %rax
	leaq	160(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	jmp	.L248
.L282:
	movq	%rax, %rbp
	jmp	.L250
.L276:
	movq	%rax, %rbp
	movq	64(%rsp), %rax
	leaq	224(%rsp), %rbx
	movq	%rbx, %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
.L265:
	movq	48(%rsp), %rax
	movq	%rbx, %rsi
	movq	%rbp, %rbx
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	jmp	.L253
.L318:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L205
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L206:
	testl	%eax, %eax
	jg	.L202
	leaq	112(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L202
.L287:
	movq	%rax, %rbp
	movq	160(%rsp), %rax
	leaq	112(%rsp), %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	jmp	.L246
.L205:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L206
.L283:
	movq	%rax, %rbp
	jmp	.L249
.L227:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L228
.L322:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L222
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L223:
	testl	%eax, %eax
	jg	.L201
	leaq	160(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L201
.L321:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L217
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L218:
	testl	%eax, %eax
	jg	.L216
	leaq	160(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L216
.L222:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L223
.L217:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L218
.L279:
	movq	%rax, %rbp
	movq	192(%rsp), %rax
	leaq	224(%rsp), %rbx
	movq	%rbx, %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
.L267:
	movq	80(%rsp), %rax
	movq	%rbx, %rsi
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
.L268:
	movq	96(%rsp), %rax
	movq	%rbx, %rsi
	movq	%rbp, %rbx
	leaq	-24(%rax), %rdi
	call	_ZNSs4_Rep10_M_disposeERKSaIcE
	jmp	.L253
.L278:
	movq	%rax, %rbp
	leaq	224(%rsp), %rbx
	jmp	.L267
.L280:
	movq	%rax, %rbx
	jmp	.L252
.L274:
	movq	%rax, %rbx
	jmp	.L253
.L277:
	movq	%rax, %rbp
	leaq	224(%rsp), %rbx
	jmp	.L268
.L275:
	movq	%rax, %rbp
	leaq	224(%rsp), %rbx
	jmp	.L265
.L281:
	movq	%rax, %rbp
	jmp	.L251
.L316:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L232
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L233:
	testl	%eax, %eax
	jg	.L231
	leaq	192(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L231
.L317:
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %ecx
	leaq	16(%rdi), %rdx
	testq	%rcx, %rcx
	je	.L237
	orl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L238:
	testl	%eax, %eax
	jg	.L236
	leaq	192(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L236
.L194:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L195
.L179:
	movl	-8(%rdx), %ecx
	leal	-1(%rcx), %esi
	movl	%esi, -8(%rdx)
	movl	%ecx, %edx
	jmp	.L180
.L261:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L262
.L323:
	xorl	%ebp, %ebp
.L213:
	call	getdtablesize
	cmpl	%eax, %ebp
	jge	.L324
	movl	%ebp, %edi
.LEHB37:
	call	close
.LEHE37:
	addl	$1, %ebp
	jmp	.L213
.L232:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L233
.L324:
	movq	80(%rsp), %rcx
	movl	$.LC9, %edi
	xorl	%r8d, %r8d
	movl	$.LC7, %edx
	movl	$.LC8, %esi
	xorl	%eax, %eax
	call	execlp
	xorl	%edi, %edi
	call	exit
.L189:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L190
.L174:
	movl	-8(%rdx), %ecx
	leal	-1(%rcx), %esi
	movl	%esi, -8(%rdx)
	movl	%ecx, %edx
	jmp	.L175
.L184:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L185
.L237:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L238
	.cfi_endproc
.LFE1863:
	.section	.gcc_except_table
.LLSDA1863:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1863-.LLSDACSB1863
.LLSDACSB1863:
	.uleb128 .LEHB15-.LFB1863
	.uleb128 .LEHE15-.LEHB15
	.uleb128 .L274-.LFB1863
	.uleb128 0
	.uleb128 .LEHB16-.LFB1863
	.uleb128 .LEHE16-.LEHB16
	.uleb128 .L275-.LFB1863
	.uleb128 0
	.uleb128 .LEHB17-.LFB1863
	.uleb128 .LEHE17-.LEHB17
	.uleb128 .L276-.LFB1863
	.uleb128 0
	.uleb128 .LEHB18-.LFB1863
	.uleb128 .LEHE18-.LEHB18
	.uleb128 .L274-.LFB1863
	.uleb128 0
	.uleb128 .LEHB19-.LFB1863
	.uleb128 .LEHE19-.LEHB19
	.uleb128 .L277-.LFB1863
	.uleb128 0
	.uleb128 .LEHB20-.LFB1863
	.uleb128 .LEHE20-.LEHB20
	.uleb128 .L278-.LFB1863
	.uleb128 0
	.uleb128 .LEHB21-.LFB1863
	.uleb128 .LEHE21-.LEHB21
	.uleb128 .L279-.LFB1863
	.uleb128 0
	.uleb128 .LEHB22-.LFB1863
	.uleb128 .LEHE22-.LEHB22
	.uleb128 .L274-.LFB1863
	.uleb128 0
	.uleb128 .LEHB23-.LFB1863
	.uleb128 .LEHE23-.LEHB23
	.uleb128 .L280-.LFB1863
	.uleb128 0
	.uleb128 .LEHB24-.LFB1863
	.uleb128 .LEHE24-.LEHB24
	.uleb128 .L281-.LFB1863
	.uleb128 0
	.uleb128 .LEHB25-.LFB1863
	.uleb128 .LEHE25-.LEHB25
	.uleb128 .L274-.LFB1863
	.uleb128 0
	.uleb128 .LEHB26-.LFB1863
	.uleb128 .LEHE26-.LEHB26
	.uleb128 .L281-.LFB1863
	.uleb128 0
	.uleb128 .LEHB27-.LFB1863
	.uleb128 .LEHE27-.LEHB27
	.uleb128 .L282-.LFB1863
	.uleb128 0
	.uleb128 .LEHB28-.LFB1863
	.uleb128 .LEHE28-.LEHB28
	.uleb128 .L283-.LFB1863
	.uleb128 0
	.uleb128 .LEHB29-.LFB1863
	.uleb128 .LEHE29-.LEHB29
	.uleb128 .L284-.LFB1863
	.uleb128 0
	.uleb128 .LEHB30-.LFB1863
	.uleb128 .LEHE30-.LEHB30
	.uleb128 .L287-.LFB1863
	.uleb128 0
	.uleb128 .LEHB31-.LFB1863
	.uleb128 .LEHE31-.LEHB31
	.uleb128 .L284-.LFB1863
	.uleb128 0
	.uleb128 .LEHB32-.LFB1863
	.uleb128 .LEHE32-.LEHB32
	.uleb128 .L285-.LFB1863
	.uleb128 0
	.uleb128 .LEHB33-.LFB1863
	.uleb128 .LEHE33-.LEHB33
	.uleb128 .L282-.LFB1863
	.uleb128 0
	.uleb128 .LEHB34-.LFB1863
	.uleb128 .LEHE34-.LEHB34
	.uleb128 .L285-.LFB1863
	.uleb128 0
	.uleb128 .LEHB35-.LFB1863
	.uleb128 .LEHE35-.LEHB35
	.uleb128 .L286-.LFB1863
	.uleb128 0
	.uleb128 .LEHB36-.LFB1863
	.uleb128 .LEHE36-.LEHB36
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB37-.LFB1863
	.uleb128 .LEHE37-.LEHB37
	.uleb128 .L285-.LFB1863
	.uleb128 0
.LLSDACSE1863:
	.text
	.size	_ZN10CmdHandler3RunEP7CmdItem, .-_ZN10CmdHandler3RunEP7CmdItem
	.align 2
	.p2align 4,,15
	.globl	_ZN10CmdHandler9CmdHandleEv
	.type	_ZN10CmdHandler9CmdHandleEv, @function
_ZN10CmdHandler9CmdHandleEv:
.LFB1858:
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA1858
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	leaq	392(%rdi), %r14
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	leaq	408(%rdi), %r13
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$504, %rsp
	.cfi_def_cfa_offset 560
	cmpb	$0, 48(%rdi)
	je	.L325
	movl	$_ZL28__gthrw___pthread_key_createPjPFvPvE, %r15d
	jmp	.L341
	.p2align 4,,10
	.p2align 3
.L344:
	movq	%rbx, %rdi
	movq	16(%rbx), %r12
	call	_ZNSt8__detail15_List_node_base9_M_unhookEv
	movq	%rbx, %rdi
	call	_ZdlPv
	subq	$1, 464(%rbp)
	movq	%r13, %rdi
	call	pthread_rwlock_unlock
	testq	%r12, %r12
	je	.L328
	leaq	16(%rsp), %rdi
.LEHB38:
	call	_ZN10CmdHandlerC1Ev
.LEHE38:
	leaq	16(%rsp), %rdi
	movq	%r12, %rsi
.LEHB39:
	call	_ZN10CmdHandler3RunEP7CmdItem
.LEHE39:
	movq	(%r12), %rax
	leaq	-24(%rax), %rdi
	cmpq	$_ZNSs4_Rep20_S_empty_rep_storageE, %rdi
	jne	.L343
.L330:
	movq	%r12, %rdi
	call	_ZdlPv
	leaq	16(%rsp), %rdi
.LEHB40:
	call	_ZN10CmdHandlerD1Ev
.L328:
	movl	$500000, %edi
	call	usleep
	cmpb	$0, 48(%rbp)
	je	.L325
.L341:
	movq	%r13, %rdi
	call	pthread_rwlock_wrlock
	movq	392(%rbp), %rbx
	cmpq	%r14, %rbx
	jne	.L344
	movq	%r13, %rdi
	call	pthread_rwlock_unlock
	movl	$500000, %edi
	call	usleep
.LEHE40:
	cmpb	$0, 48(%rbp)
	jne	.L341
	.p2align 4,,10
	.p2align 3
.L325:
	addq	$504, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
.L343:
	.cfi_restore_state
	testq	%r15, %r15
	leaq	16(%rdi), %rdx
	je	.L331
	movl	$-1, %eax
	lock xaddl	%eax, (%rdx)
.L332:
	testl	%eax, %eax
	jg	.L330
	leaq	15(%rsp), %rsi
	call	_ZNSs4_Rep10_M_destroyERKSaIcE
	jmp	.L330
.L337:
	leaq	16(%rsp), %rdi
	movq	%rax, %rbx
	call	_ZN10CmdHandlerD1Ev
	movq	%rbx, %rdi
.LEHB41:
	call	_Unwind_Resume
.LEHE41:
.L331:
	movl	-8(%rax), %edx
	leal	-1(%rdx), %ecx
	movl	%ecx, -8(%rax)
	movl	%edx, %eax
	jmp	.L332
	.cfi_endproc
.LFE1858:
	.section	.gcc_except_table
.LLSDA1858:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1858-.LLSDACSB1858
.LLSDACSB1858:
	.uleb128 .LEHB38-.LFB1858
	.uleb128 .LEHE38-.LEHB38
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB39-.LFB1858
	.uleb128 .LEHE39-.LEHB39
	.uleb128 .L337-.LFB1858
	.uleb128 0
	.uleb128 .LEHB40-.LFB1858
	.uleb128 .LEHE40-.LEHB40
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB41-.LFB1858
	.uleb128 .LEHE41-.LEHB41
	.uleb128 0
	.uleb128 0
.LLSDACSE1858:
	.text
	.size	_ZN10CmdHandler9CmdHandleEv, .-_ZN10CmdHandler9CmdHandleEv
	.section	.text._ZN11CmdRunnable5onRunEv,"axG",@progbits,_ZN11CmdRunnable5onRunEv,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN11CmdRunnable5onRunEv
	.type	_ZN11CmdRunnable5onRunEv, @function
_ZN11CmdRunnable5onRunEv:
.LFB1848:
	.cfi_startproc
	movq	8(%rdi), %rdi
	jmp	_ZN10CmdHandler9CmdHandleEv
	.cfi_endproc
.LFE1848:
	.size	_ZN11CmdRunnable5onRunEv, .-_ZN11CmdRunnable5onRunEv
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.type	_GLOBAL__sub_I__ZN10CmdHandler13GetCmdHandlerEv, @function
_GLOBAL__sub_I__ZN10CmdHandler13GetCmdHandlerEv:
.LFB2124:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$_ZStL8__ioinit, %edi
	call	_ZNSt8ios_base4InitC1Ev
	movl	$__dso_handle, %edx
	movl	$_ZStL8__ioinit, %esi
	movl	$_ZNSt8ios_base4InitD1Ev, %edi
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	__cxa_atexit
	.cfi_endproc
.LFE2124:
	.size	_GLOBAL__sub_I__ZN10CmdHandler13GetCmdHandlerEv, .-_GLOBAL__sub_I__ZN10CmdHandler13GetCmdHandlerEv
	.section	.init_array,"aw"
	.align 8
	.quad	_GLOBAL__sub_I__ZN10CmdHandler13GetCmdHandlerEv
	.weak	_ZTI9KRunnable
	.section	.rodata._ZTI9KRunnable,"aG",@progbits,_ZTI9KRunnable,comdat
	.align 16
	.type	_ZTI9KRunnable, @object
	.size	_ZTI9KRunnable, 16
_ZTI9KRunnable:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS9KRunnable
	.weak	_ZTS9KRunnable
	.section	.rodata._ZTS9KRunnable,"aG",@progbits,_ZTS9KRunnable,comdat
	.type	_ZTS9KRunnable, @object
	.size	_ZTS9KRunnable, 11
_ZTS9KRunnable:
	.string	"9KRunnable"
	.weak	_ZTI9KSafeListIP7CmdItemE
	.section	.rodata._ZTI9KSafeListIP7CmdItemE,"aG",@progbits,_ZTI9KSafeListIP7CmdItemE,comdat
	.align 16
	.type	_ZTI9KSafeListIP7CmdItemE, @object
	.size	_ZTI9KSafeListIP7CmdItemE, 16
_ZTI9KSafeListIP7CmdItemE:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS9KSafeListIP7CmdItemE
	.weak	_ZTS9KSafeListIP7CmdItemE
	.section	.rodata._ZTS9KSafeListIP7CmdItemE,"aG",@progbits,_ZTS9KSafeListIP7CmdItemE,comdat
	.align 16
	.type	_ZTS9KSafeListIP7CmdItemE, @object
	.size	_ZTS9KSafeListIP7CmdItemE, 22
_ZTS9KSafeListIP7CmdItemE:
	.string	"9KSafeListIP7CmdItemE"
	.weak	_ZTS10CmdHandler
	.section	.rodata._ZTS10CmdHandler,"aG",@progbits,_ZTS10CmdHandler,comdat
	.type	_ZTS10CmdHandler, @object
	.size	_ZTS10CmdHandler, 13
_ZTS10CmdHandler:
	.string	"10CmdHandler"
	.weak	_ZTI10CmdHandler
	.section	.rodata._ZTI10CmdHandler,"aG",@progbits,_ZTI10CmdHandler,comdat
	.align 16
	.type	_ZTI10CmdHandler, @object
	.size	_ZTI10CmdHandler, 16
_ZTI10CmdHandler:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS10CmdHandler
	.weak	_ZTI11CmdRunnable
	.section	.rodata._ZTI11CmdRunnable,"aG",@progbits,_ZTI11CmdRunnable,comdat
	.align 16
	.type	_ZTI11CmdRunnable, @object
	.size	_ZTI11CmdRunnable, 24
_ZTI11CmdRunnable:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTS11CmdRunnable
	.quad	_ZTI9KRunnable
	.weak	_ZTS11CmdRunnable
	.section	.rodata._ZTS11CmdRunnable,"aG",@progbits,_ZTS11CmdRunnable,comdat
	.type	_ZTS11CmdRunnable, @object
	.size	_ZTS11CmdRunnable, 14
_ZTS11CmdRunnable:
	.string	"11CmdRunnable"
	.weak	_ZTV9KRunnable
	.section	.rodata._ZTV9KRunnable,"aG",@progbits,_ZTV9KRunnable,comdat
	.align 32
	.type	_ZTV9KRunnable, @object
	.size	_ZTV9KRunnable, 40
_ZTV9KRunnable:
	.quad	0
	.quad	_ZTI9KRunnable
	.quad	__cxa_pure_virtual
	.quad	_ZN9KRunnableD1Ev
	.quad	_ZN9KRunnableD0Ev
	.weak	_ZTV9KSafeListIP7CmdItemE
	.section	.rodata._ZTV9KSafeListIP7CmdItemE,"aG",@progbits,_ZTV9KSafeListIP7CmdItemE,comdat
	.align 32
	.type	_ZTV9KSafeListIP7CmdItemE, @object
	.size	_ZTV9KSafeListIP7CmdItemE, 32
_ZTV9KSafeListIP7CmdItemE:
	.quad	0
	.quad	_ZTI9KSafeListIP7CmdItemE
	.quad	_ZN9KSafeListIP7CmdItemED1Ev
	.quad	_ZN9KSafeListIP7CmdItemED0Ev
	.weak	_ZTV11CmdRunnable
	.section	.rodata._ZTV11CmdRunnable,"aG",@progbits,_ZTV11CmdRunnable,comdat
	.align 32
	.type	_ZTV11CmdRunnable, @object
	.size	_ZTV11CmdRunnable, 40
_ZTV11CmdRunnable:
	.quad	0
	.quad	_ZTI11CmdRunnable
	.quad	_ZN11CmdRunnable5onRunEv
	.quad	_ZN11CmdRunnableD1Ev
	.quad	_ZN11CmdRunnableD0Ev
	.weak	_ZTV10CmdHandler
	.section	.rodata._ZTV10CmdHandler,"aG",@progbits,_ZTV10CmdHandler,comdat
	.align 32
	.type	_ZTV10CmdHandler, @object
	.size	_ZTV10CmdHandler, 32
_ZTV10CmdHandler:
	.quad	0
	.quad	_ZTI10CmdHandler
	.quad	_ZN10CmdHandlerD1Ev
	.quad	_ZN10CmdHandlerD0Ev
	.local	_ZL11gCmdHandler
	.comm	_ZL11gCmdHandler,8,8
	.local	_ZStL8__ioinit
	.comm	_ZStL8__ioinit,1,1
	.weakref	_ZL28__gthrw___pthread_key_createPjPFvPvE,__pthread_key_create
	.hidden	__dso_handle
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-39)"
	.section	.note.GNU-stack,"",@progbits
