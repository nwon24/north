# North

# Introduction

North is a low-level, stack-oriented, concatenative programming language.
It has many of the basic features of [Forth](https://en.wikipedia.org/wiki/Forth_(programming_language)), but there are crucial differences
as well.

North can either be simulated (interpreted) or compiled into a native
executable; this is controlled by a flag to the compiler. The only platform
supported for compilation right now is AMD64 machines with Linux. A distant
goal of the project is to support compilation on a wide range of architectures.
Simulation is (should be) platform-independent.

**Note that the language is still under heavy development. Features may
break suddenly or there may be undefined behaviour.**

# Usage

North files have extension `.nth`. The compiler also recognises files
with no file extension, but this is not the recommended practice.

## Simulating and compiling

To simulate the a North file `test.nth`, run the following command.
```shell
    $ north -s test.nth
```
To compile a North file `test.nth`, run the following command.
```shell
    $ north -c test.nth
```
The above command generates an assembly file `test.s`, then runs
the assembler and then the linker to generate a statically linked
executable simply named `test`. You can run this executable like
you would run any other executable.

## Further options

Use the option `-h` to generate a basic help message and exit.
Use the option `-v` to enable verbose output; this means the
assembler and linker commands will be echoed to the terminal.
