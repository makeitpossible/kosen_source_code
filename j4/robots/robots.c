#include <stdlib.h>
#include <curses.h>
#include <time.h>
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define L 60
#define H 20

extern char getChar(void);

typedef struct data{
  int x;
  int y;
  int crash;
  struct data *next;
} DATA;

void init_screen(){
  /* init screen before use */
  initscr();
  cbreak();   /* wait for keyboard input */
  nonl();
  scrollok(stdscr,TRUE);   /* enable scrolling window */
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
}

void init_data(int lv, DATA *data){
  /* init player and robots info */

  int i = 0;
  DATA *p,*new;

  data->x = random()%L + 1;  /* random player postition */
  data->y = random()%H + 1;
  data->crash = 0;
  p = data;

  /* random 5*lv robots post */
  for ( i = 0; i < MIN(5*lv,40); i++){
    new = (DATA *)malloc(sizeof(DATA));
    new->x = random()%L + 1;
    new->y = random()%H + 1;
    new->crash = 0;
    if(new->x == data->x && new->y == data->y) i--;
    else{
      p -> next = new;
      p = new;
    }
  }
  p -> next = NULL;
}

void dis_field(int lv,DATA *data){
  /* display game field */
  int i,j;

  for( i = 1; i < L+1; i++){  /* display field border */
    mvprintw(0,i,"%c",'-');
    mvprintw(H+1,i,"%c",'-');
  }
  for( j = 1; j < H+1; j++){
    mvprintw(j,0,"%c",'|');
    mvprintw(j,L+1,"%c",'|');
  }
  mvprintw(0,0,"%c",'+');
  mvprintw(0,L+1,"%c",'+');
  mvprintw(H+1,0,"%c",'+');
  mvprintw(H+1,L+1,"%c",'+');

  /* display player and robots */
  DATA *p;
  p = data;

  mvprintw(p->y,p->x,"%c",'@');
  for(p = p->next; p != NULL ; p = p->next){
    if( p->crash == 1){
      mvprintw(p->y,p->x,"%c",'*');
    }
    else mvprintw(p->y,p->x,"%c",'+');
  }
}

int robots_crash(DATA *data){
  /* return number of new robot crashed */
  DATA *p,*del_p;
  int cnt = 0;
  
  for(del_p = data->next; del_p->next != NULL; del_p=del_p->next){
    for(p = del_p->next;p != NULL;p=p->next){
      if(del_p->x == p->x && del_p->y == p->y){
	if(p->crash != 1){
	  p->crash = 1; cnt++;
	}
	if(del_p->crash != 1){
	  del_p->crash = 1; cnt++;
	}
      }
    }
  }
  return cnt;
}

void mv_robots(int x, int y, DATA *data){
  /*  move robots post  */
  DATA *p;
  
  for( p = data->next; p != NULL;p = p->next){
    if(p->crash == 0){
      mvprintw(p->y,p->x,"%c",' ');
      if(data->x > p->x) p->x ++;
      else if(data->x < p->x) p->x --;
      if(data->y > p->y) p->y ++;
      else if(data->y < p->y) p->y --;
    }
  }
}

int mv_player(int x, int y, DATA *data){
  
  DATA *p = data->next;
  int temp_x,temp_y,tlp = 0;
  
  /* player teleport */
  if(x == 5 && y == 5){
    temp_x = random()%L + 1;
    temp_y = random()%H + 1;
    tlp = 1;
  }
  else { /* normal move */
    temp_x = data->x +x;
    temp_y = data->y +y;
  }
  
  /* check new post condition
     in game field,
     has robot (crashed robot) or not */
  if(temp_x > 0 && temp_x < L+1
     && temp_y > 0 && temp_y < H+1){
    while( p != NULL){
      if(temp_x == p->x && temp_y == p->y){
	if(tlp == 1){
	  temp_x = random()%L + 1;
	  temp_y = random()%H + 1;
	  p = data;
	  p = p->next;
	}
	else return 0;
      }
      else p = p->next;
    }
    mvprintw(data->y,data->x,"%c",' ');
    data->x = temp_x;
    data->y = temp_y;
    return 1;
  }
}

int check_gm(DATA *data){
  /* return 1 when gameover,
     return 0 to continue,
     return -1 to level up */
  DATA *p;
  int temp = 0;
  
  for(p = data->next; p != NULL; p=p->next){
    if(data->x == p->x && data->y == p->y){
      return 1;
    }
    if(p->crash == 0){
      temp = 1;
    }
  }
  if( temp == 1) return 0;
  else return -1;
}

int main(void){
  
  int x,y,lv = 1,score = 0;
  DATA data;
  int tmp = 0;

  /* init for 1st time run */
  srandom(time(NULL));
  init_screen();
  init_data(lv,&data);

  /* loop until gameover */
  while(check_gm(&data) == 0){
    x = 0;y = 0;
    dis_field(lv,&data);
    /* display game level and score */
    mvprintw(H+2,3," Level:%d Score:%d   ?",lv,score);
    move(H+2,22);
    switch(getch()){
    case '1': x--; y++; break;
    case '2': y++; break;
    case '3': x++; y++; break;
    case '4': x--; break;
    case '5': break;
    case '6': x++; break;
    case '7': x--; y--; break;
    case '8': y--;break;
    case '9': x++; y--;break;
    case '0': x = 5; y = 5; break;
    default : x = -5;break;
    }
    if(x > -2){ /* not get other input */
      if(mv_player(x,y,&data) == 1){
	mv_robots(x,y,&data);
	score += robots_crash(&data);
      }
      if(check_gm(&data) == -1){ /* lever up */
	score += lv*10;
	clear();
	lv++;
	init_data(lv,&data);
      }
      refresh();
    }
  }
  dis_field(lv,&data);
  mvprintw(H+3,3,"%s","Game over !!!");
  getch();
  endwin();
  return 0;
}
