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
likewise for `edge`


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

- `black`: `#000000`
- `blue`: `#1f78b4`
- `bluegreen`: `#5fc69f`
- `brightblue`: `#0000ff`
- `brightgreen`: `#00ff00`
- `brightpurple`: `#00ffff`
- `brightred`: `#ff0000`
- `brightyellow`: `#ffff00`
- `brown`: `#b15928`
- `cyan`: `#00ffff`
- `darkgray`: `#3f3f3f`
- `darkgreen`: `#33a02c`
- `darkpink`: `#c96088`
- `darkpurple`: `#6a3d9a`
- `gray`: `#777777`
- `green`: `#8ec65e`
- `greyblue`: `#bebada`
- `greybrown`: `#7f5f67`
- `greygreen`: `#ccebc5`
- `greyred`: `#c76758`
- `lightblue`: `#80b1d3`
- `lightbrown`: `#ca9560`
- `lightgray`: `#c0c0c0`
- `lightgreen`: `#d5f65f`
- `lightgrey`: `#d9d9d9`
- `lightorange`: `#fdb462`
- `lightpink`: `#fccde5`
- `lightpurple`: `#bc80bd`
- `lightred`: `#fb8072`
- `lightteal`: `#8dd3c7`
- `orange`: `#ff7f00`
- `paleblue`: `#a6cee3`
- `palegray`: `#dcdcdc`
- `palegreen`: `#b2df8a`
- `paleorange`: `#fdbf6f`
- `palepurple`: `#c893f0`
- `palered`: `#fb9a99`
- `paleviolet`: `#cab2d6`
- `paleyellow`: `#ffffb3`
- `purple`: `#b160c9`
- `purpleblue`: `#8080ff`
- `red`: `#e31a1c`
- `teal`: `#009696`
- `violet` = `purple`
- `white`: `#ffffff`
- `yellow`: `#ffed6f`


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

#### Other changes

- The exponentiation operator is `**`, `^` is used for bitwise XOR.
- Removed __getline__, __nextfile__, __system__; removed __--safe__:
strawk should never read files or execute system commands itself.
- Different PRNG which handles floating point, instead of using random().

#### TODO

Needed changes:

- Values stored as 32 or 64bit unsigned integers instead of floating point.
- C-style hexadecimal and octal numeric constants.
- eval: function definitions.
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
cd strawk
make nuke
make install
```
