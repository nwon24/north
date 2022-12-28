#!/bin/sh

CC=${CC-cc}
ARCH=${ARCH-x86_64-linux}

set +xe

export CC
export ARCH

[ ! -z "$CC" ] || (echo "CC not set." ; exit 1)
[ ! -z "$ARCH" ] || (echo "Target architecture not set." ; exit 1)

make -B
