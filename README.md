# strange pangenome scale visualization

## Installation

### Compilation

```bash
git clone https://github.com/qwx9/strpg
cd strpg
make -j install
```

_-j_ is an optional flag to enable parallel building using all available cores.

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

### Navigation

Mouse buttons:

- Select object: click left mouse button (click on nothing to unselect)
- Move (pan) view: drag with right mouse button or press a cursor key to jump
- Zoom view: drag with left + right mouse button together: pull (drag towards bottom right) to zoom in, push (draw towards top left) to zoom out

Keyboard shortcuts:

- Pause/unpause layout: 'p' key
- Re-layout: 'r' key
- Quit: 'q' key or close window
- Arrow keys: move view by screenful in a cardinal direction


### Interaction

_strangepg_ provides a text prompt for interaction with the graph by issuing commands.
Those affect the current state of the graph by accessing
node and edge data and properties.

#### Examples

Color every second node in red:
```awk
for(i in lnode) if(i % 2 == 1) nodecolor(i, red)
```

Color nodes with sequence length > 50:
```awk
for(i in lnode) if(LN["n",i] > 50) nodecolor(i, red)
```

Color based on if-elseif-else pattern:
```awk
for(i in lnode) nodecolor(i, LN["n",i] > 200 ? red : LN["n",i] > 50 ? green : blue)
```

Color nodes with labels matching a regexp:
```awk
for(i in lnode) if(lnode[i] ~ /chr13/) nodecolor(i, red)
```

#### Variables

The data available is based on the contents of the input GFA file:

- nodes: label, optionally length and/or other tags
- edges: cigar string and optional tags

Node sequences are currently unused.
Tags may be defined or modified at runtime via the prompt.
They are defined per segment (node) or link (edge) to allow overlaps between them; for example, the GFAv1 spec specifies *FC* as a typical optional flag for both segments and links.
Existing tags are encoded as: *two_letter_tagname*["*n|e*", *id*] = *value*.

ids are unique integers 0,1,…,n ∈ ℕ internal to _strangepg_.
Two mapping tables are also exposed to translate between ids and GFA labels:

```awk
lnode[id] = label
node[label] = id
```

For example, the following segment records:
```awk
S	s1	*	LN:i:16
S	s2	ATTA	LN:i:5
```
... will result in the following definitions:
```
lnode[0] = "s1"
lnode[1] = "s2"
node["s1"] = 0
node["s2"] = 1
LN["n",0] = 16
LN["n",1] = 4
```

Notice that for _s2_ if both a sequence exists and an LN tag is specified, only the actual length of the inlined sequence is used.

#### Expressions

Commands are for now essentially _awk_ code. For example:

```awk
for(i in lnode) nodecolor(i, red)
```

#### Built-in functions and variables

Functions:
```awk
nodecolor(id, color)
```

- nodecolor(_id_, _color_): change a node's color
	* _id_: integer node id
	* _color_: RGB24 color in hexadecimal (0xRRGGBB) passed as a string
- fix(_id_, _x_, _y_): set and freeze node position
	* _id_: integer node id
	* _x_: x coordinate, floating-point
	* _y_: y coordinate, floating-point

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

#### This sucks

This is tedious, confusing and over-complicated.
However, it works, which is the most important thing for a first release.
The syntax and interface will be reworked completely over time.
The goal is a vector language with syntax such as:

```R
color[nodelabel] = red	# no function call, no quotes
color[condition] = red	# result applied to all elements for which condition is true
color[LN < 150] = red	# LN and color are vectors of equal length
```
... etc.

## Compilation options

### Installation prefix

By default, the installation path is *$HOME/.local/bin*.
To install to a different directory, use the _PREFIX_ make variable:

```bash
# example: install in /usr/local/bin via sudo:
make -j
sudo make PREFIX=/usr/local install
```

### Compiler settings

To build using a different compiler, eg. clang, use the _CC_ make variable:

```bash
make CC=clang -j
sudo make PREFIX=/usr/local install
```

