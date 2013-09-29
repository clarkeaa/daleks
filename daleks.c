#include <curses.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ROWS 19
#define NUM_COLS 29

/* special types */
enum Type {EMPTY, DOC, DALEK, PILE};
typedef struct Objecttag {
  int row;
  int col;
  enum Type type;
  struct Objecttag* next;
} Object;

/* prototypes */
void position(Object*);
void print(Object*, int, int);
bool validateMove(Object*, int, int);
bool moveObject(Object*, int);
void moveDaleks(Object*);
int distance(int,int,int,int);
char bestDalekMove(int,int,Object*);
bool checkCollisions(Object*);
void sonicScrew(Object *);
bool allDead(Object *);
Object* resetObjects(Object *);
void titleScreen();

int main()
{
  int i;
  Object* objects;
  Object* temp;
  bool playing = TRUE;
  int screwCount = 1;
  int level = 1;
  
  /*initialize original objects*/
  objects = malloc(sizeof(Object));
  objects->type = DOC;
  temp = objects;
  for(i = 0; i< 6; i++)
  {
    temp->next = malloc(sizeof(Object));
    temp = temp->next;
    temp->type = DALEK;
  }
  position(objects);

  /*initialize screen*/
  initscr();
  noecho();
  titleScreen();
  print(objects, level, screwCount);
  
  while (playing)
  {
    /*handle input*/
    char c = getch();
    if (c == 'q')
      break;
    else if (c == 's' && screwCount > 0) { 
      sonicScrew(objects);
      screwCount--;
    }
    else {
      /*move, if move isn't valid, handle next input*/
      if ( !moveObject(objects, c)/*move doc*/ )
        continue;
    }
    
    moveDaleks(objects);
    
    /*check results*/
    playing = checkCollisions(objects);
    if (playing && allDead(objects) ) {
      /*level up*/
      level++;
      screwCount++;
      
      temp = resetObjects( objects);/*temp = ptr to last dalek*/
      
      /* add 4 more daleks */
      for (i = 0; i< 4; i++) {
        temp->next = malloc(sizeof(Object));
        temp = temp->next;
        temp->type = DALEK;
      }
      
      position(objects);
    }
    
    print(objects, level, screwCount);
  }
  
  mvprintw(LINES-1, 0, "Game Over -press a key-");
  getch();
  endwin();
  return 0;
}

/*PURPOSE:arrange the doc and all the daleks randomly
//IN: linked list of objects
//OUT: objects have row and col cordinates changed */
void position(Object* pointer)
{
  Object* temp;
  int i,j;
  /*grid to keep track of open spaces*/
  bool grid[NUM_ROWS][NUM_COLS];
  for (i = 0; i< NUM_ROWS; i++)
    for(j=0; j <NUM_COLS; j++)
      grid[i][j] = FALSE;
  
  srand(time(0));
  
  /*step up the doc*/
  pointer->row = NUM_ROWS/2;
  pointer->col = NUM_COLS/2;
  grid[pointer->row][pointer->col] = TRUE;
  temp = pointer->next;

  /*set up the daleks*/
  while(temp != 0 )
  {
    int randRow = rand() % NUM_ROWS;
    int randCol = rand() % NUM_COLS;
    if (grid[randRow][randCol] == FALSE) {
      temp->row = randRow;
      temp->col = randCol;
      temp = temp->next;
      grid[randRow][randCol] = TRUE;
    }
  }
}

/*PURPOSE: print out the field
//IN:linked list of objects, level, score
//OUT: outputed to screen */
void print(Object* things, int lvl, int sc)
{
  move(0,0);
  int i,j;
  char grid[NUM_ROWS][NUM_COLS];
  for(i=0; i<NUM_ROWS; i++)
    for(j=0; j<NUM_COLS;j++)
      grid[i][j] = '.';

  Object* temp = things;
  while (temp != 0)
  {
    if ((*temp).type == DALEK)
      grid[temp->row][temp->col] = 'F';
    else if ((*temp).type == PILE)
      grid[temp->row][temp->col] = '#';
    else if (temp->type == DOC)
      grid[temp->row][temp->col] = '@';
    else
      grid[temp->row][temp->col] = '.';

    temp = temp->next;
  }

  for(i = 0; i<NUM_ROWS; i++) {
    for(j =0; j< NUM_COLS; j++) {
      printw("%c",grid[i][j]);
    }
    printw("\n");
  }
  
  mvprintw(0,NUM_COLS + 4,"Daleks");
  mvprintw(1,NUM_COLS + 4,"Adaption by A Clarke");
  mvprintw(3,NUM_COLS + 4,"Level: %d", lvl);
  mvprintw(4,NUM_COLS + 4,"SonicScrews: %d", sc);
  mvprintw(6,NUM_COLS + 4,"Controls:");
  mvprintw(7,NUM_COLS + 4,"t = teleport");
  mvprintw(8,NUM_COLS + 4,"s = sonic screw");
  mvprintw(9,NUM_COLS + 4,"q = quit");
  mvprintw(11,NUM_COLS + 4, "7   8   9\n");
  mvprintw(12,NUM_COLS + 4, "  \\ | /\n");
  mvprintw(13,NUM_COLS + 4, "u - i - o\n");
  mvprintw(14,NUM_COLS + 4, "  / | \\\n");
  mvprintw(15,NUM_COLS + 4, "j   k   l");
  
  refresh();
}

bool validateMove(Object* obj, int dRow, int dCol)
{
   int tempRow = obj->row + dRow;
   int tempCol = obj->col + dCol;

   if ( tempRow >= NUM_ROWS ||
        tempRow < 0         ||
        tempCol >= NUM_COLS ||
        tempCol < 0         ) 
   {
      return FALSE;
   } else {
      obj->row = tempRow;
      obj->col = tempCol;
      return TRUE;
   }
}

/*PURPOSE: move a object in the field
//IN:object to move, which move to do
//OUT: objects row col changed, bool if move was successful*/
bool moveObject(Object* obj, int move)
{
   switch( move ) {
   case '7':
      return validateMove(obj, -1, -1);
      break;
   case '8':
      return validateMove(obj, -1, 0);
      break;
   case '9':
      return validateMove(obj, -1, 1);
      break;
   case 'u':
      return validateMove(obj, 0, -1);
      break;
   case 'i':
      return TRUE;
      break;
   case 'o':
      return validateMove(obj, 0, 1);
      break;
   case 'j':
      return validateMove(obj, 1, -1);
      break;
   case 'k':
      return validateMove(obj, 1, 0);
      break;
   case 'l':
      return validateMove(obj, 1, 1);
      break;
   case 't':
      srand(time(0));
      obj->row = rand() % NUM_ROWS;
      obj->col = rand() % NUM_COLS;
      return TRUE;
      break;
   default:
      return FALSE;
   }

   return FALSE;
}

/*PURPOSE: move all the daleks
//IN: pointer to linked list of objects
//OUT: all their cordinates are changed*/
void moveDaleks(Object* pointer)
{
  int docRow = pointer->row;
  int docCol = pointer->col;
  Object* temp = pointer->next;

  while(temp != 0) {
    if (temp->type == DALEK)
      moveObject(temp, bestDalekMove(docRow,docCol, temp));
    temp = temp->next;
  }
}

/*PURPOSE:find distance between 2 points squared
//IN:x and y of point1 and point2
//OUT: distance squared*/
int distance(int x1,int y1,int x2,int y2)
{
  return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
}

/*PURPOSE: determine best move for dalek
//IN:row and col of doc, point to dalek
//OUT: char representing best move*/
char bestDalekMove(int docr, int docc, Object* dalek)
{
  int dalr = dalek->row;
  int dalc = dalek->col;
  char bestMove = '7';
  int shortest = distance(docr,docc,dalr-1,dalc-1);

  if (distance(docr,docc,dalr-1,dalc) < shortest) {
    bestMove = '8';
    shortest = distance(docr,docc,dalr-1,dalc);
  }
  if (distance(docr,docc,dalr-1,dalc+1)<shortest) {
    bestMove = '9';
    shortest = distance(docr,docc,dalr-1,dalc+1);
  }
  if (distance(docr,docc,dalr,dalc-1) < shortest) {
    bestMove = 'u';
    shortest = distance(docr,docc,dalr,dalc-1);
  }
  if (distance(docr,docc,dalr,dalc+1) < shortest) {
    bestMove = 'o';
    shortest = distance(docr,docc,dalr,dalc+1);
  }
  if (distance(docr,docc,dalr+1,dalc-1) <shortest) {
    bestMove = 'j';
    shortest = distance(docr,docc,dalr+1,dalc-1);
  }
  if (distance(docr,docc,dalr+1,dalc) < shortest) {
    bestMove = 'k';
    shortest = distance(docr,docc,dalr+1,dalc);
  }
  if (distance(docr,docc,dalr+1,dalc+1) <shortest) {
    bestMove = 'l';
    shortest = distance(docr,docc,dalr+1,dalc+1);
  }
  
  return bestMove;
}

/*PURPOSE:check for collisions
//IN:liked list of objects
//OUT: false if doc crashed, true otherwise*/
bool checkCollisions(Object* docptr)
{
  int i, j;
  bool keepPlaying = TRUE;
  
  Object* grid[NUM_ROWS][NUM_COLS];
  for(i = 0; i< NUM_ROWS; i++)
    for(j=0; j< NUM_COLS; j++)
      grid[i][j] = 0;

  Object* temp = docptr;
  while (temp != 0) {
    if ( grid[temp->row][temp->col] == 0 ) {
      grid[temp->row][temp->col] = temp;
    }
    else if (grid[temp->row][temp->col] == docptr ) {
      keepPlaying = FALSE;
      docptr->type = PILE;
      temp->type = PILE;
    }
    else {
      grid[temp->row][temp->col]->type = PILE;
      temp->type = PILE;
    }
    temp = temp->next;
  }

  return keepPlaying;
}

/*PURPOSE: kill daleks around the doc
//IN:linked list of objects
//OUT: linked list changed to zap daleks*/
void sonicScrew( Object* pointer )
{
  int docr = pointer->row;
  int docc = pointer->col;
  Object* last = pointer;
  Object* temp = pointer->next;
    
  while(temp != 0 ) {
    if (distance(docr,docc, temp->row, temp->col) <= 2)
    {
      last->next = temp->next;
      temp->type = EMPTY;
      free(temp);
      temp = last->next;
    }
    else 
    {
      last = temp;
      temp = temp->next;
    }
  } 
}

/*PURPOSE: check if all daleks are dead
//IN:point to link list of objects
//OUT: true if all daleks are dead*/
bool allDead( Object* pointer )
{
  Object* temp = pointer;
  
  while(temp != 0) {
    if (temp->type == DALEK)
      return FALSE;
    temp = temp->next;
  }
  return TRUE;
}

/*PURPOSE: after level up reset the objects
//IN:linked list to objects
//OUT: objects with their types reset*/
Object* resetObjects( Object* pointer )
{
  pointer->type = DOC;
  Object* temp = pointer->next;
  
  while( temp->next != 0 ) {
    temp->type = DALEK;
    temp = temp->next;
  }
  temp->type = DALEK;
  return temp;
}

void titleScreen()
{
  printw("********************************\n");
  printw("* Daleks! Adaption by A Clarke *\n");
  printw("********************************\n\n");
  printw("This is a game based of the Dr. Who");
  printw(" television show where the point is to\n");
  printw("help Dr Who (denoted by \"@\") escape ");
  printw("these horrible monster robot things called\n");
  printw("Daleks (denoted by \"F\"), to kill them you");
  printw(" must make them crash into eachother\n");
  printw("or at your disposal you also have the power of");
  printw(" SONIC SCREW, whatever that is \n");
  printw("(ones awarded each stage).  It will make any");
  printw(" daleks around you disappear.  \n");
  printw("Dr. Who also knows how to teleport to a");
  printw(" random location, but beware, you might \n");
  printw("teleport right into a dalek.\n");
  printw("Well that's it, have fun.\n\n");
  printw("Controls:\n");
  printw("t = teleport; s = super screw; q = quit\n");
  printw("Movement:\n");
  printw( "7   8   9\n");
  printw( "  \\ | /\n");
  printw( "u - i - o\n");
  printw( "  / | \\\n");
  printw( "j   k   l");
  mvprintw(LINES-1, 0, "Press a key to start.");
  getch();
  clear();
  
}
