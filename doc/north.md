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

You can also add the `-r` flag to run the executable
automatically (if the assembler and linker exited successfully).
This avoids having to run the executable yourself, but at the
moment no command line arguments can be given to the program
when it is run with this flag.

Note that the assembler is invoked as `as`, and is expected to
know AT&T assembler syntax.

## Further options

Use the option `-h` to generate a basic help message and exit.
Use the option `-v` to enable verbose output; this means the
assembler and linker commands will be echoed to the terminal.

To run a different assembler and linker, use the `-a` and `-l`
flags. For example, to use `llvm-mc` and `ld.lld` on `test.nth`, run
`north -c -a "llvm-mc -filetype=obj" -l "ld.lld" test.nth`.
Remember that the assembler must be able to take assembly
code in AT&T syntax.

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
the stack. They must be made up of a series of decimal, octal, or hexadecimal digits, with
an optional sign (`+` or `-`) preceding them. 

Hexadecimal constants are preceded by `0x`; octal constants are
preceded by `0`. An integer constant beginning with any of
the digits `0-9` is taken to be a decimal constant. This is the
same notation as is used in C.

## Floats

Floating point numbers are currently not implemented.

## String literals

Strings can be specified in the same way as in many other languages:
between sets of double quotes. Many C-style escape sequences are supported,
although not all.

| Escape sequence | Description          |
|-----------------|----------------------|
| `\n`            | Newline              |
| `\a`            | Beep                 |
| `\f`            | Formfeed             |
| `\r`            | Carriage return      |
| `\t`            | Horizontal tab       |
| `\v`            | Vertical tab         |
| `\'`            | Literal single quote |
| `\"`            | Literal double quote |
| `\\`            | Literal backslash    |

When a string literal is specified, two values are pushed onto
the stack:  the length of the string and the address of the string
in read-only memory. The address of the string is uppermost on the
stack.

## Character constants

Character constants are specified the same way they are in C.
Specifying a character constant pushes its ASCII value onto the stack.
For example `'a' .` prints 97 onto the screen.

All the same escape sequences are supported for character constants
as for strings.

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

Stack operations affect the top elements of the stack.

| Operator | Action                   |
|----------|--------------------------|
| `dup`    | `a -- a a`               |
| `drop`   | `a b -- a`               |
| `rot`    | `a b c -- b c a`         |
| `over`   | `a b -- a b a`           |
| `swap`   | `a b -- b a`             |
| `nip`    | `a b -- b`               |
| `tuck`   | `a b -- b a b`           |
| `2drop`  | `a b -- `                |
| `2dup`   | `a b -- a b a b`         |
| `2over`  | `a b c d -- a b c d a b` |
| `2swap`  | `a b c d -- c d a b`     |

## Miscellaneous operations

| Operator | Action                                                   |
|----------|----------------------------------------------------------|
| `max`    | Finds the maximum of the two numbers on top of the stack |
| `min`    | Finds the minimum of the two numbers on top of the stack |
| `abs`    | Finds the absolute value of the top number on the stack  |

## Comparison operators

Comparison operators compare the top two elements of the stack, replacing
them with either a flag being 'true' or a flag meaning 'false'. The false
flag is `0`. Contrary
to languages like C, the 'true' flag is `-1`. However, the conditional
operators (such as `if`) take any value other than `0` as 'true'.

Since comparing with `0` is common, some comparison operators compare
the top value on the stack with `0` without `0` having to be pushed onto
the stack.

| Operation | Condition                |
|-----------|--------------------------|
| `=`       | Equal                    |
| `0=`      | Equal to `0`             |
| `<>`      | Not equal                |
| `>`       | Greater than             |
| `0>`      | Greater than `0`         |
| `>=`      | Greater than or equal to |
| `<`       | Less than                |
| `0<`      | Less than `0`            |
| `<=`      | Less than or equal to    |

## Conditionals

The general structure of conditional execution in North is as follows.
```
<condition> if
  <body>
else
  <body>
endif
```

The `else` is optional; if this is omitted and the condition before
the `if` word evaluates to `0`, execution jumps straight to after
the `endif` word.

There is also the `endifs` word, which closes every open conditional
preceding it.
Multiple `if` and `else` operations can be used like this:
```
<conditiion> if
  <body>
else <condition> if
  <body>
else <condition> if
  <body>
endifs
```
Note how the `endifs` operation is used to close all the nested
`if` operations. This avoids the need for multiple `endif`s,,  which
can becoome very messy when there are mutliple `else .. if` sections
of code.

## Loops

There are three kinds of loops supported by North: `do` loops, `while` loops,
and `until` loops.

### `do` loops

The simplest kind of `loop` is the `do` loop. In this loop the housekeeping
(i.e., the management of the lower and upper bounds) is done automatically.
The general structure of this loop is as follows.
```
<upper bound> <lower bound> do <body> loop
```
The `do` loop must be ended by the `loop` word, which increments the
loop counter by `1`. In the body of the loop, using the word `i` pushes
the current value of the loop counter onto the stack. Here is
an example.
```
\ This prints the integers from 0 to 999
1000 0 do i . loop
```

Instead of `loop`, `+loop` can be used, which increments the loop 
counter by the number on top of the stack. This way decrementing loops
can be written.
```
0 1000 do i . -2 +loop
```

This kind of loop, while simple, is not very flexible because multiple
`do` loops cannot be nested; in that case the word `i` would be
ambiguous. Furthermore, since the loop counter is being handled
by the compiler/interpreter, nested loops would break the internal
register allocation.

### `while` and `until` loops

`while` and `until` loops are much more flexible than `do` loops, but
it is easier to make a mistake and result in an unwanted segmentation
fault or infinite loop.

The general structure of the `while` loop is as follow.
```
<starting value> begin <condition> while
  <body>
repeat
```
Note that the starting value and condition can be anything;
the `while` word is really the same thing as `if`, except
that it is jumped to by the `repeat` word and the condition
evaluated again. Here is a simple loop that uses `while`.
```
0 begin dup 1000 < while
  dup .
  1+
repeat
```
Note how `dup` is used whenever the loop counter is needed.
At the end of the loop, the loop counter must be on top of
the stack, since it needs to be incremented or decremented,
so the body of the loop must keep the stack as it was by
its end. The body of the loop can have any number of operations;
the only thing that matters is that at the end of the body,
everything that has been pushed onto the stack has also been popped.
Therefore, whenever working with the loop counter, use `dup` because
the resulting operations will consume the value on top of
the stack. If we left the `dup` out in the above example,
the `.` operation would consume the loop counter, and the `1+`
would be acting on an undefined value, most likely resulting
in an infinite loop.

Also note that at the end of the loop, the loop counter remains
on top of the stack, so when using nested loops, make sure to `drop`
the loop counter at the end of all inner loops.

`until` loops are the basically the same as `while` loops,
except that the condition is evaluated at the end, rather than
the beginning. The basic structure is:
```
<starting value> begin
  <body>
  <condition> until
```
Here is a simple example.
```
0 begin
  dup 2 * .
  1 + 
  dup 100 < until
```
This would print out all the even numbers less than `200`.

The caveats of the `while` loop apply equally to the `until` loop.
Since the language has no railing or safety features, simple mistakes
can lead to catastrophic and often baffling behaviour by the
resulting program, especially if the stack has been heavily used
and contains many different values.

### The `leave` operation

To exit a loop prematurely, use the `leave` operation. This exits
the loop currently running. It is equivalent to the `break` keyword
in C.

## System calls

North has a convenient interface to the system calls, insomuch
as system calls can be considered convenient. There are seven
different system call words, one for each possible number of
arguments to the system call: `syscall0`, `syscall1`, `syscall2`,
and so on up to `syscall6`.

The top-most number on the stack is taken as the system call
number (consult the documentation for your specific operating
system if you don't remember the numbers). The arguments
of the system call are then taken as the numbers after that,
with the last argument being uppermost on the stack (after
the system call number).

To illustrate, here is what the famous `hello world` program
currently looks like in North (at least on Linux).
```
"Hello, world!\n" 1 1 syscall3
```
The string literal causes its size and address to be pushed onto
the stack in reverse order. Then `1` is pushed onto the stack
to indicate standard output. The next `1` is the number of
the `write` system call in Linux. Then we use the word `syscall3`
because `write` takes three arguments.

All system calls return an error value; this value is pushed
onto the stack. If this value needs to be ignored, remember
to `drop` it. Forgetting this will lead to mysterious bugs.

## Variables and memory

Space in memory can be reserved for storing values with
the `.var` directive. The space reserved is a contiguous
block of memory of the specified size (basically arrays
but with none of the handy indexing of many languages).

The structure of the `.var` directive is as follows.
```
.var <identifier> <type> <size>
```
The identifier must be no more than 31 characters long
and must begin with an alphabetic character. The type field
is one of eight different options:

| Type  | Description             |
|-------|-------------------------|
| `i8`  | Signed 8-bit integer    |
| `i16` | Signed 16-bit integer   |
| `i32` | Signed 32-bit integer   |
| `i64` | Signed 64-bit integer   |
| `u8`  | Unsigned 8-bit integer  |
| `u16` | Unsigned 16-bit integer |
| `u32` | Unsigned 32-bit integer |
| `u64` | Unsigned 64-bit integer |

**Note: for now, the signedness doesn't matter, since our arithmetic
words don't differentiate between signed and unsigned integers. It
has just been added in case future developments require the user
to know if an array contains signed or unsigned integers.**

The `size` field must be a decimal integer. It is the number
of cells that will be allocated, where each cell is the size
of the type just specified. So if a size of `8` and a type of
`u32` were specified, the array would take up `32` bytes in
memory.

Once a variable has been declared, it can be referred to by
its identifier; doing so pushes the address of the first cell
onto the stack. Offsets in the array can be obtained by adding
onto the base address pushed onto the stack. Note that there aren't
any fancy pointer arithmetics, so to get the number second in
an array of 64-bit integers, `8` has to be added to the base pointer.

Reading and writing from these memory locations is done using the `!` (store)
and `@` (load) family of words. These words are suffixed by either `8`, `16`, `32`,
or `64`, to indicate whether an 8-bit, 16-bit, 32-bit, or 64-bit integer
should be stored or loaded. With both operations, the top value
of the stack is the target memory address; in the case of the `!` operation,
the number directly below the address is the value to be stored.

Here is an example. The following code stores the first ten multiples
of three in an array in memory.
```
.var foo u64 10

0 begin dup 10 < while
  dup dup 8 * foo + over 3 * swap !64
  1+
repeat
```

## Macros

North allows macros to be defined using the `.macro` directive.
The syntax is:
```
.macro <identifier> <tokens> .endm
```
Any sequence of tokens can be placed in the definition of a macro;
they will replace the macro identifier whenever the macro is 
referenced.

Passing arguments to macros is not supported because it is easy
enough to do that by passing arguments on the stack. Additionally,
recursive macros are not supported.

## Include files

North code can be included from multiple files via the `.include`
directive. The name of the file to be included must be a string.
```
.include <name of file>
```
By default, the file name is taken to be relative to the name
of the file that includes it. Extra search paths can be added
to the compiler via the `-I` flag. These directories are taken
relative to the directory in which the compilation/simulation
command is run. Multiple
search paths need multiple `-I` flags.

## Command line arguments

The `argc` and `argv` words push the argument count and the
argument vector onto the stack. The `argc` word simply
pushes the number, while the `argv` word pushes the address
of first string address. This matches the `argc` and `argv`
parameters passed to a normal C `main` function.

This means `argv @64` leaves the address of the first argument
string (the path of the executable) on the stack. Then `argv @64 @8`
is the first character of the first string.

See [echo.nth](../examples/echo.nth) for an example of how
to use `argv`.

## Functions

Functions can be defined with the `.func` and `.endf` directives.
The syntax is similar to the syntax used for defining macros:
```
.func <identifier> <operations> .endf
```
Functions differ to macros in that their body is not expanded
whenever they are called; rather, execution jumps to the start
of the function's body. Note that since arguments are passed on
the stack, North functions do not use the same calling convention
as C functions. This means that for the moment, it is impossible
to link North code with code from another language. In the future,
a possible goal is to introduce a way to write functions that
are indeed compatible with C functions.

In the function body, the `return` operation can be used to
exit the function before the end of the body. If at the end
of the function body there is no `return`, the operation
is implied.
