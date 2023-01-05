#ifndef SIMULATE_H
#define SIMULATE_H

#include <stdint.h>

#if defined(__x86_64__)
typedef int64_t word;
#else
#error "No other architecture currently supported for simulation"
#endif

extern char **simulated_argv;
extern int simulated_argc;

void simulate(void);

#endif /* SIMULATE_H */
