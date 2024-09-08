# Strange pangenome scale visualization

Large graph interactive visualization à la [Bandage](https://github.com/rrwick/Bandage).
Currently supports graphs in [GFAv1](https://github.com/GFA-spec/GFA-spec/blob/master/GFA1.md) format.

Sales pitch:
```
Visualizing large graphs in the million node scale and beyond remains a challenge and is relevant to multiple fields of study. In
pangenomics, as databases are continuously enriched by new and high quality assemblies, there is a growing need for tools and
methods tailored to handle extremely large volumes of data. As pangenome sizes multiply, available techniques quickly hit
operational limits in terms of processing time and memory. In particular, visualizing graphs in a general, intuitive and interactive
manner is useful for analysis and interpretation, yet computationally prohibitive at such a scale. The main objective of this
project is the development of strangepg, a new tool and visualization workflow aiming to address these limitations by employing
the combination of offline indexing and graph coarsening with the extensive use of a generic external memory layer. By
offloading the major computational effort to a preprocessing step, the interactive visualizer loads only a small fraction of the
total data and fetches more from disk only on request. The use of external memory ensures that every step can be performed on
commodity hardware for arbitrary sized graphs. strangepg is being implemented in C in a highly modular, portable and
extensible manner, designed to allow substituting different algorithms for most steps with minimal effort, and aims to provide a
general framework for experimentation with layouting and new visualization techniques.
```

<p align="center"><img src="strangelove.png"/></p>

_Note: this is a work in progress.
Please consider this to be a public beta of sorts
and feel free to send bug reports, feature requests or comments.
Thanks!_

## Table of contents

[Features](#features)
[Installation](#installation)
[Usage](#usage)
[Layouting](#layouting)
[Navigation](#navigation)
[Interaction](#interaction)
[Loading tags from CSV files](#csv)
[Additional compilation settings](#compilationsettings)
[Known bugs](#bugs)
[Used and bundled alien software](#bundled)
[9front](#9front)


## <a name="features"></a>Features

- Scaling to arbitrarily large graphs via coarsening (work in progress)
- GFA information loaded in two passes in parallel to start layouting as soon as possible
- High performance graphics with modern and efficient renderer
- Highly modular, extensible and cross-platform by design
- Embedded graph manipulation language based on GFA tags
- Amenable to experimentation with custom layouts
- Highly responsive: no loading bars, immediate feedback whenever possible
- GFA files: no assumptions on order or label types (strings or integers)
- Written from scratch in C with almost no dependencies

Near finished:
- Offline coarsening scheme, a few bugs remain
- Node and edge shaping according to length and other data
- Further graphics performance improvements on Linux
- 3D layouting and navigation
- Additional capabilities in the graph language
- Better documentation, manpages
- Multiple graph handling

Near future:
- Better UI!
- Path handling
- macOS support
- GBZ support

Future:
- Additional annotation overlays
- Online graph coarsening, with no preprocessing step
- Better graph manipulation language: vector language
- IGV-like subviews
- More user-friendly layout specification/implementation

Released under the terms of the MIT license.

## <a name="installation"></a>Installation

Currently only Linux (and 9front) are supported.
A macOS port will arrive as soon as someone kindly sacrifices their laptop
for a while.

Installation can be done from source or via [bioconda](https://bioconda.github.io/).

The binary's name is _strpg_.


#### Bioconda

Install conda, then:

```bash
conda install strangepg
```

#### Compilation

```bash
git clone https://github.com/qwx9/strangepg
cd strangepg
make -j install
```

_-j_ is an optional flag to enable parallel building using all available cores.
This installs the binaries ```strpg``` and ```strawk```,
by default in *$HOME/.local/bin*.
If this directory is not in your $PATH or a different installation directory is desired,
see the `Additional compilation settings` section below.

#### Dependencies

strangepg requires an OpenGL implementation and X11 libraries.
Those are usually already present on typical Linux systems.

Command line for ubuntu (adapt to your system):
```bash
apt install libbsd0 libgl-dev libglvnd-dev libglx-dev libmd0 libx11-dev libxau-dev libxcb1-dev libxcursor-dev libxdmcp-dev libxext-dev libxfixes-dev libxi-dev libxrandr-dev 
```

Tested with gcc 11.4.0 and 13.2.0, and clang 14.0.0 and 17.0.6
on Void Linux and Ubuntu 22.04/24.04.

## <a name="usage"></a>Usage

_strangepg_ requires at least one input file as argument.
It currently supports graphs in GFA format.

```bash
strpg file.gfa
```

Some test examples exist in the `test/` directory.
For example:

```bash
strpg test/03.232.gfa
```

#### Command-line options

- -b:	white-on-black theme
- -c file:	load metadata tags from CSV file (see format below; can be repeated)
- -f file:	load exported layout from file
- -t nth:	number of layouting workers (default: 4)
- -l alg:	select layouting algorithm (see Layouts below) (default: fr)
- -R:	do not reset layout once metadata is done loading

## <a name="layouting"></a>Layouting

_strangepg_ ships with a few selectable layouting algorithms currently all based
on a spring model force-directed approach.
Select one with the *-l* option.
Layouting preallocates a pool of worker threads, but depending on the algorithm,
only one worker may be active. The default is 4, changed via the *-t* option:

```bash
# example: parallel FR layout with 8 workers:
strpg -l pfr -t 8 file.gfa
```

Single-threaded layouts will only use one thread regardless.

Layouting may be paused/unpaused with the 'p' key,
and can be restarted with the 'r' key at will.
Restarting it is cheap; try it if the layout doesn't look good.

Termination conditions for the algorithms are not yet well tuned wrt. the size of
the graphs, and layouting may continue with little noticeable changes.
Use the 'p' key to freeze/unfreeze layouting.

#### Available layouts

- _pfr_: parallelized variant of the Fruchterman-Reingold algorithm (default)
- _fr_: single threaded version of the above
- _conga_: fixed node placement on a line by GFA file order
- _random_: random fixed node placement

Experimental, problem-specific layouts:
- _linear_: _fr_ layout with the addition of optionally fixed position nodes (_fx_ and _fy_ GFA segment tags) and initial position (via _mv_ segment tag)
- _circ_: attempt at a circular version of the above
- _bo_: similar to _pline_ but using BO tags for initial placement along a horizontal line

#### Loading from and saving to file

A pre-existing layout may be loaded with the `-f` flag irrespective of the selected layout algorithm.
Layouts can't yet be selected at runtime or ran partially but they may be interrupted and resumed with the 'p' key.

The current layout may be exported or imported at runtime with the `exportlayout("file")` and `importlayout("file")` functions
(see the Interaction section below).
The output file format is binary.

## <a name="navigation"></a>Navigation

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
- Arrow keys: move view by screenful up/down/left/right

Hovering over an edge or node shows some information on the prompt window.

## <a name="interaction"></a>Interaction

_strangepg_ provides a text prompt for interaction with the graph by issuing commands.
They are effectively [_awk_](https://awk.dev) one-liners.
GFA tags and labels are transformed into tables which can then be used to change the state of the graph in arbitrary ways.
For more in-depth information, check out [the help document](strawk.md).

#### Examples

Color every other node in red:
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

Currently most of the functionality is limited to coloring, but editing the graph is next.
Note that the language is admittedly cumbersome for this purpose and is not final.


## <a name="csv"></a>Loading tags from CSV files

CSV files can be loaded at start up with the '-c' flag or at runtime to feed or modify tags for existing nodes.
The '-c' flag may be used multiple times to load more than one CSV file at startup.
The first column is always reserved for node labels, and all subsequent columns are tags values.
The first line must be a header specifying a tag name for each column.

For example:
```
Node,CN,CO,CL
utig4-142,1,55231,purple
utig4-143,0,53,red
...
```

The name of the first column does not matter.
The `CL` tag is used as a node's color and can thus also be set in this way.
`Color` can also be used.
Nodes labels must refer to existing nodes in the input GFA file.

A CSV file may be loaded at runtime with the `readcsv("file.csv")` command
(see the Interaction section).

*Loading multiple CSV files one after the other is allowed.*
In other words, variables such as color are not reset between files.
CSV files thus needn't be merged together.

#### Format notes

The accepted format here is more stringent than usual.
The implementation is not localized, ie. commas are always field separators.
There are no escape sequences or special quoting rules, ie. quotes are not special characters.
Line breaks within a field are disallowed.
Lines must be terminated with a new line (LF) character, ie. the return character (CR) is not handled.

Each line must have the same number of fields as the header, but fields can be empty.

## <a name="compilationsettings"></a>Additional compilation settings

#### Installation prefix

By default, the installation path is *$HOME/.local/bin*.
To install to a different directory, use the _PREFIX_ make variable:

```bash
# example: install in /usr/local/bin via sudo:
make -j
sudo make PREFIX=/usr/local install
```

#### Compiler

To build using a different compiler, eg. clang, use the _CC_ make variable:

```bash
make CC=clang -j
sudo make PREFIX=/usr/local install
```

Tested with clang and gcc only.

## <a name="bugs"></a>Known bugs

- strawk leaks some memory; awk wasn't meant to be used this way and plugging
  the leaks is not trivial
- currently broken on WSL because of GL initialization errors; not sure why

## <a name="bundled"></a>Used and bundled alien software

Data structures:
- [khashl](https://github.com/attractivechaos/khashl)
- [chan](https://github.com/tylertreat/chan)

Linux graphics:
- sokol_gfx, sokol_app, sokol_nuklear and glue code from [sokol](https://github.com/floooh/sokol)
- [HandmadeMath](https://github.com/StrangeZak/Handmade-Math)
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear)

Used but not bundled:
- GL extension loading via [flextGL](https://github.com/mosra/flextGL)
- GNU Bison or Plan9 yacc(1) for awk

strawk is based on [onetrueawk](https://github.com/onetrueawk/awk).


![](plan.png)

## <a name="9front"></a>9front

Build with _mk_ instead of _make_ in the usual manner.
Additionally requires [npe](https://git.sr.ht/~ft/npe); it's really only required to build khash.
A better solution might exist since SDL2 isn't used at all.
