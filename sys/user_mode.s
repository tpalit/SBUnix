# Testing user mode. Should be removed.

.text

.global switch_to_user_mode
.extern test_user_function
	
switch_to_user_mode:
	movq $0x23, %rcx
	movq %rcx, %ds
	movq %rcx, %es
	movq %rcx, %fs
	movq %rcx, %gs

	movq %rsp, %rcx
	pushq $0x23
	pushq %rcx
	pushfq
	pushq $0x1b
#	pushq $test_user_function
	pushq %rdx
	iretq
