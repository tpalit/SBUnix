# This file will write the gdt and idt into
# the memory and refresh the registers.

# Note - In x86_64 Linux environments, the
# input arguments to an assembly routine go into
# RDI, RSI, RDX, RCX, R8 and R9, unlike in 32 bit
# environments where they are all on the stack.

# Copyright (c) 2013 by Tapti Palit. All rights reserved.

.text

######
# load a new GDT
#  parameter 1: address of gdtr
#  parameter 2: new code descriptor offset
#  parameter 3: new data descriptor offset
.global _flush_gdt
.global _flush_idt
.global _flush_tss
	
_flush_gdt:

	lgdt (%rdi)
	movq  $0x10, %rdx
	pushq $0x08                  # push code selector
	movabsq $.done, %r10
	pushq %r10                  # push return address
	lretq                       # far-return to new cs descriptor ( the retq below )
.done:
	movq %rdx, %es
	movq %rdx, %fs
	movq %rdx, %gs
	movq %rdx, %ds
	movq %rdx, %ss
	retq

_flush_idt:
	lidt (%rdi)
	retq

_flush_tss:
	mov $0x2b, %ax 		#The index is 0x28, but set the two bits for RPL 3
	ltr %ax
	retq
