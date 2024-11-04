# strange graph manipulation language

strawk is a fork of [onetrueawk](https://github.com/onetrueawk/awk)
with the purpose of using it as an embedded language.
The main addition is the `eval()` function,
which parses and executes a string.
This enables strawk to be used interactively,
interpreting user commands with persistent changes in its internal state.

In standard awk, one script is interpreted at start up
and from then on it is ran against awk's input,
one line at a time:
it interprets data.
In contrast, while retaining this ability,
strawk can also interpret code by parsing a string as if it were
a continuation of the original script,
executing the new code, then discarding it.


## GFA tags as awk arrays

Variables in strawk are tables of node and edge labels and tags,
loaded from a GFA file, a CSV file, or created on the fly in the prompt.

The data available is based on the contents of the input GFA file:

- nodes: label, optionally length and/or other tags
- edges: cigar string and optional tags

Node sequences are currently unused.
Tags may be defined or modified at runtime via the prompt.
All of the tags contained in `S` (segment) and `L` (link) records in every input GFA file are saved as tables with the same name, indexed by node label (segments) or pair of node label (links) as such:

```awk
nodetag[label] = value
edgetag[label1 dir1,label2 dir2] = value
```

_dir1_ and _dir2_ are orientations of the nodes,
in the same format as _L_ records: "+" for forward, "-" for reverse.

For example, the following GFA file excerpt:
```awk
S	s1	*	LN:i:16
S	s2	ATTA	LN:i:5	CL:z:#581845	FC:i:1
L	s1	+	s2	+	0M	FC:i:1
```
... will result in the following definitions:

```
node["s1"] = 0
node["s2"] = 1
LN["s1"] = 16
LN["s2"] = 4
CL["s2"] = "#581845"
FC["s2"] = 1
edge["s1+","s2+"] = 0
cigar["s1+","s2+"] = "0M"
FC["s1+","s2+"] = 1
```

`node` is a hash table mapping GFA labels to an internal id,
likewise for `edge`.


Notice that for segment `s2` when both an inlined sequence exists (character string other than '\*') and an `LN` tag is specified,
the value of `LN` will be *the actual length of the sequence*,
rather than the value of the tag in the file.

_Note: like in awk, the comma character `,` inside brackets has a special meaning
and is encoded internally by the ascii character `0x1c` (File Separator)._


## Expressions

#### Short-hand notation

There now is a simpler way to express setting a the value of a tag
for all nodes matching a certain condition:

```awk
CL[LN[i] < 100] = green
# is equivalent to:
for(i in CL) if(LN[i] < 100) nodecolor(i, green)
```

The format is the following:

```awk
tag[ condition ] = value
```

In the `condition`, `i` is a node which has a value for `tag`.
Do note that this loops through basically the entire collection of nodes.

Of course, conditional expressions may be more complicated:

```awk
for(i in node) if(LN[i] < 100 && (CL[i] == green || CN[i] == red)) nodecolor(i, blue)
# in shorthand:
CL[LN[i] < 100 && (CL[i] == green || CN[i] == 1)] = blue
```

Or even:

```awk
for(i in node) nodecolor(i, LN[i] < 100 ? blue : LN[i] < 1000 ? green : LN[i] < 10000 ? orange : red)
# can't really express this in short-hand in one go
```

To set the value for all nodes, set the condition to something that is always true:

```awk
tag[ 1 ] = value
```

This syntax will also work to set a single value:

```awk
tag[i == "s33"] = value
```

Note that this loops through the entire collection of nodes.
To set the value of a single tag, this will do it directly:

```awk
tag["s33"] = value
```

However, this will not work as expected for variables with side effects
(see next subsection).
In general, unless the graph is big and this is slow,
prefer the shorthand syntax.
That way, it won't be necessary to remember which function to use.

**It's a hack,** but is extremely convenient and easy to implement,
unlike the language changes required to make this robust and correct.
It will be remodeled in future, but it's a lower priority right now,
and other mechanisms will be added to help here.


#### Tags with side-effects

Because strawk is a separate process and **does not share memory with strangepg**,
tags which have some sort of effect on the graph need to be set with functions.
For example, setting the `CL` tag is expected to change the color
of corresponding nodes, but just changing the value in strawk
won't be communicated to strangepg.
Using a function which tells strangepg that a change occurred will.
In future, strawk will be integrated with strangepg,
and hooks would be implemented which automatically handle this,
but for the time being, certain tags such as color need to be set with a function:

- `CL`: nodecolor(name, value)
- `x0`: initx(name, value)
- `y0`: inity(name, value)
- `fx`: fixx(name, value)
- `fy`: fixy(name, value)

For example:

```awk
nodecolor("s1", red)
```

`nodecolor` sends strangepg information that the `s1` node needs to change color to red.
Without using the function, this does not happen.

Currently, changing a variable's value directly rather than through a function
will not work as expected.
In other words, the following will change the _CL_ value for the node, but not the color of the node on screen:

```awk
CL["s1"] = red
```

This is due to a limitation of the current approach,
which will be addressed in the future.
Anything affecting how a node or edge is displayed
must instead be done via the provided built-in functions.

#### Corner cases

Currently, because of design of awk and some of the changes,
some cases will produce confusing results:

```awk
# if i is not in LN, it will be instanciated with value 0 anyway
CL[LN[i] < 1000] = red
# do this instead:
for(i in LN) if(LN[i] < 1000) nodecolor(i, red)

# this only updates nodes with a CN tag, rather than all of them
CN[CL[i] == red] = 4
# do this instead:
for(i in node) if(CN[i] == red) CN[i] = 4

# this will update *all* nodes with CL
CL["s1"] = red
# do this instead:
nodecolor("s1", red)
# or if it doesn't use such a function, even though it's complicated:
tag[i == "s1"] = red
```

This will be dealt with as soon as possible.


## Built-in functions and variables

The following general functions are provided:

- `importlayout(file)`: load layout from file
- `exportlayout(file)`: save layout to file
- `readcsv(file)`: load data from a CSV file
- `findnode(node)`: lookup a node by name, then center and zoom in on it

In addition, the following tag functions are to be used to update some specific tags
(see section on side-effects):

- `nodecolor(node, color)`: set `CL` tag and change node's color
- `initx(node, x)`: set `x0` tag and initial x coordinate
- `inity(node, y)`: set `y0` tag and initial y coordinate
- `fixx(node, x)`: set `fx` tag and fix x coordinate to constant value
- `fixy(node, x)`: set `fy` tag and fix y coordinate to constant value

The following color constants are provided as well:

- `black`: `0x000000`
- `blue`: `0x1f78b4`
- `bluegreen`: `0x5fc69f`
- `brightblue`: `0x0000ff`
- `brightgreen`: `0x00ff00`
- `brightpurple`: `0x00ffff`
- `brightred`: `0xff0000`
- `brightyellow`: `0xffff00`
- `brown`: `0xb15928`
- `cyan`: `0x7acdcd`
- `darkgray`: `0x3f3f3f`
- `darkgreen`: `0x33a02c`
- `darkpink`: `0xc96088`
- `darkpurple`: `0x6a3d9a`
- `gray`: `0x777777`
- `green`: `0x8ec65e`
- `greyblue`: `0xbebada`
- `greybrown`: `0x7f5f67`
- `greygreen`: `0xccebc5`
- `greyred`: `0xc76758`
- `lightblue`: `0x80b1d3`
- `lightbrown`: `0xca9560`
- `lightgray`: `0xc0c0c0`
- `lightgreen`: `0xd5f65f`
- `lightgrey`: `0xd9d9d9`
- `lightorange`: `0xfdb462`
- `lightpink`: `0xfccde5`
- `lightpurple`: `0xbc80bd`
- `lightred`: `0xfb8072`
- `lightteal`: `0x8dd3c7`
- `orange`: `0xff7f00`
- `paleblue`: `0xa6cee3`
- `palegray`: `0xdcdcdc`
- `palegreen`: `0xb2df8a`
- `paleorange`: `0xfdbf6f`
- `palepurple`: `0xc893f0`
- `palered`: `0xfb9a99`
- `paleviolet`: `0xcab2d6`
- `paleyellow`: `0xffffb3`
- `purple`: `0xb160c9`
- `purpleblue`: `0x8080ff`
- `red`: `0xe31a1c`
- `teal`: `0x009696`
- `violet` = `purple`
- `white`: `0xffffff`
- `yellow`: `0xffed6f`


## User definitions

It is possible to define one's own variables.
In other words, variables can be modified or new ones added,
and any existing function may be called (exceptions below).
However, currently functions cannot be defined.

Note that any `print` output will go to the terminal,
in strangepg's stdout and/or stderr stream.


## Language features

There are a number of additions and differences with standard awk.

#### `eval` function

This is the most important change.
It's a hack that would hopefully no longer be necessary in future.
It's also currently fairly limited; the way it is used in strangepg
is by adding a pattern at the end which just executes an expression as a pattern:

```awk
{ eval("{" $0 "}") }
```

strawk then pretends that there is more code to interpret and adds a branch
to the original parsing tree, then executes the new code.
Once done it discards the new branch and new allocations.
Errors within evaluated expressions do notcause the program to exit.

#### Values stored as 64bit integers

Standard awk stores numerical values as double-precision floating point numbers
(and strings).
To avoid imprecision with large numbers
and allow writing out raw bytes or performing bitwise operations,
the basic type is now 64-bit integers instead.
Values are converted automatically to floating point
if they are floating point constants,
are used in arithmetic expressions involving floats or functions which take or return floats
(such as `exp()`, `log()`, `cos()`, etc.),
or are explicitely cast using the new `float()` function.
`rand()` remains the same, returning floating point numbers in the range [0,1).

#### Bitwise operations

Unlike typical approaches to add these to the language,
involving the addition of new functions,
strawk instead adds them in the language's grammar as new operators:

- Binary operators: | (or), & (and), ^ (xor), << (left shift), >> (right shift)
- Unary operators: ` (complement: sucks but overloading ~ makes things too complicated)
- Assignment operators: &=, |=, ^=, <<=, >>=

The precedence rules are the same as C, **except for `|`, `^` and `&`**,
which have a **higher** precedence than all comparison operators (==, <, etc.).
This means that the following are now equivalent:

```awk
v = (a & 1) != 0
v = a & 1 != 0		# as opposed to C: a & (1 != 0)
```

#### Numerical values, base and binary output

Numbers in hexadecimal and octal are now valid:

```awk
x = 0xdeadbeefcafe
y = -0xcafebabe
printf "0x%x", x
```

The values are 8-byte wide and can overflow.
Note that numbers may have trailing characters (which will be ignored),
just like standard awk.
It is possible to prohibit this, but such a lax rule is easier to handle.
While a numerical value will be set,
the type of the variable or constant will then be a string.
Again like standard awk, if one operand in an expression is a string,
then string comparison will be performed.

```awk
$ strawk 'BEGIN{print 348.5e-1fas_g}'
34.85
$ strawk 'BEGIN{if(3.5a < 17) print "yes"}'
$ 
```

A new `bytes(x)` function has been added to output the raw 64-bit representation
to stdout in native byte order:

```sh
$ strawk 'BEGIN{x=0xdeadbeefcafebabe; bytes(x)}' | xxd
00000000: beba feca efbe adde                      ........
```

This feature's placement is a bit odd, so the interface may change in the future.

#### Other changes

- The exponentiation operator is `**`, `^` is used for bitwise XOR.
- Removed __getline__, __nextfile__, __system__; removed __--safe__:
strawk should never read files or execute system commands itself.
- Removed __ENVIRON__.
- Different PRNG which handles floating point, instead of using random().

#### TODO

Needed changes:

- Pointer objects: variables referencing other variables
- Become a vector language


## Motivation

An obvious question might be why do this rather than use existing languages,
like lua or python, or more lightweight custom languages?
Here is are some of the reasons:

- Avoiding additional dependencies; vendoring realistically only makes sense
for the smaller self-contained languages.
- The syntax for many of them is unsuitable for non-programmers,
yet it's supposed to be one of the main ways to interact with strangepg;
the most lightweight and simple syntax is needed,
and I'd argue that something like lua for example definitely doesn't fit.
- There is a minimal feature set I require and would rather not have to
reimplement myself, such as regular expressions.
- It must be as quick and low effort as possible; I would have to write code
to integrate it and communicate with it; it has to run on a separate thread,
and other constraints.
- It must be cross-platform, and must be able to run at least on 9front
with minimal effort.

In the end, awk fulfills basically all requirements.
All it takes is forking and exec'ing an awk process,
hooking up its stdin and stdout to a pipe,
then just passing user commands to it.
What was missing was an `eval()` function,
which took less than 50 lines of code and a few hours of tinkering to hack in.
It's a typical Unix and Plan9-style solution to such a problem,
and ends up incredibly effective, while adding only around 7k lines of code
and needing no external dependencies.

## Rebuilding strawk

Only necessary if strawk is modified.
_GNU bison_ is then needed to regenerate some of the files.
To use a different _YACC_ implementation, edit the _makefile_.

```bash
$ cd strawk
$ make nuke
$ make install
```
