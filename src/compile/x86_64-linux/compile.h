#ifndef COMPILE_H_
#define COMPILE_H_

#define PRINT_ASM "print:\n" \
"\t	pushq %%rbx		\n" \
"\t	pushq %%rbp\n" \
"\t	xorq %%rcx, %%rcx		# %%rcx counter\n" \
"\t	cmpq $0, %%rdi\n" \
"\t	jge 1f\n" \
"\t	movq $printbuf, %%rsi\n" \
"\t	movb $'-', (%%rsi)\n" \
"\t	incq %%rcx\n" \
"\t	negq %%rdi\n" \
"\t\n" \
"1:\t	movq %%rcx, %%r9		# Save starting offset\n" \
"1:\t	cmpq $0, %%rdi		# %%rdi <= 0?\n" \
"\t	jle 1f			# Yes, exit loop\n" \
"\t	movq %%rdi, %%rax         # %%rdi / 10\n" \
"\t	xorq %%rdx, %%rdx\n" \
"\t	movq $10, %%rbx\n" \
"\t	divq %%rbx               # Quot: %%rax, remain: %%rdx\n" \
"\t	movq $'0', %%rbx         # Get ASCII for '0'\n" \
"\t	add %%rdx, %%rbx          # Get ASCII code for digit\n" \
"\t	movq $printbuf, %%rsi         # Load pointer to buffer\n" \
"\t	addq %%rcx, %%rsi         # Add offset\n" \
"\t	movb %%bl, (%%rsi)        # Move character to buffer\n" \
"\t	movq %%rax, %%rdi         # %%rdi = %%rdi / 10\n" \
"\t	incq %%rcx               # Increment counter\n" \
"\t	jmp 1b                  # Loop\n" \
"1:\t      movq $printbuf, %%rdi\n" \
"\t	addq %%rcx, %%rdi\n" \
"\t	movb $'\n', (%%rdi)        # Add newline\n" \
"\t	movq %%r9, %%rax         # Reverse string: %%rax = 0 or 1, %%rdx = %%rcx -1\n" \
"\t	movq %%rcx, %%rdx\n" \
"\t	decq %%rdx\n" \
"2:\t	cmpq %%rax, %%rdx         # %%rdx <= %%raz? (finished reversing?)\n" \
"\t	jle 2f                  # Yes, exit loop\n" \
"\t	movq $printbuf, %%rdi         # Load pointer to buffer\n" \
"\t	addq %%rdx, %%rdi         # Add left-hand offset\n" \
"\t	movb (%%rdi), %%bl        # Save character\n" \
"\t	movq $printbuf, %%rdi         # Load pointer to buffer\n" \
"\t	addq %%rax, %%rdi         # Add right-hand offset\n" \
"\t	movb (%%rdi), %%bh        # Save character\n" \
"\t	movb %%bl, (%%rdi)        # Swap\n" \
"\t	movq $printbuf, %%rdi         # Swap other character\n" \
"\t	addq %%rdx, %%rdi\n" \
"\t	movb %%bh, (%%rdi)\n" \
"\t	incq %%rax               # Increment left-hand offset\n" \
"\t	decq %%rdx               # Increment right-hand offset\n" \
"\t	jmp 2b                  # Loop\n" \
"2:	movq $1, %%rax           # Set up 'write' system call\n" \
"\t	movq $1, %%rdi           # STDOUT_FILENo\n" \
"\t	movq $printbuf, %%rsi         # buffer\n" \
"\t	movq %%rcx, %%rdx         # count\n" \
"\t	incq %%rdx               # Include newline\n" \
"\t	syscall\n" \
"\t	popq %%rbp        \n" \
"\t	popq %%rbx\n" \
"\t	ret\n" \
"\t	\n" \
"\t	.section .data\n" \
"printbuf:\n" \
"	.skip 20\n" \
"\n"

#endif /* COMPILE_H_ */
