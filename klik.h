#ifndef _KLIK__H_
#define _KLIK__H_

/* Directional Constants */
#define LEFT   (1)
#define UP     (2)
#define RIGHT  (3)
#define DOWN   (4)
#define STOP   (0)

/* Object Type Constants */
#define STA    (0)
#define MOT    (1)
#define MOV    (2)

/* The force, lowercase 'f' */
#define GRAV   (1)

/* Objects in the warehouse */
struct  thing {
    char   display;                     /* The image displayed */
    int    type;                        /* The kind of object */
    int    pushed;                      /* How fast it must move */
    int    motivation;                  /* How fast it naturally moves */
    int    direction;                   /* Current direction of motion */
    int    freefall;                    /* Does gravity apply? */
    int    checked;                     /* Administrative, to move only once */
};

/* Grid maintenance functions */
struct thing **buildWarehouse(int x, int y);
void condemnWarehouse(struct thing **warehouse, int x);
void initializeWarehouse(struct thing **warehouse, int x, int y);
void fillWarehouse(struct thing **warehouse, int x, int y);

/* Administrative functions */
int readFile(char *filename, struct thing **warehouse);
void showGrid(struct thing **warehouse, int x, int y);

/* Actual RUBE-ing functions */
void cycle(struct thing ***warehouse, int x, int y);
int moveThing(struct thing **new, struct thing **old, int x, int y);
int priority(struct thing *object);

#endif

