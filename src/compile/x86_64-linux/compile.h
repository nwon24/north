#ifndef COMPILE_H_
#define COMPILE_H_

#define PRINT_ASM "print:\n" \
"\tpushq %%rbx		\n" \
"\tpushq %%rbp\n" \
"\txorq %%rcx, %%rcx		# %%rcx counter\n" \
"\tcmpq $0, %%rdi\n" \
"\tjge 1f\n" \
"\tmovq $printbuf, %%rsi\n" \
"\tmovb $'-', (%%rsi)\n" \
"\tincq %%rcx\n" \
"\tnegq %%rdi\n" \
"\t\n" \
"1:\tmovq %%rcx, %%r9		# Save starting offset\n" \
"1:\n" \
"\tmovq %%rdi, %%rax         # %%rdi / 10\n" \
"\txorq %%rdx, %%rdx\n" \
"\tmovq $10, %%rbx\n" \
"\tdivq %%rbx               # Quot: %%rax, remain: %%rdx\n" \
"\tmovq $'0', %%rbx         # Get ASCII for '0'\n" \
"\tadd %%rdx, %%rbx          # Get ASCII code for digit\n" \
"\tmovq $printbuf, %%rsi         # Load pointer to buffer\n" \
"\taddq %%rcx, %%rsi         # Add offset\n" \
"\tmovb %%bl, (%%rsi)        # Move character to buffer\n" \
"\tmovq %%rax, %%rdi         # %%rdi = %%rdi / 10\n" \
"\tincq %%rcx               # Increment counter\n" \
"\tcmpq $0, %%rdi		# %%rdi <= 0?\n" \
"\tjg 1b			# No, continue loop\n" \
"1:\tmovq $printbuf, %%rdi\n" \
"\taddq %%rcx, %%rdi\n" \
"\tmovb $\'\\n\', (%%rdi)        # Add newline\n" \
"\tmovq %%r9, %%rax         # Reverse string: %%rax = 0 or 1, %%rdx = %%rcx -1\n" \
"\tmovq %%rcx, %%rdx\n" \
"\tdecq %%rdx\n" \
"2:\tcmpq %%rax, %%rdx         # %%rdx <= %%raz? (finished reversing?)\n" \
"\tjle 2f                  # Yes, exit loop\n" \
"\tmovq $printbuf, %%rdi         # Load pointer to buffer\n" \
"\taddq %%rdx, %%rdi         # Add left-hand offset\n" \
"\tmovb (%%rdi), %%bl        # Save character\n" \
"\tmovq $printbuf, %%rdi         # Load pointer to buffer\n" \
"\taddq %%rax, %%rdi         # Add right-hand offset\n" \
"\tmovb (%%rdi), %%bh        # Save character\n" \
"\tmovb %%bl, (%%rdi)        # Swap\n" \
"\tmovq $printbuf, %%rdi         # Swap other character\n" \
"\taddq %%rdx, %%rdi\n" \
"\tmovb %%bh, (%%rdi)\n" \
"\tincq %%rax               # Increment left-hand offset\n" \
"\tdecq %%rdx               # Increment right-hand offset\n" \
"\tjmp 2b                  # Loop\n" \
"2:movq $1, %%rax           # Set up 'write' system call\n" \
"\tmovq $1, %%rdi           # STDOUT_FILENo\n" \
"\tmovq $printbuf, %%rsi         # buffer\n" \
"\tmovq %%rcx, %%rdx         # count\n" \
"\tincq %%rdx               # Include newline\n" \
"\tsyscall\n" \
"\tpopq %%rbp        \n" \
"\tpopq %%rbx\n" \
"\tret\n" \
"\t\n" \
"\t.section .data\n" \
"printbuf:\n" \
"\t.skip 20\n" \
"\n"

#endif /* COMPILE_H_ */
