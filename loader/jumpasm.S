#include <loader/control.h>
#include <loader/jumper.h>
#include <loader/multiboot2.h>
	

	.section ".text"
	.balign 8
	.code32
	.global jump_to_relocated
jump_to_relocated:
	# The ljmp instruction requires two arguments
	# - an immediate segment selector (for CS)
	# - an immediate address to jump to
	# Because we don't know the second at compile time, we have to use
	# a self modifying code (and we also use it for the first operand for
	# convenience)

	# Set the selector argument: the current value of CS
	movw	%cs, %ax
	movw	%ax, 2f

	# Set the address argument: our first argument
	movl	4(%esp), %eax
	movl	%eax, 1f

	.byte	0xea			# Opcode of ljmp
1:
	.long	0			# Address for ljmp
2:
	.word	0			# Selector for ljmp

	# If something oes wrong with the long jump, hang on
	# TODO: goto die()
1:
	jmp 1b


# Documentation for long mode activation can be found in
#   AMD64 Architecture Programmer's Manual, Volume 2: System Programming
#   http://developer.amd.com/wordpress/media/2012/10/24593_APM_v21.pdf
#     14.6 - Enabling and Activating Long Mode
	
	.section ".jumper-text"
	.balign 8
	.code32
	.global jump_to_longmode
jump_to_longmode:
	# At this point, the loader has already initialized a page table and
	# loaded the CR3 with its address, and the jumper has already set up
	# a valid 64 bits GDT (however, CS still points to a 32 bits segment
	# descriptor: JUMPER_GTD_CODE32).
	# Here we just have to set the right bits and jump to the payload.

	movl	$STAR, %ecx
	rdmsr
	movl	%eax, %ecx
	addl	$1f, %eax
	movl	%ecx, 0(%eax)
	addl	$long_mode, 0(%eax)
	movw	$JUMPER_GDT_CODE64, 4(%eax)

	movl	%cr4, %eax
	orl	$CR4_PAE, %eax
	movl	%eax, %cr4

	movl	$EFER, %ecx
	rdmsr
	orl	$EFER_LME, %eax
	wrmsr

	movl	%cr0, %eax
	orl	$CR0_PG, %eax
	movl	%eax, %cr0

	.byte	0xea			# Opcode of ljmp
1:
	.long	0			# Address for ljmp
2:
	.word	0			# Selector for ljmp

	.code64
long_mode:
	# Reset the START register to zero
	movl	$STAR, %ecx
	xorl	%eax, %eax
	xorl	%edx, %edx
	wrmsr
	
	# Set RAX to the Multiboot2 magic value
	xorq	%rax, %rax
	movl	$MULTIBOOT2_MAGIC_EAX, %eax

	# Set RBX to the Multiboot2 information structure address
	movq	12(%rsp), %rbx

	# Set RDX to the first byte of the preserve zone
	movq	20(%rsp), %rdx

	# Set RCX to the first byte following the preserve zone
	movq	28(%rsp), %rcx

	# Now jump to the payload, our job is over ...
	movq	4(%rsp), %rdi
	jmpq	*%rdi
