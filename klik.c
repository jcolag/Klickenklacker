/* Interpreter for:
 *      RUBE II:  Das Klickenklacker
 * Based on a novel (language) by Christopher A. Pressey.
 * RUBE II Interpreter code by John Colagioia, with some ideas
 * borrowed from the original RUBE interpreter, also by Pressey;
 * released on 28 August 2000.
 * 
 * See impl.txt for various implementation notes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <malloc.h>
#include <string.h>
#ifdef __MSDOS__
#include <mem.h>
#include <dos.h>
#endif                                  /* __MSDOS__ */
#include "klik.h"
#include "io.h"

struct thing **warehouse;               /* The main board */
int     height,                         /* Height of board */
        width,                          /* Width of board */
        ansi,                           /* ANSI terminal? */
        ltrunc,                         /* Line length */
        top;                            /* Screen height */
enum modes mode = NONE;

int main(int argc, char *argv[]) {
    int     cont = 1,
            display = 1,
            tempdisplay,
            option_index = 0;
    unsigned long
            xsize = 0,
            ysize = 0,
            cycleno = 0,
            mincycle = 0,
            maxcycle;
    char    fname[256] = "\000",
            optc = 0;

    static struct option long_options[] = {
        { "no-ansi",        no_argument,       NULL, 'a' },
        { "begin-at",       required_argument, NULL, 'b' },
        { "end-at",         required_argument, NULL, 'e' },
        { "file",           required_argument, NULL, 'f' },
        { "display-height", required_argument, NULL, 'h' },
        { "quiet",          no_argument,       NULL, 'q' },
        { "truncate-width", required_argument, NULL, 't' },
        { "width",          required_argument, NULL, 'x' },
        { "height",         required_argument, NULL, 'y' },
        { NULL,             0,                 NULL, 0 }
    };

    ansi = 1;                           /* Assume ANSI-compatible unless told otherwise. */
    ltrunc = 0;                         /* Assume no line truncation */
    top = 0;                            /* Assume no vertical truncation */
    maxcycle = (unsigned)(-1);          /* Assume no termination */

    /* Get the execution details from the command line */
    while (optc != -1) {
        optc = getopt_long(argc, argv, "ab:e:f:h:qt:x:y:",
                long_options, &option_index);
        switch (optc) {
        case 'a':               /* Not an ANSI terminal? */
            ansi = 0;
            mode = FLAT;
            break;
        case 'b':               /* Delay animation? */
            mincycle = atol(optarg);
            break;
        case 'e':               /* End program early? */
            maxcycle = atol(optarg);
            break;
        case 'f':
            strcpy(fname, optarg);
            break;
        case 'h':               /* Show top only? */
            top = atol(optarg);
            break;
        case 'q':               /* Quiet/No animation? */
            display = 0;
            mode = FLAT;
            break;
        case 't':               /* Change line length */
            ltrunc = atol(optarg);
            break;
        case 'x':               /* Change X size? */
            xsize = atol(optarg);
            if (xsize < 2) {
                fprintf(stderr,
                    "%s:  Warehouse must be at least 2x2 cells!\n",
                    argv[0]);
            }
            break;
        case 'y':               /* Change Y size? */
            ysize = atol(optarg);
            if (ysize < 2) {
                fprintf(stderr,
                    "%s:  Warehouse must be at least 2x2 cells!\n",
                    argv[0]);
            }
            break;
        default:
            break;
        }
    }

    if (strlen(fname) == 0 && optind < argc) {
        strcpy(fname, argv[optind]);
    }
    if (strlen(fname) == 0) {
        fprintf(stderr, "%s:  No filename specified!\n", argv[0]);
        return -1;
    }

    /* Set up screen information */
    mode = guess_screen_mode(mode);
    initialize_screen(mode, &xsize, &ysize);

    /* Allocate the "warehouse" space */
    warehouse = buildWarehouse(xsize, ysize);
    initializeWarehouse(warehouse, width, height);

    /* Other initializations */
    tempdisplay = display;
    if (mincycle && display) {
        display = 0;
    }

    /* Fill the grid from the given file */
    if (readFile(fname, warehouse)) {
        exit(-4);
    }
    fillWarehouse(warehouse, width, height);

    /* A second command-line argument shows the animation */
    if (display) {
        showGrid(warehouse, width, height);
    }
    /* Allow the simulation to run, animating if necessary */
    while (cont) {
        cycle(&warehouse, width, height);
        if (display) {
            sleep(1);
            showGrid(warehouse, width, height);
        }
        if (cycleno >(unsigned)maxcycle) {
            cont = 0;
        }
        if (cycleno == mincycle) {
            display = tempdisplay;
        }
        ++cycleno;
    }

    /* Should we ever exit the loop normally(which would happen if
     * the value in cont would ever get changed), clean up the toys
     */
    condemnWarehouse(warehouse, width);
    warehouse = NULL;
    close_window(mode);
    return 0;
}


/*** Warehouse-maintainance Functions ***/

struct thing **buildWarehouse(int x, int y) {
    /* Dyanamically allocate a grid of the specified size, plus
     * boundary regions.
     */
    struct thing   **grid;
    int              i;

    width = x;
    height = y;
    grid = (struct thing **) malloc(sizeof(struct thing *) *(width + 2));
    for (i=0;i<width+2;i++) {
        grid[i] = (struct thing *) malloc(sizeof(struct thing) *(height + 2));
    }
    return grid;
}


void condemnWarehouse(struct thing **warehouse, int x) {
    /* Deallocate the memory used for a particular grid. */
    int    i;

    for (i=0;i<x+2;i++) {
        free(warehouse[i]);
    }
    free(warehouse);
    return;
}


void initializeWarehouse(struct thing **warehouse, int x, int y) {
    /* Fill in obvious initial data for a RUBE grid. */
    int    i, j;

    for (j=0;j<y+2;j++) {
        for (i=0;i<x+2;i++) {
            warehouse[i][j].display = ' ';
            warehouse[i][j].freefall = 0;
        }
    }
    fillWarehouse(warehouse, x, y);
    return;
}


void fillWarehouse(struct thing **warehouse, int x, int y) {
    /* Complete the data for each cell, based on the object currently
     * assigned to the cell.
     */
    struct thing   *cell;
    int         i, j;

    for (i=0;i<x;i++) {
        for (j=y-1;j>-1;j--) {
        /* Examining each cell... */
            cell = &warehouse[i][j];
            cell->checked = 0;              /* No cell is checked to start */
            switch (cell->display) {
            case '=': case '/': case '\\': case '>':
            case '<': case 'W': case  'V': case 'A':
            case ',': case '+': case  '-': case ':':
            case '.': case 'K': case  'F': case 'I':
            case 'N': case 'C': case  'M':
                /* Stationary Objects never move */
                cell->direction = STOP;
                cell->motivation = 0;
                cell->type = STA;
                cell->freefall = 0;
                #if !defined(__MSDOS__) && !defined(USE_CURSES)
                /* In non-DOS, non-curses systems, 'I'
                 * isn't going to work.  We should still
                 * accept it in the program, though,
                 * for portability.
                 */
                if (cell->display == 'I') {
                    fprintf(stderr,
                        "Warning:  Input will not function on this system\n");
                }
                #endif                  /* not __MSDOS__ or USE_CURSES */
                break;
                /* Assign data to the "movers and shakers" */
            case '(':
                cell->direction = RIGHT;
                cell->motivation = 1;
                cell->type = MOT;
                break;
            case ')':
                cell->direction = LEFT;
                cell->motivation = 1;
                cell->type = MOT;
                break;
            case '^':
                cell->direction = UP;
                cell->motivation = 1;
                cell->type = MOT;
                break;
            case ' ':
                cell->direction = DOWN;
                cell->motivation = 1;
                cell->type = MOT;
                cell->freefall = 0;
                break;
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
            case '8': case '9': case 'a': case 'b':
            case 'c': case 'd': case 'e': case 'f':
                /* "Crates"--or Moveable Objects--can
                 * move, but are immoble to start.
                 */
                cell->direction = STOP;
                cell->motivation = 0;
                cell->type = MOV;
                break;
            default:
                if (!isspace(cell->display)) {
                    fprintf(stderr, "Invalid character: %c\n",
                        cell->display);
                }
                break;
            }
            /* Each Object that moves really "pushes itself." */
            cell->pushed = cell->motivation;
            /* Unsupported Objects deal with gravity. */
            if (warehouse[i][j+1].display == ' ' && cell->type != STA) {
                cell->freefall = 1;
            }
            /* Things that fall onto other things no longer worry
             * about gravity.
             */
            if (cell->freefall && warehouse[i][j+1].display != ' ' &&
                !warehouse[i][j+1].freefall) {
                cell->freefall = 0;
            }
            /* The one exception to gravity is the Rocket, which
             * must go up, instead of down.
             */
            if (cell->display == '^') {
                cell->freefall = 0;
            }
        }
    }
    return;
}


/*** Initialization Functions ***/

int readFile(char *filename, struct thing **warehouse) {
    /* Relatively straightforward function which opens a file, and places
     * the data into the Warehouse grid, truncating lines where it becomes
     * necessary, and ignoring any excess lines.
     */
    FILE   *infile;
    int     x, y, count;
    char    ch;

    /* Obvious file stuff */
    infile = fopen(filename, "r");
    if (infile == NULL) {
        perror(filename);
        return -1;
    }
    x = y = 1;

    /* Start reading, character by character */
    while (!feof(infile) &&(y < height + 2)) {
        fscanf(infile, "%c", &ch);
        /* Lines cannot be longer than the grid is wide */
        if (x > width) {
            fprintf(stderr, "Warning:  %s, Line #%d truncated", filename, y);
            /* "Carriage Return," and then search for the next line */
            count = 0;
            while (ch != '\n') {
                fscanf(infile, "%c", &ch);
                ++count;
            }
            /* Tell user how many characters were lost */
            fprintf(stderr, " by %d characters.\n", count);
        }
        /* Execute a pseudo-"Carriage Return" on a newline */
        if (ch == '\n') {
            x = 1;
            ++y;
        } else {
        /* Otherwise, just store the character */
            warehouse[x][y].display = ch;
            ++x;
        }
    }
    /* Can't have more lines than there are in the grid */
    if (y > height + 1) {
        /* Provide a count of how many lines are ignored */
        count = 0;
        do {
            do {
                fscanf(infile, "%c", &ch);
            } while (!feof(infile) && ch != '\n');
            ++count;
        } while (!feof(infile));
        fprintf(stderr, "Warning:  %d lines truncated from file.\n", count);
    }

    fclose(infile);
    return 0;
}


void showGrid(struct thing **warehouse, int x, int y) {
    /* Print the warehouse contents to the screen. */
    int    i, j;

    /* Clear the screen */
    clear_screen(mode);

    /* If horizontal/vertical size is limited, change it here */
    if (ltrunc && ltrunc < x) {
        x = ltrunc;
    }
    if (top && top < y) {
        y = top;
    }

    /* Run through the grid, printing each character */
    for (j=1;j<=y;j++) {
        if (!ansi) {
            printf("\n");
        }
        for (i=1;i<=x;i++) {
            show_character(mode, warehouse, i, j);
        }
        if (ltrunc != 0 && x >= ltrunc) {
            printf("\n");
        }
    }
    dump_output(mode, '\000');
    return;
}


/*** RUBE Activity Functions ***/

int moveThing(struct thing **new, struct thing **old, int x, int y) {
    /* Relocate an object based on its direction and speed. */
    struct thing   *object;
    int             dir, mot,
                    i, j;

    /* Set the starting point */
    i = x;
    j = y;
    object = &(old[x][y]);

    /* Quick optimization:  With gravity handled with the .freefall
     * field(see below), spaces don't have to do anything, so we can
     * ignore them.
     */
    if (object->display == ' ') {
        return 0;
    }

    /* Stationary objects don't move.  Ever. */
    if (object->type == STA) {
        memcpy(&(new[i][j]), object, sizeof(struct thing));
        return 0;
    }

    /* An object in freefall acts like an object being pushed downward,
     * so just treat it as if it is moving downward.
     */
    if (object->freefall) {
        mot = GRAV;
        dir = DOWN;
        if (new[x][y + mot].display == ' ') {
            j = y + mot;
        }
    } else {
    /* Otherwise, change the coordinates based on the direction and
     * velocity of the object in question.  Nothing fancy.
     */
        mot = object->pushed;
        dir = object->direction;
        if (dir == LEFT) {
            i = x - mot;
        } else if (dir == RIGHT) {
            i = x + mot;
        } else if (dir == UP) {
            j = y - mot;
        } else if (dir == DOWN) {
            j = y + mot;
        } else {
            /* Do Nothing */;
        }
    }

    /* If a motive object is about to move horizontally beneath a
     * reflecting object, then change the "dozer," rather than moving
     * it.  Change the direction, too.
     */
    if ((new[i][j-1].display == ',') && (object->type == MOT) && (dir % 2 != 0)) {
        dir = (dir==LEFT)?RIGHT:LEFT;
        object->display = (object->display=='(')?')':'(';
        i = x;
        j = y;
    } else if ((dir % 2 != 0) && !(object->freefall) &&(new[i][j].display == ' ') &&
   (new[i][j+1].display == ' ')) {
    /* An object that has become unsupported should enter freefall. */
        j += GRAV;
        object->freefall = 1;
    } else if ((new[i][j].display == '/') &&(dir == RIGHT) &&(new[i][j-1].display == ' ')) {
    /* Objects colliding with ramps will be pushed upwards.  Technically,
     * it would be much nicer if ramps and similar ideas were handled
     * with an "OnCollision" function accessible through a function
     * pointer.  I was going to do it that way, but could not find a
     * usable parameter list that was still readable.
     */
        j -= 1;
    } else if ((new[i][j].display == '\\') &&(dir == LEFT) &&(new[i][j-1].display == ' ')) {
        j -= 1;
    }

    /* Collisions are special cases which can only occur if any
     * movement is about to happen and the destination cell is full.
     */
    if ((i != x || j != y) &&(old[i][j].display != ' ')) {
        /* If the object in the destination cell can still be moved */
        if (old[i][j].checked == 0) {
            int itmp, jtmp, olddir, oldmot;

            /* Save the old heading */
            olddir = old[i][j].direction;
            oldmot = old[i][j].pushed;
            /* Replace its heading with ours */
            old[i][j].pushed = mot;
            old[i][j].direction = dir;
            itmp = i;
            jtmp = j;
            /* Make sure we haven't already snuck through */
            if (new[x][y].display == object->display) {
                new[x][y].display = ' ';
            }
            /* Push the thing that's in the way */
            if (!moveThing(new, old, i, j)) {
                /* If it failed, we can't move */
                i = x;
                j = y;
            }
            /* Restore the heading to the object we just shoved */
            old[itmp][jtmp].pushed = oldmot;
            old[itmp][jtmp].direction = olddir;
        } else {
        /* If we're not allowed to move the object in the way, then
         * we can't move, ourselves.
         */
            i = x;
            j = y;
        }
    }

    /* Transfer the data from the previous "frame" to the new. */
    memcpy(&(new[i][j]), object, sizeof(struct thing));
    /* Only return a 1 if we moved */
    if (i == x && j == y) {
        return 0;
    }
    if (object->type == MOT) {
        object->checked = 0;
    } else {
        object->checked = 1;
    }
    return 1;
}


int priority(struct thing *object) {
    /* Return the priority of a particular object.  Bigger numbers
     * represent higher priority objects.
     */
    int    pri;

    /* An imaginary object doesn't have a priority. */
    if (object == NULL) {
        return -1;
    }
    /* Stationary objects cannot be moved.  Ever.  Therefore, they are
     * the highest priority of all.
     */
    if (object->type == STA) {
        pri = 5;
    } else if (object->type == MOT) {
    /* Motive Objects are next, with vertical movement before the
     * horizontal.
     */
        if (object->direction % 2 && !object->freefall) {
            pri = 3;
        } else {
            pri = 4;
        }
    } else if (object->type == MOV) {
    /* Finally, we have the Moveable Objects/Crates. */
        if (object->direction % 2 && !object->freefall) {
            pri = 1;
        } else {
            pri = 2;
        }
    } else {
    /* Anything else is the lowest priority. */
        pri = 0;
    }
    return pri;
}


void cycle(struct thing ***warehouse, int x, int y) {
    /* Move all the objects in the grid. */
    struct thing   **temp,
        *object, *tempobj;
    int          i, j, pri, tempval, tempval2, value;

    /* Create a temporary grid in which we can place objects. */
    temp = buildWarehouse(x, y);
    initializeWarehouse(temp, x, y);

    /* For each cell in the original grid, handle their "special
     * traits."  This really should be handled with a more object-
     * oriented mentality.  I was originally going to put a function
     * pointer into the Thing structure, but couldn't decide on the
     * right parameters.
     */
    for (j=1;j<y;j++) {
        for (i=1;i<x;i++) {
            object = &(*warehouse)[i][j];
            switch (object->display) {
                /* Conveyor belts move the object immediately above
                 * one cell in the appropriate direction.
                 */
                case '>':
                    tempobj = &((*warehouse)[i][j-1]);
                    if (tempobj->display != ' ') {
                        tempobj->direction = RIGHT;
                        tempobj->pushed = 1;
                    }
                    break;
                case '<':
                    tempobj = &((*warehouse)[i][j-1]);
                    if (tempobj->display != ' ') {
                        tempobj->direction = LEFT;
                        tempobj->pushed = 1;
                    }
                    break;
                    /* Arithmetic operators, "packing" and "unpacking,"
                     * take a crate on either side, and place the result
                     * immediately underneath, assuming there is space.
                     * Of course, it is a destructive calculation.
                     */
                case '+':
                    /* Check the left digit. */
                    tempobj = &((*warehouse)[i-1][j]);
                    if (isxdigit(tempobj->display)) {
                        /* Account for hex digits */
                        if (isdigit(tempobj->display)) {
                            tempval = tempobj->display - '0';
                        } else {
                            tempval = tempobj->display - 'a' + 10;
                        }
                        /* If successful so far, check the
                         * right digit.
                         */
                        tempobj = &((*warehouse)[i+1][j]);
                        if (isxdigit(tempobj->display)) {
                            if (isdigit(tempobj->display)) {
                                tempval += tempobj->display - '0';
                            } else {
                                tempval += tempobj->display - 'a' + 10;
                            }
                            tempobj->display = ' ';
                           (*warehouse)[i-1][j].display = ' ';
                            /* No overflow in RUBE--just
                             * give the remainder.
                             */
                            tempval %= 16;
                            tempobj = &((*warehouse)[i][j+1]);
                            if (tempval < 10) {
                                tempobj->display = tempval + '0';
                            } else {
                                tempobj->display = tempval - 10 + 'a';
                            }
                        }
                    }
                    break;
                case '-':
                    tempobj = &((*warehouse)[i-1][j]);
                    if (isxdigit(tempobj->display)) {
                        if (isdigit(tempobj->display)) {
                            tempval = tempobj->display - '0';
                        } else {
                            tempval = tempobj->display - 'a' + 10;
                        }
                        tempobj = &((*warehouse)[i+1][j]);
                        if (isxdigit(tempobj->display)) {
                            if (isdigit(tempobj->display)) {
                                tempval -= tempobj->display - '0';
                            } else {
                                tempval -= tempobj->display - 'a' + 10;
                            }
                            tempobj->display = ' ';
                           (*warehouse)[i-1][j].display = ' ';
                            tempval = (tempval + 16) % 16;
                            tempobj = &((*warehouse)[i][j+1]);
                            if (tempval < 10) {
                                tempobj->display = tempval + '0';
                            } else {
                                tempobj->display = tempval - 10 + 'a';
                            }
                        }
                    }
                    break;
                    /* Replicators, above-to-below, and below-to-above. */
                case ':':
                    tempobj = &((*warehouse)[i][j-1]);
                    if (tempobj->type == MOV &&(*warehouse)[i][j+1].display == ' ') {
                        memcpy(&((*warehouse)[i][j+1]), tempobj,
                            sizeof(struct thing));
                    }
                    break;
                case '.':
                    tempobj = &((*warehouse)[i][j+1]);
                    if (tempobj->type == MOV &&(*warehouse)[i][j-1].display == ' ') {
                        memcpy(&((*warehouse)[i][j-1]), tempobj,
                            sizeof(struct thing));
                    }
                    break;
                    /* So-called winches and "swinches," each works like
                     * a replicator, except that they move rather than
                     * copy, and a swinch transforms to the other
                     * direction each cycle.
                     */
                case 'V':
                    object->display = 'A';
                    /* No break; -- Fall through! to next case */
                case 'W':
                    tempobj = &((*warehouse)[i][j-1]);
                    if (tempobj->type == MOV &&(*warehouse)[i][j+1].display == ' ') {
                        memcpy(&((*warehouse)[i][j+1]), tempobj,
                            sizeof(struct thing));
                        tempobj->display = ' ';
                    }
                    break;
                case 'A':
                    object->display = 'V';
                    /* No break; -- Fall through! to next case */
                case 'M':
                    tempobj = &((*warehouse)[i][j+1]);
                    if (tempobj->type == MOV &&(*warehouse)[i][j-1].display == ' ') {
                        memcpy(&((*warehouse)[i][j-1]), tempobj,
                            sizeof(struct thing));
                        tempobj->display = ' ';
                    }
                    break;
                    /* The Gate is the generic comparator.  The crate
                     * above is compared with the crate below.  If the
                     * first crate(the one above) is less, it is moved
                     * to the left of the gate.  If greater, it is moved
                     * to the right of the gate.  Otherwise, it is left
                     * on top.
                     */
                case 'K':
                    tempobj = &((*warehouse)[i][j-1]);
                    if (tempobj->type == MOV) {
                        if (isdigit(tempobj->display)) {
                            tempval = tempobj->display - '0';
                        } else {
                            tempval = tempobj->display - 'a' + 10;
                        }
                        tempobj = &((*warehouse)[i][j+1]);
                        if (isdigit(tempobj->display)) {
                            tempval2 = tempobj->display - '0';
                        } else {
                            tempval2 = tempobj->display - 'a' + 10;
                        }
                        tempobj = &((*warehouse)[i][j-1]);
                        if (tempval > tempval2 &&(*warehouse)[i+1][j].display == ' ') {
                            memcpy(&((*warehouse)[i+1][j]), tempobj,
                                sizeof(struct thing));
                            tempobj->display = ' ';
                        }
                        if (tempval < tempval2 &&(*warehouse)[i-1][j].display == ' ') {
                            memcpy(&((*warehouse)[i-1][j]), tempobj,
                                sizeof(struct thing));
                            tempobj->display = ' ';
                        }
                    }
                    break;
                    /* Numerical output prints the decimal value of the
                     * crate that is above it.
                     */
                case 'N':
                    tempobj = &((*warehouse)[i][j-1]);
                    if (tempobj->type == MOV) {
                        if (isdigit(tempobj->display)) {
                            value = tempobj->display - '0';
                        } else {
                            value = tempobj->display - 'a' + 10;
                        }
                        dump_number(mode, value);
                    }
                    break;
                    /* Character output prints the character whose ASCII
                     * value(in hex) are the crates to the left and
                     * right of it.
                     */
                case 'C':
                    tempobj = &((*warehouse)[i-1][j]);
                    if (tempobj->type == MOV) {
                        if (isdigit(tempobj->display)) {
                            tempval = tempobj->display - '0';
                        } else {
                            tempval = tempobj->display - 'a' + 10;
                        }
                    } else {
                        break;
                    }
                    tempobj = &((*warehouse)[i+1][j]);
                    if (tempobj->type == MOV) {
                        if (isdigit(tempobj->display)) {
                            tempval2 = tempobj->display - '0';
                        } else {
                            tempval2 = tempobj->display - 'a' + 10;
                        }
                    } else {
                        break;
                    }
                    tempval = tempval * 16 + tempval2;
                    dump_output(mode, tempval);
                    (*warehouse)[i-1][j].display = ' ';
                    (*warehouse)[i+1][j].display = ' ';
                    break;
                    /* Character input, as written, will only work under
                     * DOS-like systems, since that's the only place that
                     * kbhit() is available.
                     * Where it is available, each key pressed generates
                     * nibbles to the left and right of the 'I', which
                     * work are equivalent to those used in character
                     * output.
                     */
                case 'I':
                    if (kbhit()) {
                        tempval = getch();
                    } else {
                        break;
                    }
                    tempval2 = tempval % 16;
                    tempval /= 16;
                    if (tempval < 10) {
                        tempval += '0';
                    } else {
                        tempval += 'a' - 10;
                    }
                    tempobj = &((*warehouse)[i-1][j]);
                    if (tempobj->display == ' ') {
                        tempobj->display = tempval;
                    }
                    if (tempval2 < 10) {
                        tempval2 += '0';
                    } else {
                        tempval2 += 'a' - 10;
                    }
                    tempobj = &((*warehouse)[i+1][j]);
                    if (tempobj->display == ' ') {
                        tempobj->display = tempval2;
                    }
                    break;
                    /* The furnace is the cleanup crew, destroying
                     * everything surrounding it.
                     */
                case 'F':
                   (*warehouse)[i-1][j-1].display = ' ';
                   (*warehouse)[ i ][j-1].display = ' ';
                   (*warehouse)[i+1][j-1].display = ' ';
                   (*warehouse)[i-1][ j ].display = ' ';
                   (*warehouse)[i+1][ j ].display = ' ';
                   (*warehouse)[i-1][j+1].display = ' ';
                   (*warehouse)[ i ][j+1].display = ' ';
                   (*warehouse)[i+1][j+1].display = ' ';
                    break;
                default:
                    break;
            }
        }
    }
    /* Now move all the objects in priority order.  Conflicts within a
     * priority are undefined, so the left-to-right, top-to-bottom
     * approach to resolution won't bother anyone.
     */
    for (pri=5;pri>0;pri--) {
        for (j=1;j<y;j++) {
            for (i=1;i<x;i++) {
                if ((priority(&(*warehouse)[i][j]) == pri) &&
                        ((*warehouse)[i][j].checked == 0)) {
                    moveThing(temp, *warehouse, i, j);
                }
            }
        }
    }
    /* Update the new data. */
    fillWarehouse(temp, x, y);
    /* Destroy the old data. */
    condemnWarehouse(*warehouse, x);
    /* Update the pointer. */
    *warehouse = temp;
    return;
}
