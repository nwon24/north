#!/bin/sh

CC=${CC-cc}
ARCH=${ARCH-x86_64-linux}

case $ARCH in
    x86_64-*)
	ln -sf x86_64/x86_64.nth stdlib/sysdeps/arch.nth
	;;
esac
case $ARCH in
    x86_64-linux)
	ln -sf x86_64/linux/syscall.nth stdlib/sysdeps/syscall.nth
	ln -sf x86_64/linux/const.nth stdlib/sysdeps/const.nth
	;;
    x86_64-freebsd)
	ln -sf x86_64/freebsd/syscall.nth stdlib/sysdeps/syscall.nth
	ln -sf x86_64/freebsd/const.nth stdlib/sysdeps/const.nth
	;;
esac

set +xe

export CC
export ARCH

[ ! -z "$CC" ] || (echo "CC not set." ; exit 1)
[ ! -z "$ARCH" ] || (echo "Target architecture not set." ; exit 1)

make "$@"
