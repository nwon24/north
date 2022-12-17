# North

This is planned to be a new implementation of a Forth-like languge in C.

If you have never heard of Forth, Forth is a low-level stack-based
language. For more information visit [its Wikipedia page](https://en.wikipedia.org/wiki/Forth_(programming_language)) or the [Forth website](https://forth.com). For a tutorial on how to write code
in the language see the [Gforth manual](https://gforth.org/manual/).

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

On a *nix system, simply running `make` in the root of the project
should build everything successfully. You just need a basic C toolchain. The binary
will be in the `src` directory.

Note that when generating assembly code, North assumes that the GNU
assembler is installed, or at least an assembler that understands AT&T syntax.

# Acknowledgements

This project was much inspired by [this project](https://gitlab.com/tsoding/porth)
and this [series of videos](https://www.youtube.com/playlist?list=PLpM-Dvs8t0VbMZA7wW9aR3EtBqe2kinu4). Highly recommend checking it out.
