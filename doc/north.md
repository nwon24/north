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

Note that the assembler is invoked as `as`, and is expected to
know AT&T assembler syntax.

## Further options

Use the option `-h` to generate a basic help message and exit.
Use the option `-v` to enable verbose output; this means the
assembler and linker commands will be echoed to the terminal.

# The Language

The basics of the language (such as stack operations and arithmetic
operations) are the same as Forth. For completeness, a brief
overview of these basic facilities is given here.

## Comments

There are two styles of comments in the language. A single
backslash `\` comments out the rest of the line. Anything
between parentheses `(` and `)` will also be ignored
as comments. This latter way is slightly less clear if
the comment spans several lines, so the former method is
the recommended one. For inline comments, of course,
using parentheses is better.

## Integers

An integer constant has the effect of pushing its value onto
the stack. They must be made up of a series of decimal digits, with
an optional sign (`+` or `-`) preceding them. Currently integers
into other bases are not recognised.

## Floats

Floating point numbers are currently not implemented.

## Arithmetic operations

Arithmetic operators operator on the elements that are currently
on the stack. If you are new to stack-orientated languages, this
will be confusing because it means operators are placed after
the operands, in a form known as reverse Polish notation. The
following examples make this clear.
```
10 15 +    \ 25
8 2 -      \ 6 
-9 3 *     \ -27
45 9 /     \ 5
37 5 mod   \ 2
```
An easy way to remember the notation is to think of moving
the operator from between the two operands, as is with
the common infix notation, to after the second operand.

Also note that arithmetic operators, as with most other
words of the language, consume their arguments. This means
the operands of a word are replaced by the result. In the
first example, `10` and `15` are removed from the stack ('popped')
and replaced by their sum, `25`. 

Here is a table of all the arithmetic operations currently
supported. In this table, and all other following tables,
values written left to right indicate the state of the stack,
where the top of the stack (the value most recently pushed) is
on the right.

| Operator | Action                    |
|----------|---------------------------|
| `+`      | `a b -- a + b`            |
| `-`      | `a b -- a - b`            |
| `*`      | `a b -- a * b`            |
| `/`      | `a b -- a // b`           |
| `mod`    | `a b -- a % b`            |
| `/mod`   | `a b -- (a % b) (a // b)` |
| `and`    | `a b -- a & b`            |
| `or`     | `a b -- a \| b`           |
| `xor`    | `a b -- a ^ b`            |
| `lshift` | `a b -- a << b`           |
| `rshift` | `a b -- a >> b`           |
| `negate` | `a -- -a`                 |
| `1+`     | `a -- a + 1`              |
| `1-`     | `a -- a - 1`              |

## The `.` word
The `.` word pops the top value from the stack
and prints it to the screen in decimal notation with a newline. 
Currently it prints the value as a signed integer.
A future improvement is to introduce a word `.u` for
printing unsigned integers.

## Stack operations

<!--  LocalWords:  Arithmetics
 -->
