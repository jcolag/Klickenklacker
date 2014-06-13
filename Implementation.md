RUBE II:  Das Klickenklacker, First Sketch at Implementation
============================================================

The RUBE II interpreter ("klik") supports the following syntax in the
command line.

    klik <sourcefile>
    klik <options> <sourcefile>
    klik <sourcefile> <options>

...where options are a single "word," prefaced by a dash ('`-`'), and are made up of any ordering of the following.

| **Option**    | **Long Option **           | **Example**            | **Description**                                 |
| ------------- |:--------------------------:|:----------------------:| ----------------------------------------------- |
| `-a`          | `--no_ansi`                | `-a`                   | No ANSI signals to clear screen                 |
| `-b` _<num>_  | `--begin_at` _<num>_       | `-b 150`               | Begin animation at specified frame number       |
| `-e` _<num>_  | `--end_at` _<num>_         | `-e 400`               | End program after specified frame number        |
| `-f` _<file>_ | `--filename` _<file>_      | `-f examples/echo.rub` | Program file; explicit `-f` option not required |
| `-h` _<num>_  | `--display-height` _<num>_ | `-h 12`                | Display no more than the first lines            |
| `-q`          | `--quiet`                  | `-q`                   | "Quiet Mode"--does not show animation           |
| `-t` _<num>_  | `--truncate-width` _<num>_ | `-t 78`                | Truncate display lines to a particular width    |
| `-x` _<num>_  | `--width` _<num>_          | `-x 25`                | Sets the width (x-dimension) of the warehouse   |
| `-y` _<num>_  | `--height` _<num>_         | `-y 10`                | Sets the depth (y-dimension) of the warehouse   |

Options can appear in any order.

So, as an example, we can have something like...

    klik -h24 -t80 -x500 -y750 hugeprog.rub

...which will run the RUBE II program in a warehouse measuring 500 by 750 cells, but only display the upper-left screenful.

Example Programs
================

A language implementation is useless without good samples, so the following (which, honestly, were created for testing, so don't expect miracles) are included.

| **File** | **Description**                              |
| -------- | -------------------------------------------- |
| echo.rub | A simple program that parrots keyboard input |
| hi.rub   | Hello, World! - inelegant                    |
| hi2.rub  | Hello, World! - slightly more RUBE-ish       |
| test.rub | Fairly extensive test                        |
| bob.rub  | Not quite 99 Bottles of Beer, but a few bottles of something, at least... |

Random Meanderings
==================

The primary object in _RUBE II_ is the grid of objects, much like in Rube.  Each object in the grid is one of the three types:  _Stationary_, _Motive_, and _Moveable_.

This suggests a preliminary structure for the objects in general.

    struct thing {
     char   display;
     int    type;
     int    pushed;
     int    motivation;
     int    direction;
    }

 - Stationary Objects have pushed and motivation set to zero at all times, resetting each round if necessary.

 - Motive Objects have motivation set to one.  Each cycle, an engine tries to set pushed to one, which will succeed if nothing if there is no conflict.

 - Moveable Objects have motivation and pushed set to zero, but a Motive Object can set pushed to one until the end of the round.

The basic _RUBE II_ resolution algorithm, therefore, is something like...

- save old gridstate
- for each square in grid
  - conflict = no
  - if square+(direction*pushed) is filled
    - check old state of square
    - resolve conflict = (yes/no)
  - if conflict = no
    - square+(direction*pushed) = square
    - point square to square+(direction*pushed)
    - square.pushed = square.motivation

That works, but precedence rules might be better suited...

- create newgrid
- for each square in grid, ordered by precedence
  - point sq to newgrid.square+(direction*pushed)
  - if sq is empty
    - sq = grid.square
  - else if newgrid.square is empty
    - point sq to newgrid.square
    - sq = grid.square
  - else
    - point sq to newgrid.square
    - square.pushed = sq.motivation
    - square.direction = sq.direction
    - choose square on newgrid again
  - sq.pushed = sq.motivation;

Precedence Rules are:

 1.  Stationary Objects
 1.  Motive Objects - Vertical
 1.  Motive Objects - Horizontal
 1.  Moveable Objects - Vertical
 1.  Moveable Objects - Horizontal

Optimizations
-------------

It seems to be much easier to treat gravity independently from the rest of the action.  Not only does it speed up execution, since spaces have no interesting semantics to speak of, but it also serves as a template for other forces (wind, maybe?  slower falling?).

Specific comments on this implementation
----------------------------------------

 - The core code is fairly well-structured, in my opinion, but the "peripheral" code could be better managed as functions pointed to from within the Thing structure.  However, great thought needs to be given to the parameters, since changing every last call for every code change is not an appealing notion.

 - The code, as it stands, is 100% ANSI, as far as I know, except for input.  It makes it through Borland C++ 5.01 (bcc -wall) and gcc/CygWin (gcc -ansi -pedantic) without warnings.  To do so, however, requires that the input command be conditionally compiled, meaning that input is impossible on other platforms (until someone tells me how to poll the input stream on other systems).

Where to go from here
---------------------

 - One thing I would really like to do is to clean up the "Before" and "On Collision" code, and make it a bit more object-oriented, as mentioned in the code, itself.  If that gets done in a reasonable way, it might even be possible to create a "RUBE framework" where the set of objects and rules can be defined externally, rather than hardcoding everything.

 - Color might be a useful visualization technique, changing the color each cycle that an object stays in the same place.

 - It would be really nice if input worked for all the major systems out there.

 - There are one or two other objects that might come in handy for _RUBE II_ programmers, like comparison objects which can block or free neighboring cells.  An addition to that might be to give each crate a weight given its value, making this a scale.

 - Speaking of addition, an "overflow crate" might be nice.

 - Finally, There appears to be a minor bug or two regarding gravity.  The only known such bug is an admitted oversight--I need to add vertical collision code for the ramps, so that objects that fall on a ramp slide down.  It's probably just another line of code for each case, but that shouldn't delay the interpreter any more than it already has, right?

