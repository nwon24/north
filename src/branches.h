#ifndef BRANCHES_H_
#define BRANCHES_H_

#define MAX_NESTED_BRANCHES 20

#include "ops.h"

struct operation;

void cross_reference_branches(struct operation *ops);

#endif /* BRANCHES_H_*/
