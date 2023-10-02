#include <loader/offsets.h>


# Entry point of the kernel with multiboot 2.
# This small portion of code has the following goals :
# - Set up a stack, its address is stored in the mistral header
# - Save the values of %eax and %ebx which are meaningfull on the stack
# - TODO: Copy back the overlayed data (see mistral basic scheme)
# - Jump to the C entry point which perform initialization	

	.section ".bootstrap"
	.balign 8
	.code32
	.global entry_multiboot2
entry_multiboot2:
	movl	$mistral_header, %edx
	movl	MISTRAL_STACK(%edx), %esp
	pushl	%eax
	pushl	%ebx
	call	main_multiboot2
	hlt


# Documentation for multiboot2 can be found in
#   The Multiboot Specification version 1.6
#   http://nongnu.askapache.com/grub/phcoder/multiboot.pdf

	.section ".multiboot2"
	.align	64
.header_start:
	# The magic field of Multiboot header
	.long	0xe85250d6
	.long	0
	.long	(.header_end - .header_start)
	.long	-(0xe85250d6 + 0 + (.header_end - .header_start))

	# The address tag of Multiboot header
	.balign	8
	.word	2
	.word	0
	.long	24
	.long	.header_start
	.long	__kernel_text_start
	.long	__kernel_data_end
	.long	__kernel_bss_end

	# The entry address tag of Multiboot header
	.balign	8
	.word	3
	.word	0
	.long	12
	.long	entry_multiboot2

	# The terminating tag of Multiboot header
	.balign	8
	.word	0
	.word	0
	.long	8
.header_end:
