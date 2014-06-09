Klickenklacker
==============

Das Klickenklacker is the reference interpreter for the RUBE II automaton-like programming language.

Introduction
------------

RUBE II, like its predecessor, is an automaton-like system, where a set of objects are governed by the predetermined laws of "physics."  Once constructed, the RUBE II program executes forever in time units known as _frames_, and so there is no "current-" or "next instruction," in the traditional programming sense.

Frames are not unlike frames in a movie, where each is a step in the action.  Each frame is created from the last through the following a simple process of determining the next location for objects, resolving impending collisions between objects, and committing the resolved movements.

The collision resolution is made up of two parts:  __Precedence rules__ and __recursive effects__.

 - Precedence rules govern which objects and/or directions are more important in a collision:  A higher-precedence object involved in a collision will be allowed to progress, while any other objects will be held in their previous locations.
 - A Recursive Effect is just a fancy term stating that if the resolution of one collision causes another potential collision, the new collision must also be resolved, using the same precedence rules.

Objects and Collisions
======================

Objects fall into three major categories.  In order of their precedence, they are _Stationary Objects_, _Motive Objects_, and _Moveable Objects_.

 - _Stationary Objects_ are items that are immobile throughout the course of the program.  As such, they have the highest precedence of all objects, since they must always be in the same location.  They include structural objects, and data manipulation objects.

 - _Motive Objects_ are objects which can change the location of other (_Moveable_) objects.  They automatically move in a preset direction and, when a Moveable Object (see below) is encountered, "push" that object at the same speed.  These include the so-called _dozers_.

 - _Moveable Objects_ are data.  They can be pushed by _Motive Objects_, and are frequently operated upon by Stationary Objects.  Note that, when being pushed by a Motive Object, the recursive nature of RUBE II implicitly "promotes" the _Moveable Object_ to a _Motive Object_, for purposes of collision resolution.

Within each category of objects (except _Stationary Objects_, for obvious reasons), there are two axes of motion along which the object might move.  The precedence of axes are ordered such that vertical movement is resolved before horizontal movement.  Diagonal movement, if possible, is resolved after the "cardinal" movements have been resolved.

The only remaining precedence issues are along the same axis of motion, so-called _head-to-head_ collisions.  Within this case, there are two subcases.  Each gives both objects the same precedence, but the collision must be handled differently in each case.

 - First, the easy one:  If two already-adjacent objects are trying to enter each other's locations, the resulting collision halts both objects.

 - Alternatively, if two non-adjacent objects are trying to enter the same location, the result is undefined.  One object or the other will be treated as though it has a higher precedence for the purposes of the single collision.

 - Finally, at the bottom of the precedence rules are Moveable Objects which are "transferred" to a new location by a Stationary Object (see the Conveyor Objects, Incline Objects, Winch Object, Swinch Objects, and Gate Object).  In such a collision, the transfer is simply ignored.

Forces
------

RUBE supports a single force, a pseudo-gravity which pulls all non-Stationary Objects downward at a rate of one location per frame.  For purposes of conflict resolution, one can consider each object to have an implicit Motive Object above it, pushing downward.

That is, unless stopped, all objects (other than Stationary) will move downward, one location every frame.

Stationary Objects
------------------

As described before, Stationary Objects have the highest precedence, as
befits their use as "operation nodes" and/or structural features.  Among
them are:

### Structural

Structural Objects are Stationary Objects which do not alter Crates (see Moveable Objects).  At most, they simply change the location of a nearby Object.

| **Object** | **Name**       | **Special Features**                        |
|:----------:| -------------- | ------------------------------------------- |
|    `=`     | Girder         | None                                        |
|    `/`     | Incline Right  | Motive force up 1 to right, left 1 to above |
|    `\`     | Incline Left   | Motive force up 1 to left, right 1 to above |
|    `>`     | Right Conveyor | Motive force right 1 to above               |
|    `<`     | Left Conveyor  | Motive force left 1 to above                |
|    `W`     | Winch Up       | Move below to above                         |
|    `M`     | Winch Down     | Move above to below                         |
|    `V`     | Swinch Up      | Move below to above, change to A            |
|    `A`     | Swinch Down    | Move above to below, change to V            |
|    `,`     | Reflector      | Reverse Dozers (see below) that pass below  |

### Operational

Operational Objects are Stationary Objects which, in addition to being usable as Structural Objects Girder), also make modification to nearby Moveable Objects.

| **Object** | **Name**       | **Special Features**                    |
|:----------:| -------------- | --------------------------------------- |
|    `+`     | Packer         | Adds left to right, sum below           |
|    `-`     | Unpacker       | Subtracts right from left, result below |
|    `:`     | Replicator     | Copy above to below                     |
|    `.`     | Replicate Up   | Copy below to above                     |
|    `K`     | Gate           | Move above to left if less than below, move above to right if greater than below |
|    `F`     | Furnace        | Destroy surrounding                     |

Note that Packers and Unpackers use Modular arithmetic.  Only values `0` through `f` result.

### Communications

Communications Objects act entirely like Girders, but also allow for communications with the outside world.

| **Object** | **Name**      | **Special Features**                          |
|:----------:| ------------- | --------------------------------------------- |
|    `I`     | Input         | Creates input character nibbles below         |
|    `N`     | Output Number | Prints nibble above                           |
|    `C`     | Output Char   | Prints character represented by nibbles above |

It should be noted that a RUBE II program may contain more than one Input Object (I).  Should this happen, then input characters are delivered, each to a different Input Object in succession, from left to right, then top to bottom, like a scan line of a television.  When no more Input Objects are available, the first one is used again.

### Motive Objects

As mentioned above, Motive Objects move under their own power, and have a higher precedence than Moveable Objects, effectively pushing them to where they are (presumably) needed.

At this time, there are only three Motive Objects, plus the implicit Motive Object which handles gravity.

| **Object** | **Name**      | **Special Features** |
|:----------:| ------------- | ---------------------|
|    `(`     | Right Dozer   | Moves Right          |
|    `)`     | Left Dozer    | Moves Left           |
|    `^`     | Rocket        | Moves Up             |

### Moveable Objects

Moveable Objects, as described above, are the data of the RUBE II program.  They can be moved by Motive Objects, and manipulated by Stationary Objects.

| **Object** | **Name**      | **Special Features** |
|:----------:| ------------- | -------------------- |
|  `0`..`9`  | Crate         | Values 0 through 9   |
|  `a`..`f`  | Crate         | Values 10 through 15 |

