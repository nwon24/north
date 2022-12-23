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
operators (such as `if`) take another other than `0` as 'true'.

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

