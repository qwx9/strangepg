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

Full list:

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

