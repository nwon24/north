# North

This is planned to be a new implementation of a Forth-like language in C.

If you have never heard of Forth, Forth is a low-level stack-based
language. For more information visit [its Wikipedia page](https://en.wikipedia.org/wiki/Forth_(programming_language)) or the [Forth website](https://forth.com). For a tutorial on how to write code
in the language see the [Gforth manual](https://gforth.org/manual/).

For documentation on North visit the [doc](./doc) subdirectory.

# How will this be different to Forth?

For now all the basic operations of North are the same as Forth. However,
this North is not intended to be a new implementation of Forth; rather, it
is meant to be a *Forth-like* language. As the language develops, it
might (and probably will) deviate from Forth.

Note that there are already slight differences between this language and
Forth. For one, North code must be written in a file and then compiled
or simulated; there is no interactive mode. Furthermore, loops and conditionals
can be written outside of colon definitions.

# How to build

On a *nix system, simply running `./build.sh` in the root of the project
should build everything successfully. You just need a basic C toolchain. The binary
will be in the root of the project.

By default, `./build.sh` runs `make` with `CC=cc` and `TARGET=x86_64-linux`. 
For the moment, no other platform is supported, but it is possible to compile
`north` will another C compiler. For example, running `CC=clang ./build.sh`
uses `clang` as the C compiler.

# How to use

For basic usage and the language reference, see the [North documentation](./doc/north.md).

Note that when generating assembly code, North assumes that the GNU
assembler is installed, or at least an assembler that understands AT&T syntax.

# How to test

Running `./test.sh` will run the tests on each folder inside the [tests](./tests)
folder. At the end of all the tests the number of successful tests will
be printed onto the screen.

# Acknowledgments

This project was much inspired by [this project](https://gitlab.com/tsoding/porth)
and this [series of videos](https://www.youtube.com/playlist?list=PLpM-Dvs8t0VbMZA7wW9aR3EtBqe2kinu4). Highly recommend checking it out.
