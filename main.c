#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VSIZE 14
#define HSIZE 14
#define MINE_CHANCE 10

#define MINE 1 << 6
#define EXPLORED 1 << 5
#define FLAG 1 << 4

#define MINE_CHAR '!'
#define BOOM_CHAR '@'
#define MASK_CHAR '~'
#define CLEARED_CHAR '+'
#define FLAG_CHAR 'F'
#define EMPTY_CHAR '-'

#define SET_MINE(r, c) field[(r)][(c)] |= MINE
#define HAS_MINE(r, c) (field[(r)][(c)] & MINE) != 0
#define EXPLORE(r, c) field[(r)][(c)] |= EXPLORED
#define IS_EXPLORED(r, c) (field[(r)][(c)] & EXPLORED) != 0
#define IS_EMPTY(r, c) nearbymines((r), (c)) == 0
#define SET_FLAG(r, c) field[(r)][(c)] |= IS_EXPLORED((r), (c)) ? 0 : FLAG
#define DEL_FLAG(r, c) field[(r)][(c)] &= ~FLAG
#define HAS_FLAG(r, c) (field[(r)][(c)] & FLAG) != 0

#define PRINT_MINE() printf("%-3c", MINE_CHAR)
#define PRINT_NUM_NEARBY_MINES(r, c) printf("%-3d", nearbymines((r), (c)))
#define PRINT_EMPTY() printf("%-3c", EMPTY_CHAR)
#define PRINT_MASK() printf("%c%c%c", MASK_CHAR, MASK_CHAR, MASK_CHAR)
#define PRINT_CLEARED() printf("%-3c", CLEARED_CHAR)
#define PRINT_BOOM() printf("%-3c", BOOM_CHAR)
#define PRINT_FLAG() printf("%-3c", FLAG_CHAR)

#define BAD_FLAG 2
#define DONE -1

#define ACTION_EXPLORE 1
#define ACTION_FLAG 2
#define ACTION_DELFLAG 3

static char field[VSIZE][HSIZE];

void printfield(void (*f)(int, int)) {
  int r, c;

  printf("\n    ");
  for (c = 0; c < HSIZE; c++)
    printf("%-3d", c+1);
  printf("\n   .");
  for (c = 0; c < HSIZE; c++)
    printf("---", c+1);
  printf(".\n");

  for (r = 0; r < VSIZE; r++) {
    printf("%3d|", r+1);
    for (c = 0; c < HSIZE; c++)
      f(r, c);
    printf("| %d\n", r+1);
  }
  printf("   '");
  for (c = 0; c < HSIZE; c++)
    printf("---", c+1);
  printf("'\n    ");
  for (c = 0; c < HSIZE; c++)
    printf("%-3d", c+1);
  printf("\n\n");
}

int nearbymines(int, int);

void raw(int r, int c) {
  if (HAS_MINE(r, c))
    IS_EXPLORED(r, c) ? PRINT_BOOM() : PRINT_MINE();
  else
    IS_EMPTY(r, c) ? PRINT_EMPTY() : PRINT_NUM_NEARBY_MINES(r, c);
}

void cleared(int r, int c) {
  if (HAS_MINE(r, c))
    PRINT_CLEARED();
  else
    IS_EMPTY(r, c) ? PRINT_EMPTY() : PRINT_NUM_NEARBY_MINES(r, c);
}

void mystery(int r, int c) {
  if (IS_EXPLORED(r, c))
    IS_EMPTY(r, c) ? PRINT_EMPTY() : PRINT_NUM_NEARBY_MINES(r, c);
  else
    HAS_FLAG(r, c) ? PRINT_FLAG() : PRINT_MASK();
}

void genmines() {
  int r, c;

  srand(time(NULL));

  for (r = 0; r < VSIZE; r++)
    for (c = 0; c < HSIZE; c++)
      if (rand() % MINE_CHANCE == 0)
        SET_MINE(r, c);
}

int eachnear(int (*f)(int, int), int r, int c) {
  int counter = 0;

  // left
  if (c != 0)
    counter += f(r, c - 1);
  // up
  if (r != 0)
    counter += f(r - 1, c);
  // right
  if (c != HSIZE - 1)
    counter += f(r, c + 1);
  // down
  if (r != VSIZE - 1)
    counter += f(r + 1, c);
  // top left
  if (r != 0 && c != 0)
    counter += f(r - 1, c - 1);
  // top right
  if (r != 0 && c != HSIZE - 1)
    counter += f(r - 1, c + 1);
  // bottom left
  if (r != VSIZE - 1 && c != 0)
    counter += f(r + 1, c - 1);
  // bottom right
  if (r != VSIZE - 1 && c != HSIZE - 1)
    counter += f(r + 1, c + 1);

  return counter;
}

int hasmine(int r, int c) {
  return HAS_MINE(r, c);
}

int nearbymines(int r, int c) {
  return eachnear(hasmine, r, c);
}

int explore(int r, int c) {
  if (IS_EXPLORED(r, c))
    return 1;

  if (HAS_FLAG(r, c))
    return 2;

  EXPLORE(r, c);

  if (HAS_MINE(r, c))
    return 0;
  else if (IS_EMPTY(r, c))
    eachnear(explore, r, c);

  return 1;
}

int done() {
  int r, c;

  for (r = 0; r < VSIZE; r++)
    for (c = 0; c < HSIZE; c++)
      if (! IS_EXPLORED(r, c) && ! HAS_MINE(r, c))
        return 0;

  return 1;
}

void getrc(int *r, int *c) {
  int ok ;

  do {
    printf("Enter row and column: ");
    scanf("%d %d", r, c);

    --*r;
    --*c;

    ok = 1;
    ok &= (*r >= 0 && *r < VSIZE);
    ok &= (*c >= 0 && *c < HSIZE);
  } while (!ok);
}

void clearscreen() {
    system("cls");
}

void actionfeedback(int status) {
    if (status == BAD_FLAG)
      printf("Can't explore a flagged spot!\n");
}

int gameinprogress(int status) {
    if (done())
        return -1;
    return status != 0;
}

void prompt(int *action, int *r, int *c) {
    printf("Enter %d to explore\n"
           "      %d to flag\n"
           "      %d to delete flag\n",
		   ACTION_EXPLORE, ACTION_FLAG, ACTION_DELFLAG);
    scanf("%d", action);

    getrc(r, c);
}

void handle_action(int action, int r, int c, int *status) {
    if (action == ACTION_EXPLORE)
      *status = explore(r, c);
    else if (action == ACTION_FLAG)
      SET_FLAG(r, c);
    else if (action == ACTION_DELFLAG)
      DEL_FLAG(r, c);
}

int main() {
  int action, r, c, status = 1;

  genmines();

  do {
    clearscreen();
    actionfeedback(status);
    printfield(mystery);
    prompt(&action, &r, &c);
    handle_action(action, r, c, &status);
  } while (gameinprogress(status));

  clearscreen();
  status ? printfield(cleared) : printfield(raw);
  return status ? 0 : 1;
}

