#define KERNEL_PMA  0x100000
#define KERNEL_VMA  0xffffffff80000000

# Documentation for multiboot2 can be found in
#   The Multiboot Specification version 1.6
#   http://nongnu.askapache.com/grub/phcoder/multiboot.pdf

	.section ".multiboot2"
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
	.long	.header_start       - KERNEL_VMA + KERNEL_PMA
	.long	__kernel_text_start - KERNEL_VMA + KERNEL_PMA
	.long	__kernel_data_end   - KERNEL_VMA + KERNEL_PMA
	.long	__kernel_bss_end    - KERNEL_VMA + KERNEL_PMA

	# The entry address tag of Multiboot header
	.balign	8
	.word	3
	.word	0
	.long	12
	.long	entry - 0xffffffff80000000 + 0x100000

	# The terminating tag of Multiboot header
	.balign	8
	.word	0
	.word	0
	.long	8
.header_end:

	.code64
	.section ".text"
	.global entry
	.balign	8
entry:
	movq	$_entry_stack_top, %rsp
	pushq	%rax
	pushq	%rbx
	call	main
	hlt

	.section ".bss"
	.balign	4096
_entry_stack_bottom:
	.space	4096
_entry_stack_top:
