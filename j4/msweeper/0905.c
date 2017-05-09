#include <stdio.h>        /* for file operations */
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>       /* for file operations */
#include <time.h>         /* for time operations */

typedef struct {
  int data;
  int flag;
  int open;
} BLOCK;

typedef struct {
  char name[20];
  int data[3];
  double time;
} PLAYER;

void init_data(int *col,int *row, int *M);
void init_screen();
void init_map(int col,int row,int M, BLOCK map[col][row]);
void game_play(int col,int row,int M, BLOCK map[col][row]);
void display_map(int col,int row,int M,BLOCK map[col][row]);
void display_help(int row,int mark_cnt,int M);
int auto_open(int x,int y,int col,int row,BLOCK map[col][row]);
int mark(int x,int y,int col,int row,BLOCK map[col][row]);
int check_mark(int col,int row,BLOCK map[col][row]);
void game_over(time_t start_t,int gameover,int col,int row,int M,BLOCK map[col][row]);
int check_gm(int col,int row,BLOCK map[col][row]);
void show_mines(int col,int row,BLOCK map[col][row]);
void high_score(double time,int col,int row,int M);
void view_score();

void main(void)
{

  int col,row,M;
  srand(time(NULL));
  init_data(&col,&row,&M);
 
  BLOCK map[col][row];
  init_map(col,row,M,map);

  game_play(col,row,M,map);
}

void init_data(int *col,int *row, int *M){
  int max_x,max_y;

  init_screen();      /* init screen to use */
  getmaxyx(stdscr,max_y,max_x);  /* get screen size */

  mvprintw(1,3,"%s","Input map column : ");
  scanw("%d",&*col);
  while( *col <= 1  || 3*(*col) > max_x){
    mvprintw(5,3,"%s","Inputted data is bigger than screen size !!!");
    move(1,3); clrtoeol();      /* clear to end of line */
    mvprintw(1,3,"%s","Input map column : ");
    scanw("%d",&*col);
  }

  move(5,3); clrtoeol(); 
  mvprintw(2,3,"%s","Input map row : ");
  scanw("%d",&*row);
  while( *row <= 1 || *row+3 > max_y){
    mvprintw(5,3,"%s","Inputted data is bigger than screen size !!!");
    move(2,3); clrtoeol();
    mvprintw(2,3,"%s","Input map row : ");
    scanw("%d",&*row);
  }
  
  move(5,3); clrtoeol(); 
  mvprintw(3,3,"%s","Input mine numbers : ");
  scanw("%d",&*M);
  while( *M <= 0 || *M > (*col)*(*row)){
    mvprintw(5,3,"%s","Inputted data is bigger than map size !!!");
    move(3,3);clrtoeol();
    mvprintw(3,3,"%s","Input mine numbers : ");
    scanw("%d",&*M);
  }
}

void init_screen(){
  initscr();          /* init screen to use */
  cbreak();           /* wait for keyboard input */
  nonl();
  scrollok(stdscr,TRUE);   /* enable scrolling window */
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE); /* can get special key like backspace,delete . . . */
  start_color();        /* init to use color */
  init_pair(1,COLOR_WHITE,COLOR_BLACK);    /* set colors for display */
  init_pair(2,COLOR_YELLOW,COLOR_BLACK);
  init_pair(3,COLOR_GREEN,COLOR_BLACK);
  init_pair(4,COLOR_RED,COLOR_BLACK);
}

void init_map(int col,int row,int M, BLOCK map[col][row]){
  int i,j,k;
  int x,y;
  
  for(i = 0; i < col; i++){         
    for(j = 0; j < row; j++){
      map[i][j].data = 0;          /* all postitions r empty */
      map[i][j].flag = 0;          /* all postitions r not marked */
      map[i][j].open = 0;          /* all postitions r not opened */
    }
  }
 
  for(k = 0; k < M; k++){         /* generate random mine postition  */
    i = rand()%col;
    j = rand()%row;
    
    if(map[i][j].data == -1){     /* check if that postition has mine or not*/
      k--;
    }
    else {
      map[i][j].data = -1;

      for(x = i-1; x < i+2; x++){    /* add 1 to all postitions around mine */
	for( y = j-1; y < j+2; y++){
	  if(x > -1 && x < col && y > -1 && y < row && map[x][y].data >= 0 ){
	    map[x][y].data++;
	  }
	}
      }
    }
  }
}

void display_map(int col,int row,int M, BLOCK map[col][row]){
  /* display gameboard function */

  int i,j;
     
  for(i = 0; i < col; i++){   /* print x axis */
    attron(WA_BOLD|COLOR_PAIR(2));
    mvprintw(0,3*(i+1),"%d",i);
  }
  
  for( j = 0; j < row; j++){   /* print y axis */
    attron(COLOR_PAIR(2));
    mvprintw(j+1,0,"%d",j);
        
    for(i = 0; i < col; i++){         /* print map contents */
      move((j+1),3*(i+1));
      if(map[i][j].open == 1){
	switch(map[i][j].data){
	case  0 : attron(COLOR_PAIR(1)); addch(' ');;break;
	case -1 : attron(COLOR_PAIR(4)); addch('M');;break;
	default : attron(COLOR_PAIR(1)); addch(map[i][j].data+'0');break;
	}
      }
      else if(map[i][j].flag == 1){
	attron(COLOR_PAIR(3));
	addch('?');
      }
      else{
	attron(COLOR_PAIR(1));
	addch('.');
      }
    }
  }
}

void game_play(int col,int row,int M, BLOCK map[col][row]){
  /* main game function */
  int	x=0,y=0,c;
  int	mark_cnt = 0,gameover = 0;	
  time_t start_t;
  
  clear();
  display_map(col,row,M,map);
  mvprintw(row+2,6,"%s","Press any key to start . . . ");
  move(1,3); refresh();
  time(&start_t);             /* get start time  point */  
  while (gameover == 0){      /* keep continuing until gameover */
    switch(c = getch()){      /* get input key from keyboard */
    case 'o': gameover = auto_open(x,y,col,row,map); break;
    case 'm': {
      mark_cnt += mark(x,y,col,row,map);
      if(mark_cnt == M){      /*  marked nums = mines nums ? */
	gameover = check_mark(col,row,map);
      }
      break;
    }
    case '1': x--; y++; break;
    case '2': y++; break;
    case '3': x++; y++; break;
    case '4': x--; break;
    case '6': x++; break;
    case '7': x--; y--; break;   /* change x,y axises base on input key */
    case '8': y--; break;
    case '9': x++; y--; break;
    case KEY_LEFT:  x--; break;
    case KEY_RIGHT: x++; break;
    case KEY_UP:    y--; break;
    case KEY_DOWN:  y++; break;
    }
    
    if (x < 0)  x = col - 1;    /*keep x,y value in map size */
    else if (x >= col)  x = 0;
    if (y < 0)  y = row - 1;
    else if (y >= row)  y = 0;
    
    display_map(col,row,M,map);
    display_help(row,mark_cnt,M);
    move(y+1,3*(x+1));      /* move curses */
    refresh();
  }
  game_over(start_t,gameover,col,row,M,map); 
  endwin();   /* close curses windows */
}

void display_help(int row,int mark_cnt,int M){
  /* display mine left number and input guide */

  attron(COLOR_PAIR(1));
  mvprintw(row+1,2,"%s"," Mines left : ");
  printw("%d\n",M-mark_cnt);//clrtoeol;
  mvprintw(row+2,2,"%s"," Use number key or 4 arrow keys to move, 'o' key to open and 'm' key to mark ");
}

int mark(int x,int y,int col,int row,BLOCK map[col][row]){
  /* mark postition when 'm' is pressed */
  
  map[x][y].flag = 1 - map[x][y].flag;
  
  if(map[x][y].flag == 1) return 1;    /* return val for mark count */
  else return -1;
}

int auto_open(int x,int y,int col,int row,BLOCK map[col][row]){
  /* auto-open when 'o' is pressed */
  /* return val for gameover */

  int temp;
  /* keep x,y value in map size */
  if(x < 0 || x >= col || y < 0 || y >= row || map[x][y].open == 1) return 0;
  
  if(map[x][y].data == -1 )   /* map[x][y] is mine */
    return 1;
  else{
    if(map[x][y].flag == 0){  /* open if map[x][y] not flagged */
      map[x][y].open = 1;
    }
    if(map[x][y].data == 0 ){    
      auto_open(x-1,y-1,col,row,map); 
      auto_open(x,y-1,col,row,map);
      auto_open(x+1,y-1,col,row,map);
      auto_open(x-1,y,col,row,map);    /* auto check map[x][y] around pos */
      auto_open(x+1,y,col,row,map);
      auto_open(x-1,y+1,col,row,map);
      auto_open(x,y+1,col,row,map);
      auto_open(x+1,y+1,col,row,map);
    }
  }
  temp = check_gm(col,row,map);        /* check if all save post are not open */
  if(temp == 1) return -1;
  else return 0;
}


int check_mark(int col,int row,BLOCK map[col][row]){
  /* auto-check when marked number equal mines number */
  /* return val for gameover */
  
  int i,j;
  for(i = 0; i < col; i++){        
    for(j = 0; j < row; j++){
      if(map[i][j].flag == 1 && map[i][j].data != -1) return 1;
    }
  }
  return -1;     
}

int check_gm(int col,int row,BLOCK map[col][row]){
  /* check if all mines pos are not opend is TRUE or NOT */
  /* return val to keep program continue or not */
  
  int i,j;
  for(i = 0; i < col; i++){        
    for(j = 0; j < row; j++){
      if(map[i][j].open == 0 && map[i][j].data != -1)
	return 0;    /* game is  continuing */
    }
  }
  return 1;    /* player win  */
}

void show_mines(int col,int row,BLOCK map[col][row]){
  /* open all mines postitions in map */ 
  int i,j;
  for(i = 0; i < col; i++){        
    for(j = 0; j < row; j++){
      if(map[i][j].data == -1){
	map[i][j].open = 1;
      }
    }
  }
}

void game_over(time_t start_t,int gameover,int col,int row,int M,BLOCK map[col][row]){
  /* display played time when gameover */
  /* save player info if get new highscore */
  /* display highscore */
  
  time_t end_t;
  char temp[5];
  int highscore = 0;

  time(&end_t);             /* get finish time point */
  show_mines(col,row,map);  /* display ending map */
  display_map(col,row,M,map);
  
  move(row+1,2);clrtoeol;
  switch(gameover){
  case 1: addstr(" *** YOU LOSE *** ");break;
  case -1: addstr(" *** YOU WIN *** ");highscore = 1;break;
  }
  mvprintw(row+2,2,"%s"," Played time : ");
  printw("%.0f",difftime(end_t,start_t));
  addstr(" seconds.\n");

  mvprintw(row+3,2,"%s"," Press any key to continue ....."); 
  getch();
  
  if(highscore == 1){
    high_score(difftime(end_t,start_t),col,row,M);     /* save player's highscore */
  }
}

void high_score(double time,int col,int row,int M){
  /* save player's highscore to file */
  /* read highscore from file to display */
  FILE *db;
  PLAYER player;
  int write_db = 0;
  
  if( access("data.db",F_OK) == -1 )  write_db = 1;  /* check if file existed or not */
  else{
    db = fopen("data.db","rb");
    fread(&player, sizeof(player),1,db);   /* read data from file */
    if(time/M <= player.time/player.data[2]){
      write_db = 1;
    }
  }
  
  if(write_db == 1){
    clear();
    mvprintw(1,3,"%s"," Input your name : "); /* get player name */
    getstr(player.name);
    player.data[0] = col;
    player.data[1] = row;
    player.data[2] = M;
    player.time = time;

    db = fopen("data.db","wb");     /* write data to file */
    fwrite(&player,sizeof(player),1,db);
    fclose(db);
    view_score();
  }
  else view_score();
}

void view_score(){
  /* read highscore data from file & display in screen */

  FILE *db;
  PLAYER player = {0};
  int max_x;;
  
  if(access("data.db",F_OK) != -1){  /* file not exits */
    db = fopen("data.db","rb");
    fread(&player, sizeof(player), 1,db);
    fclose(db);
  }
  
  clear();                            /* clear screen */
  max_x = getmaxx(stdscr);
  mvprintw(2,max_x/2-16,"%s","######### HIGH SCORE ##########");
  mvprintw(3,max_x/2-16,"%s","Player name : ");     /* display player name */
  addstr(player.name);
  mvprintw(4,max_x/2-16,"%s","Map size : ");        /* display map size */
  printw("%d",player.data[0]);
  addstr(" x ");
  printw("%d",player.data[1]);
  mvprintw(5,max_x/2-16,"%s","Mine numbers : ");    /* display mine nums */
  printw("%d",player.data[2]);
  mvprintw(6,max_x/2-16,"%s","Played time : ");     /* display played time */
  printw("%.0f",player.time);
  addstr(" second.");
  mvprintw(7,max_x/2-16,"%s","###############################");
  mvprintw(9,0,"%s"," Press any key to exit . . . ");
  getch();
}

