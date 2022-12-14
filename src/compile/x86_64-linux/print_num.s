	.section .text
	.global _start
_start:
	movq $0, %rdi
	call print

	movq $60, %rax
	movq $0, %rdi
	syscall

print:
	pushq %rbx		
	pushq %rbp
	xorq %rcx, %rcx		# %rcx counter
	cmpq $0, %rdi
	jge 1f
	movq $buf, %rsi
	movb $'-', (%rsi)
	incq %rcx
	negq %rdi

1:	movq %rcx, %r9		# Save starting offset
1:	
	movq %rdi, %rax         # %rdi / 10
	xorq %rdx, %rdx
	movq $10, %rbx
	divq %rbx               # Quot: %rax, remain: %rdx
	movq $'0', %rbx         # Get ASCII for '0'
	add %rdx, %rbx          # Get ASCII code for digit
	movq $buf, %rsi         # Load pointer to buffer
	addq %rcx, %rsi         # Add offset
	movb %bl, (%rsi)        # Move character to buffer
	movq %rax, %rdi         # %rdi = %rdi / 10
	incq %rcx               # Increment counter
	cmpq $0, %rdi		# %rdi <= 0?
	jg 1b			# No, continue loop
1:      movq $buf, %rdi
	addq %rcx, %rdi
	movb $'\n', (%rdi)        # Add newline
	movq %r9, %rax         # Reverse string: %rax = 0 or 1, %rdx = %rcx -1
	movq %rcx, %rdx
	decq %rdx
2:	cmpq %rax, %rdx         # %rdx <= %raz? (finished reversing?)
	jle 2f                  # Yes, exit loop
	movq $buf, %rdi         # Load pointer to buffer
	addq %rdx, %rdi         # Add left-hand offset
	movb (%rdi), %bl        # Save character
	movq $buf, %rdi         # Load pointer to buffer
	addq %rax, %rdi         # Add right-hand offset
	movb (%rdi), %bh        # Save character
	movb %bl, (%rdi)        # Swap
	movq $buf, %rdi         # Swap other character
	addq %rdx, %rdi
	movb %bh, (%rdi)
	incq %rax               # Increment left-hand offset
	decq %rdx               # Increment right-hand offset
	jmp 2b                  # Loop
2:	movq $1, %rax           # Set up 'write' system call
	movq $1, %rdi           # STDOUT_FILENo
	movq $buf, %rsi         # buffer
	movq %rcx, %rdx         # count
	incq %rdx               # Include newline
	syscall
	popq %rbp        
	popq %rbx
	ret
	
	.section .data
buf:
	.skip 20
