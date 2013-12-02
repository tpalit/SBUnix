# 
# This is the common part of every interrupt handler
# Our friendly processor will handle the pushing and
# popping of the RFLAGS, RCX, and RIP.
# So we are left with having to deal with the other
# registers.

# The routine _isr_common_handler will take the
# address of the specific irc routine to run
# and then invoke it.

# When everything is done, then we pop back
# the stack in the reverse order, into their
# respective registers.

# Note - In x86_64 Linux environments, the
# input arguments to an assembly routine go into
# RDI, RSI, RDX, RCX, R8 and R9, unlike in 32 bit
# environments where they are all on the stack.

# Note - The PUSHA instruction isn't valid in the
# 64 bit mode. So, we'll manually push the registers!
# That's so much fun!

# Copyright (c) 2013 by Tapti Palit. All rights reserved.
	
.text

.extern isr_handler_body_0
.extern isr_handler_body_32
	
.global _isr_handler_head_0
.global _isr_handler_head_10	
.global _isr_handler_head_13
.global _isr_handler_head_14
.global _isr_handler_head_32
.global _isr_handler_head_33

_isr_handler_head_0:

	# Please don't interrupt
	cli
	
	# Push the common registers
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rsp
	push %rbp
	push %rsi
	push %rdi

	# No need to push the segment
	# registers since we're in long mode.
	
	call isr_handler_body_0

	# Pop stuff back
	
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rsp
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	# Turn interrupts back on
	sti

	# Return from interrupt
	iretq

# The Invalid TSS exception
_isr_handler_head_10:

	# Please don't interrupt
	cli
	
	# Push the common registers
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rsp
	push %rbp
	push %rsi
	push %rdi

	# No need to push the segment
	# registers since we're in long mode.
	
	call isr_handler_body_10

	# Pop stuff back
	
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rsp
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	# Turn interrupts back on
	sti

	# Return from interrupt
	iretq
	
# For General Protection
_isr_handler_head_13:

	# Please don't interrupt
	cli
	
	# Push the common registers
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rsp
	push %rbp
	push %rsi
	push %rdi

	# No need to push the segment
	# registers since we're in long mode.
	
	call isr_handler_body_13

	# Pop stuff back
	
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rsp
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	# Turn interrupts back on
	sti

	# Return from interrupt
	iretq

# For Page Fault
_isr_handler_head_14:

	cli
	pop %r10
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rsp
	push %rbp
	push %rsi
	push %rdi
        pushq %r8
        pushq %r9
        pushq %r10
        pushq %r11
	pushq %r12
	pushq %r13
        pushq %r14
	pushq %r15
	pushq %rdi
	pushq %rsi
	pushq %rdx
	
	call isr_handler_body_14

	popq %rdx
	popq %rsi
	popq %rdi
	popq %r15
        popq %r14
        popq %r13
        popq %r12
        popq %r11
        popq %r10
        popq %r9
        popq %r8
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rsp
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	sti

	iretq
	
_isr_handler_head_32:
	# Please don't interrupt
	cli
	
	# Push the common registers
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rsp
	pushq %rbp
	pushq %rsi
	pushq %rdi

	# No need to push the segment
	# registers since we're in long mode.
	
	callq schedule_on_timer

	# Pop stuff back
	
	popq %rdi
	popq %rsi
	popq %rbp
	popq %rsp
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax

	# Send the EOI to PIC
	# both the Master and Child
	mov $0x20, %al
	out %al, $0x20
	out %al, $0xA0

	# Turn interrupts back on
	sti

	# Return from interrupt
	iretq

_isr_handler_head_33:
	# Please don't interrupt
	cli
	
	# Push the common registers
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rsp
	pushq %rbp
	pushq %rsi
	pushq %rdi

	# No need to push the segment
	# registers since we're in long mode.
	
	callq isr_handler_body_33

	# Pop stuff back
	
	popq %rdi
	popq %rsi
	popq %rbp
	popq %rsp
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax

	# Send the EOI to PIC
	# both the Master and Child
	mov $0x20, %al
	out %al, $0x20
	out %al, $0xA0
	
	# Turn interrupts back on
	sti

	# Return from interrupt
	iretq





