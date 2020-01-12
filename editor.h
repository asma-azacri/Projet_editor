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
#include "terminal.h"


//le buffer 
struct abuf {
  char *b;
  int len;
};

void inserer_caractere (int c);
void inserer_nvligne() ;
void supprimer_char();
char *editeuerligneToString(int *buflen);
void ouvrir_editeur(char *filename);
void enregistrer();
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab) ;
void editeur_Scroll();
void editeur_Dessine_ligne(struct abuf *ab);
void editeur_dessine_StatusBar(struct abuf *ab);
void editeur_dessine_MessageBar(struct abuf *ab) ;
void refrechir_lapage();
void status_editeur(const char *fmt, ...);
char *editeur_prompt(char *prompt);
void bouger_curseur(int key);
void editeur_Process_Keypress();
void initEditor();
