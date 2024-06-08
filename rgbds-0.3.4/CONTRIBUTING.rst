Contributing
============

RGBDS was created in the late 90's and has received contributions from several
developers since then. It wouldn't have been possible to get to this point
without their work and, for that reason, it is always open to the contributions
of other people.

Reporting Bugs
--------------

Bug reports are essential to improve RGBDS and they are always welcome. If you
want to report a bug:

1. Make sure that there isn't a similar issue already reported
   `here <https://github.com/rednex/rgbds/issues>`__.

2. Figure out a way of reproducing it reliably.

3. If there is a piece of code that triggers the bug, try to reduce it to the
   smallest file you can.

4. Create a new `issue <https://github.com/rednex/rgbds/issues>`__.

Of course, it may not always be possible to give an accurate bug report, but it
always helps to fix it.

Requesting new features
-----------------------

If you come up with a good idea that could be implemented, you can propose it to
be done.

1. Create a new `issue <https://github.com/rednex/rgbds/issues>`__.

2. Try to be as accurate as possible. Describe what you need and why you need
   it, maybe with examples.

Please understand that the contributors are doing it in their free time, so
simple requests are more likely to catch the interest of a contributor than
complicated ones. If you really need something to be done, and you think you can
implement it yourself, you can always contribute to RGBDS with your own code.

Contributing code
-----------------

If you want to contribute with your own code, whether it is to fix a current
issue or to add something that nobody had requested, you should first consider
if your change is going to be small (and likely to be accepted as-is) or big
(and will have to go through some rework).

Big changes will most likely require some discussion, so open an
`issue <https://github.com/rednex/rgbds/issues>`__ and explain what you want to
do and how you intend to do it. If you already have a prototype, it's always a
good idea to show it. Tests help, too.

If you are going to work on a specific issue that involves a lot of work, it is
always a good idea to leave a message, just in case someone else is interested
but doesn't know that there's someone working on it.

1. Fork this repository.

2. Checkout the ``develop`` branch.

3. Create a new branch to work on. You could still work on ``develop``, but it's
   easier that way.

4. Sign off your commits: ``git commit -s``

5. Follow the Linux kernel coding style, which can be found in the file
   ``Documentation/process/coding-style.rst`` in the Linux kernel repository.
   Note that the coding style isn't writen on stone, if there is a good reason
   to deviate from it, it should be fine.

6. Download the files ``checkpatch.pl``, ``const_structs.checkpatch`` and
   ``spelling.txt`` from the folder ``scripts`` in the Linux kernel repository.

7. To use ``checkpatch.pl`` you can use ``make checkpatch``, which will check
   the coding style of all patches between the current one and the upstream
   code. By default, the Makefile expects the script (and associate files) to be
   located in ``../linux/scripts/``, but you can place them anywhere you like as
   long as you specify it when executing the command:
   ``CHECKPATCH=../path/to/folder make checkpatch``.

8. Create a pull request against the branch ``develop``.

9. Be prepared to get some comments about your code and to modify it. Tip: Use
   ``git rebase -i origin/develop`` to modify chains of commits.
