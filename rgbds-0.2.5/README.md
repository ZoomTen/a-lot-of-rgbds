# RGBDS

RGBDS (Rednex Game Boy Development System) is a free assembler/linker package
for the Game Boy and Game Boy Color. It consists of:

  - rgbasm  (assembler)
  - rgblink (linker)
  - rgbfix  (checksum/header fixer)
  - rgbgfx  (PNG‐to‐Game Boy graphics converter)

rgbds-linux is a fork of the original RGBDS which aims to make the programs
more like other UNIX tools.


## Installing RGBDS (UNIX)

RGBDS requires libpng and pkg-config to be installed.

On Mac OS X, install them with [Homebrew](http://brew.sh/). On other Unixes,
use the built-in package manager.

You can test if they're installed by running `pkg-config --cflags libpng`:
if the output is a path, then you're good, and if it outputs an error then
you need to install them via a package manager.

To build the programs on a UNIX or UNIX-like system, just run in your terminal:

```sh
make
```

Then to install the compiled programs and manual pages, run (with appropriate
privileges):

```sh
make install
```

After installation, you can read the manuals with the man(1) command. E.g.,

```sh
man 1 rgbasm
```

Note: the variables described below can affect installation behavior when given
on the make command line. For example, to install rgbds in your home directory
instead of systemwide, run the following:

```sh
mkdir -p $HOME/{bin,man/man1,man/man7}
make install PREFIX=$HOME
```

`PREFIX`: Location where RGBDS will be installed. Defaults to /usr/local.

`BINPREFIX`: Location where the RGBDS programs will be installed. Defaults
to ${PREFIX}/bin.

`MANPREFIX`: Location where the RGBDS man pages will be installed. Defaults
to ${PREFIX}/man.

`Q`: Whether to quiet the build or not. To make the build more verbose, clear
this variable. Defaults to @.


## Installing RGBDS (Windows)

Windows builds are available here: https://github.com/bentley/rgbds/releases

Copy the .exe files to C:\Windows\ or similar.
