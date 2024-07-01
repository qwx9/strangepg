# strange graph manipulation language

## Variables

The data available is based on the contents of the input GFA file:

- nodes: label, optionally length and/or other tags
- edges: cigar string and optional tags

Node sequences are currently unused.
Tags may be defined or modified at runtime via the prompt.
All of the tags contained in _S_ (segment) and _L_ (link) records in every input GFA file are saved as tables with the same name, indexed by node label (segments) or pair of node label (links) as such:

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

Notice that for segment _s2_ when both an inlined sequence exists (character string other than '\*') and an _LN_ tag is specified,
the value of _LN_ will be *the actual length of the sequence*,
rather than the value of the tag in the file.

_Note: like in awk, the comma character ',' between brackets has a special meaning
and is encoded internally by the ascii character 0x1c (File Separator)._

## Expressions

Commands are for now essentially _awk_ code. For example:

```awk
for(i in node) nodecolor(i, red)
```

_node_ is a hash table mapping GFA labels to an internal id.

Node selection is currently done by looping through all nodes (or edges)
and selecting them with a conditional expression:

```awk
for(i in node) if(CL[i] == red) nodecolor(i, green)	# color red nodes in green
```

The following expressions are also valid:

```awk
nodecolor("s1", red)	# color s1 node
for(i in node) if(LN[i] < 100 && (CL[i] == green || CL[i] == red)) nodecolor(i, blue)
```

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

Functions:
```awk
nodecolor(label, color)	# change node color
importlayout(file)	# import layout from file
exportlayout(file)	# export layout to file
readcsv(file)		# read csv data from file
fixnode(label, x, y)	# freeze node position in space (for "linear", "pline" layouts only)
fixnodex(label, x)	# set initial x position without freezing (for "linear", "pline", "bo" layouts)
```

- nodecolor(_label_, _color_): change a node's color
	* _label_: GFA segment label
	* _color_: RGB24 color in hexadecimal (0xRRGGBB) passed as a string
- readcsv(_file_): read node tags from csv file
	* _file_: file path, string
- importlayout(_file_): set node positions and angles from file
	* _file_: file path, string
- exportlayout(_file_): write node positions and angles to file
	* _file_: file path, string
- fixnode(_label_, _x_, _y_): set and freeze node position (layout-dependent)
	* _label_: GFA segment label
	* _x_: x coordinate, floating-point
	* _y_: y coordinate, floating-point
- fixnode(_label_, _x_): set initial node position (layout-dependent)
	* _label_: GFA segment label
	* _x_: x coordinate, floating-point

Color constants:
```awk
red = "0xff0000"
green = "0x00ff00"
blue = "0x0000ff"
orange = "0xffff00"
yellow = "0xff7f00"
grey = "0xffff00"
brown = "0x7f3f00"
black = "0x0"
white = "0xffffff"
lightblue = "0x1f78b4"
lightorange = "0xff7f00"
lightgreen = "0x33a02c"
lightred = "0xe31a1c"
violet = "0x6a3d9a"
lightbrown = "0xb15928"
paleblue = "0x8080ff"
palegreen = "0x8ec65e"
palered = "0xc76758"
paleorange = "0xca9560"
paleviolet = "0xc893f0"
greybrown = "0x7f5f67"
lightviolet = "0xb160c9"
palebluegreen = "0x5fc69f"
pinkviolet = "0xc96088"
cyan = "0x8dd3c7"
paleyellow = "0xffffb3"
greyblue = "0xbebada"
lightorange2 = "0xfb8072"
lightblue2 = "0x80b1d3"
lightorange2 = "0xfdb462"
lightgreen2 = "0xb3de69"
greypink = "0xfccde5"
lightgrey = "0xd9d9d9"
lightviolet = "0xbc80bd"
greygreen = "0xccebc5"
lightyellow = "0xffed6f"
lightblue2 = "0xa6cee3"
lightgreen3 = "0xb2df8a"
lightpink = "0xfb9a99"
lightorange3 = "0xfdbf6f"
greyviolet = "0xcab2d6"
paleyellow = "0xffff99"
```

## User definitions

_strangepg_ essentially runs and communicates with a backgrounded awk instance.
As such any awk single-line expression valid within the default pattern can be specified.
In other words, variables can be modified or new ones added,
and any existing function may be called (exceptions below).
Currently, functions may not be defined.

## The future

This is tedious and confusing, but is not the end of the story.
The syntax and interface will be reworked completely over time.
The goal is a vector language with syntax such as:

```R
color[label] = red	# no function call, no quotes
color[condition] = red	# result applied to all elements for which condition is true
color[LN < 150] = red	# LN and color are vectors of equal length
```
... etc.

## Changes compared to standard awk

_strawk_, the awk binary currently used by _strangepg_,
is derived from [onetrueawk](https://github.com/onetrueawk/awk),
which is the continuation of standard (bwk) awk.

It has a number of changes:

1. Bit operations have been added,
but as part of the grammar instead of functions such as or(), and(), etc.
Operands are cast to unsigned 64-bit integers first.
Since awk values are still floating-point internally,
*this may yield unexpected results* (until fixed).
They are the following:
- binary operators: | (or), & (and), ^ (xor), << (left shift), >> (right shift)
- unary operators: ` (complement: sucks but overloading ~ makes things too complicated)
- assignment operators: &=, |=, ^=, <<=, >>=

2. The exponentiation operator is **.

3. Precedence rules for bit operators are *not* C-like:
|, ^ and & have *higher* precedence than all comparison operators (==, <, etc.).
This means that the following are now equivalent:

```awk
v = (a & 1) != 0
v = a & 1 != 0		# as opposed to C: a & (1 != 0)
```

4. New function: eval().
It's a horrible hack and will be rendered obsolete by future improvements.
The purpose is to allow awk to parse and evaluate a string as code.
The alternative is to write a parser in awk and use patterns
(with the constraint that user expressions shouldn't be too obscure or tedious).
Doing this for now is much simpler, but it's still nasty.
Errors within evaluated expressions do not cause the program to exit.

5. Removed _getline_, _nextfile_, _system_; removed _--safe_.

6. Different PRNG which handles floating-point; this may be reconsidered later.

TODO (short term):
- Unsigned 64-bit internal values instead of floating-point
- C-style hexadecimal and octal numeric constants
- Multi-line expressions
- Uncrapped eval or better
- ...

## Rebuilding strawk

Only necessary if strawk is modified.
_GNU bison_ is then needed to regenerate some of the files.
To use a different _YACC_ implementation, edit the _makefile_.

```bash
cd strawk
make nuke
make install
```
