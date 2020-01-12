#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 1


enum editorKey {
  //retour
  BACKSPACE = 127,
  //fléche gauche
  ARROW_LEFT = 1000,
  //fléche right
  ARROW_RIGHT,
  //fléche vers le haut
  ARROW_UP,
  //fléche vers le bas 
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  //le bas de page 
  PAGE_DOWN
};

//la structeur 
typedef struct erow {
  int size;
  int rsize;
  char *chars;
  char *render;
} erow;


struct editorConfig {
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  //nombre de rows
  int screenrows;
   //nombre de rcolonne 
  int screencols;
  int numrows;
  erow *row;
  int dirty;
  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
  struct termios orig_termios;
};


struct editorConfig E;

void clear();
void status_editeur(const char *fmt, ...);
void refrechir_lapage();
char *editorPrompt(char *prompt);
void effacer_fenetre(const char *s);
void disableRawMode() ;
void enableRawMode();
int editorReadKey();
int position_cursur(int *rows, int *cols);
int taille_fenetre(int *rows, int *cols) ;
int editorRowCxToRx(erow *row, int cx);
void refrechir_ligne(erow *row) ;
void inserer_ligne(int at, char *s, size_t len);
void editorFreeRow(erow *row) ;
void supprimer_ligne(int at);
void inserer_char_line(erow *row, int at, int c);
void editorRowAppendString(erow *row, char *s, size_t len);
void supprimer_char_ligne(erow *row, int at);
