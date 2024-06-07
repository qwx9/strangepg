# strange pangenome scale visualization

Huge graph interactive visualization à la [Bandage](https://github.com/rrwick/Bandage).
Currently supports graphs in [GFAv1](https://github.com/GFA-spec/GFA-spec/blob/master/GFA1.md) format.

_Note: this is a work in progress. That includes this README._

## Features

[...]
- Written from scratch in pure C (Plan 9 C: C89 + some extensions)
- High performance graphics with modern and efficient renderer
- Mouse-driven UI navigation
- strawk: embedded graph manipulation language for arbitrary coloring, filtering, etc. based on GFA tags
- Highly responsive: no loading bars, immediate feedback whenever possible
- Highly modular, extensible and cross-platform by design
- Extensible layouting: easy to modify or add layout algorithms (and rebuild)
- Minimal external dependencies, bundling small header-only libraries whenever possible

Licensed under MIT.

#### Alien software

Data structures:
- khash from [klib](https://github.com/attractivechaos/klib)
- [chan](https://github.com/tylertreat/chan)

strawk is based on [onetrueawk](https://github.com/onetrueawk/awk).

Linux graphics:
- sokol_gfx, sokol_app, sokol_nuklear and glue code from [sokol](https://github.com/floooh/sokol)
- [HandmadeMath](https://github.com/StrangeZak/Handmade-Math)
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear)

Used but not bundled:
- GL extension loading via [flextGL](https://github.com/mosra/flextGL)
- GNU Bison or Plan9 yacc(1)

## Installation

### Compilation

```bash
git clone https://github.com/qwx9/strpg
cd strpg
make -j install
```

_-j_ is an optional flag to enable parallel building using all available cores.
This installs the binaries ```strpg``` and ```strawk```.

### Dependencies

Full list (ubuntu package names):

- libbsd0
- libgl-dev
- libglvnd-dev
- libglx-dev
- libmd0
- libx11-dev
- libxau-dev
- libxcb1-dev
- libxcursor-dev
- libxdmcp-dev
- libxext-dev
- libxfixes-dev
- libxi-dev
- libxrandr-dev

Most of these would be installed already on a typical Linux system.

Tested with gcc 11.4.0 and 13.2.0, and clang 14.0.0 and 17.0.6
on void linux and lubuntu.

### Platforms

Currently only Linux and 9front are supported.

## Usage

_strangepg_ requires at least one input file as argument.
It currently supports graphs in GFA format.

```bash
strpg file.gfa

```

### Command-line options

- -b:	white-on-black theme
- -t nth:	number of layouting workers (default: 4)
- -l alg:	select layouting algorithm (see Layouts below) (default: fr)

### Layouts

_strangepg_ ships with a few selectable layouting algorithms currently all based
on a spring model force-directed approach.
Select one with the *-l* option.
Layouting preallocates a pool of worker threads, but depending on the algorithm,
only one worker may be active. The default is 4, changed via the *-t* option:

```bash
# example: parallel FR layout with 8 workers: 
strpg -l pfr -t 8 file.gfa
```

Available layouts:

- _fr_: Fruchterman-Reingold-based layout (default)
- _pfr_: parallel version of the above
- _conga_: fixed node placement on a line by GFA file order
- _random_: random fixed node placement
- _linear_: _fr_ layout with the addition of optionally fixed position nodes (_fx_ and _fy_ GFA segment tags) and initial position (via _mv_ segment tag)
- _pline_: parallel version of the above
- _bo_: similar to _pline_ but using BO tags for initial placement along a horizontal line

### Navigation

Mouse buttons:

- Select object: click left mouse button (click on nothing to unselect)
- Move (pan) view: drag with right mouse button or press a cursor key to jump
- Zoom view: three options:
	* mouse scroll keys
	* hold control + right mouse
	* hold left + right mouse button and drag: pull (drag towards bottom right) to zoom in, push (draw towards top left) to zoom out

Keyboard shortcuts:

- Pause/unpause layout: 'p' key
- Re-layout: 'r' key
- Quit: 'q' key or close window
- Reset initial position: 'escape' key
- Arrow keys: move view by screenful in cardinal direction

9front only:
- Draw labels: 'l' key
- Draw nodes as arrows according to direction: 'a' key


### Interaction

_strangepg_ provides a text prompt for interaction with the graph by issuing commands.
Those affect the current state of the graph by accessing
node and edge data and properties.
They are effectively [_awk_](https://awk.dev) one-liners.

#### Examples

Color every second node in red:
```awk
for(i in node) if(i % 2 == 1) nodecolor(i, red)
```

Color nodes with sequence length > 50:
```awk
for(i in node) if(LN[i] > 50) nodecolor(i, red)
```

Color based on if-elseif-else pattern:
```awk
for(i in LN) nodecolor(i, LN[i] > 200 ? red : LN[i] > 50 ? green : blue)
```

Color nodes with labels matching a regexp:
```awk
for(i in node) if(i ~ /chr13/) nodecolor(i, red)
```

#### Variables

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

#### Expressions

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
which will be addressed in future.
Anything affecting how a node or edge is displayed
must instead be done via the provided built-in functions.

#### Built-in functions and variables

Functions:
```awk
nodecolor(label, color)	# change node color
fixnode(label, x, y)	# freeze node position in space (for "linear", "pline" layouts only)
fixnodex(label, x)	# set initial x position without freezing (for "linear", "pline", "bo" layouts)
```

- nodecolor(_label_, _color_): change a node's color
	* _label_: GFA segment label
	* _color_: RGB24 color in hexadecimal (0xRRGGBB) passed as a string
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

#### User definitions

_strangepg_ essentially runs and communicates with a backgrounded awk instance.
As such any awk single-line expression valid within the default pattern can be specified.
In other words, variables can be modified or new ones added,
and any existing function may be called (exceptions below).
Currently, functions may not be defined.

#### Changes compared to standard awk

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

[...]

TODO (short term):
- Unsigned 64-bit internal values instead of floating-point
- C-style hexadecimal and octal numeric constants
- Multi-line expressions
- ...

_TODO: turn this around: language features/manpage_

#### This sucks

This is tedious, confusing and over-complicated.
However, it works, which is the most important thing for a first release.
The syntax and interface will be reworked completely over time.
The goal is a vector language with syntax such as:

```R
color[label] = red	# no function call, no quotes
color[condition] = red	# result applied to all elements for which condition is true
color[LN < 150] = red	# LN and color are vectors of equal length
```
... etc.

## Additional compilation settings

### Installation prefix

By default, the installation path is *$HOME/.local/bin*.
To install to a different directory, use the _PREFIX_ make variable:

```bash
# example: install in /usr/local/bin via sudo:
make -j
sudo make PREFIX=/usr/local install
```

### Compiler

To build using a different compiler, eg. clang, use the _CC_ make variable:

```bash
make CC=clang -j
sudo make PREFIX=/usr/local install
```

Tested with clang and gcc only.

### Rebuilding strawk

_GNU bison_ is needed to regenerate some of the files.
To use a different _YACC_ implementation, edit the _makefile_.

```bash
cd strawk
make nuke
make install
```


## 9front

Build with _mk_ instead of _make_ in the usual manner.
Additionally requires [npe](https://git.sr.ht/~ft/npe).
