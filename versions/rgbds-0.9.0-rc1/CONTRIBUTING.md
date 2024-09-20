# Contributing

RGBDS was created in the late '90s and has received contributions from several
developers since then. It wouldn't have been possible to get to this point
without their work, and it is always open to the contributions of other people.

## Reporting Bugs

Bug reports are essential to improve RGBDS and they are always welcome. If you
want to report a bug:

1. Make sure that there isn't a similar issue
   [already reported](https://github.com/gbdev/rgbds/issues).
2. Figure out a way of reproducing it reliably.
3. If there is a piece of code that triggers the bug, try to reduce it to the
   smallest file you can.
4. Create a new [issue](https://github.com/gbdev/rgbds/issues).

Of course, it may not always be possible to give an accurate bug report, but it
always helps to fix it.

## Requesting new features

If you come up with a good idea that could be implemented, you can propose it to
be done.

1. Create a new [issue](https://github.com/gbdev/rgbds/issues).
2. Try to be as accurate as possible. Describe what you need and why you need
   it, maybe with examples.

Please understand that the contributors are doing it in their free time, so
simple requests are more likely to catch the interest of a contributor than
complicated ones. If you really need something to be done, and you think you can
implement it yourself, you can always contribute to RGBDS with your own code.

## Contributing code

If you want to contribute with your own code, whether it is to fix a current
issue or to add something that nobody had requested, you should first consider
if your change is going to be small (and likely to be accepted as-is) or big
(and will have to go through some rework).

Big changes will most likely require some discussion, so open an
[issue](https://github.com/gbdev/rgbds/issues) and explain what you want to
do and how you intend to do it. If you already have a prototype, it's always a
good idea to show it. Tests help, too.

If you are going to work on a specific issue that involves a lot of work, it is
always a good idea to leave a message, just in case someone else is interested
but doesn't know that there's someone working on it.

Note that you must contribute all your changes under the MIT License. If you are
just modifying a file, you don't need to do anything (maybe update the copyright
years). If you are adding new files, you need to use the `SPDX-License-Identifier: MIT`
header.

1. Fork this repository.
2. Checkout the `master` branch.
3. Create a new branch to work on. You could still work on `master`, but it's
   easier that way.
4. Compile your changes with `make develop` instead of just `make`. This
   target checks for additional warnings. Your patches shouldn't introduce any
   new warning (but it may be possible to remove some warning checks if it makes
   the code much easier).
5. Test your changes by running `./run-tests.sh` in the `test` directory.
   (You must run `./fetch-test-deps.sh` first; if you forget to, the test suite will fail and remind you mid-way.)
5. Format your changes according to `clang-format`, which will reformat the
   coding style according to our standards defined in `.clang-format`.
6. Create a pull request against the branch `master`.
7. Be prepared to get some comments about your code and to modify it. Tip: Use
   `git rebase -i origin/master` to modify chains of commits.

## Adding a test

The test suite is a little ad-hoc, so the way tests work is different for each program being tested.

Feel free to modify how the test scripts work, if the thing you want to test doesn't fit the existing scheme(s).

### RGBASM

Each `.asm` file corresponds to one test.
RGBASM will be invoked on the `.asm` file with all warnings enabled.

If a `.out` file exists, RGBASM's output (`print`, `println`, etc.) must match its contents.
If a `.err` file exists, RGBASM's error output (`warn`, errors, etc.) must match its contents.

If a `.out.bin` file exists, the object file will be linked, and the generated ROM truncated to the length of the `.out.bin` file.
After that, the ROM must match the `.out.bin` file.

### RGBLINK

Each `.asm` file corresponds to one test, or one *set* of tests.

All tests begin by assembling the `.asm` file into an object file, which will be linked in various ways depending on the test.

#### Simple tests

These simply check that RGBLINK's output matches some expected output.

A `.out` file **must** exist, and RGBLINK's output must match that file's contents.

Additionally, if a `.out.bin` file exists, the `.gb` file generated by RGBLINK must match it.

#### Linker script tests

These allow applying various linker scripts to the same object file.
If one or more `.link` files exist, whose names start the same as the `.asm` file, then each of those files correspond to one test.

Each `.link` linker script **must** be accompanied by a `.out` file, and RGBLINK's output must match that file's contents when passed the corresponding linker script.

#### Variant tests

These allow testing RGBLINK's `-d`, `-t`, and `-w` flags.
If one or more <code>-<var>&lt;flag&gt;</var>.out</code> or <code>-no-<var>&lt;flag&gt;</var>.out</code> files exist, then each of them corresponds to one test.

The object file will be linked with and without said flag, respectively; and in each case, RGBLINK's output must match the `.out` file's contents.

### RGBFIX

Each `.flags` file corresponds to one test.
Each one is a text file whose first line contains flags to pass to RGBFIX.
(There may be more lines, which will be ignored; they can serve as comments to explain what the test is about.)

RGBFIX will be invoked on the `.bin` file if it exists, or else on default-input.bin.

If no `.err` file exists, RGBFIX is simply expected to be able to process the file normally.
If one *does* exist, RGBFIX's return status is ignored, but its output **must** match the `.err` file's contents.

Additionally, if a `.gb` file exists, the output of RGBFIX must match the `.gb`.

### RGBGFX

There are three kinds of test.

#### Simple tests

Each `.png` file corresponds to one test.
RGBGFX will be invoked on the file.
If a `.flags` file exists, it will be used as part of the RGBGFX invocation (<code>@<var>&lt;file&gt;</var>.flags</code>).

If `.out.1bpp`, `.out.2bpp`, `.out.pal`, `.out.tilemap`, `.out.attrmap`, or `.out.palmap` files exist, RGBGFX will create the corresponding kind of output, which must match the file's contents.
Multiple kinds of output may be tested for the same input.

If no `.err` file exists, RGBGFX is simply expected to be able to process the file normally.
If one *does* exist, RGBGFX's return status is ignored, but its output **must** match the `.err` file's contents.

#### Reverse tests

Each `.1bpp` or `.2bpp` file corresponds to one test.
RGBGFX will be invoked on the file with `-r 1` for reverse mode, then invoked on the output without `-r 1`.
The round-trip output must match the input file's contents.
If a `.flags` file exists, it will be used as part of the RGBGFX invocation (<code>@<var>&lt;file&gt;</var>.flags</code>).

#### Random seed tests

Each `seed*.bin` file corresponds to one test.
Each one is a binary RNG file which is passed to the `rgbgfx_test` program.

### Downstream projects

1. Make sure the downstream project supports <code>make <var>&lt;target&gt;</var> RGBDS=<var>&lt;path/to/RGBDS/&gt;</var></code>.
   While the test suite supports any Make target name, only [Make](//gnu.org/software/make) is currently supported, and the Makefile must support a `RGBDS` variable to use a non-system RGBDS directory.

   Also, only projects hosted on GitHub are currently supported.
2. Add the project to `test/fetch-test-deps.sh`: add a new `action` line at the bottom, following the existing pattern:
   
   ```sh
   action  <owner>  <repo>  <date of last commit>  <hash of last commit>
   ```

   (The date is used to avoid fetching too much history when cloning the repositories.)
3. Add the project to `test/run-tests.sh`: add a new `test_downstream` line at the bottom, following the existing pattern:

   ```sh
   test_downstream  <owner>  <repo>  <makefile target>  <build file>  <sha1 hash of build file>
   ```