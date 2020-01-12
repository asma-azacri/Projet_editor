#include "editor.h"

#define BUFFER_SIZE 1024
#define ABUF_INIT {NULL, 0}




// inserer un caractere dans editeur
void inserer_caractere (int c)
{
  if (E.cy == E.numrows)
  {
    inserer_ligne(E.numrows, "", 0);
  }
  inserer_char_line(&E.row[E.cy], E.cx, c);
  E.cx++;
}

//inserer un nouveau ligne dans editeur
void inserer_nvligne() 
{
  if (E.cx == 0) {
    inserer_ligne(E.cy, "", 0);
  } else {
    erow *row = &E.row[E.cy];
    inserer_ligne(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
    row = &E.row[E.cy];
    row->size = E.cx;
    row->chars[row->size] = '\0';
    refrechir_ligne(row);
  }
  E.cy++;
  E.cx = 0;
}

// supprimer un caractere dans editeur
void supprimer_char() 
{
  if (E.cy == E.numrows) return;
  if (E.cx == 0 && E.cy == 0) return;

  erow *row = &E.row[E.cy];
  if (E.cx > 0) {
    supprimer_char_ligne(row, E.cx - 1);
    E.cx--;
  } else {
    E.cx = E.row[E.cy - 1].size;
    editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
    supprimer_ligne(E.cy);
    E.cy--;
  }
}

//mettre le contenu d'une ligne dans un buffer 
//verifier par kawtar
char *editeuerligneToString(int *buflen)
 {
  int totlen = 0;
  int j;
  for (j = 0; j < E.numrows; j++)
    totlen += E.row[j].size + 1;
  *buflen = totlen;

  char *buf = malloc(totlen);
  char *p = buf;
  for (j = 0; j < E.numrows; j++) {
    memcpy(p, E.row[j].chars, E.row[j].size);
    p += E.row[j].size;
    *p = '\n';
    p++;
  }

  return buf;
}

// ouvrir un fichier
void ouvrir_editeur(char *filename) 
{
  free(E.filename);
  E.filename = strdup(filename);
   char buffer[BUFFER_SIZE];
    int totalRead = 0;
  
    FILE *fp = fopen(filename, "r");
    if (!fp) effacer_fenetre("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
      while (linelen > 0 && (line[linelen - 1] == '\n' ||
                           line[linelen - 1] == '\r'))
        linelen--;
      inserer_ligne(E.numrows, line, linelen);
    }
    free(line);
    fclose(fp);
   E.dirty = 0;
}

// enregistrer  le fichier
void enregistrer() 
{
  if (E.filename == NULL) {
    E.filename = editeur_prompt("Enregistrer sous: %s (ESC to cancel)");
    if (E.filename == NULL) {
      status_editeur("Enregistrer annulé");
      return;
    }
  }

  int len;
  char *buf = editeuerligneToString(&len);

  int fd = open(E.filename, O_RDWR | O_CREAT, 0644);

  if (fd != -1) {
   // tronquer un fichier à un fichier spécifié (ftruncate)
    if (ftruncate(fd, len) != -1) {
      if (write(fd, buf, len) == len) {
        close(fd);
        free(buf);
        E.dirty = 0;
        status_editeur("%d octets écrits sur le disque", len);
        return;
      }
    }
    close(fd);
  }

  free(buf);
  status_editeur("ne peux pas enregistrer! Erreur I / O: %s", strerror(errno));
}


/*** append buffer ***/

void abAppend(struct abuf *ab, const char *s, int len) 
{
  char *new = realloc(ab->b, ab->len + len);

  if (new == NULL) return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(struct abuf *ab) 
{
  free(ab->b);
}

/*** output ***/
//faire défiler
//éditeur Scroll
void editeur_Scroll()
 {
  E.rx = 0;
  if (E.cy < E.numrows) {
    E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
  }

  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }
  if (E.cy >= E.rowoff + E.screenrows) {
    E.rowoff = E.cy - E.screenrows + 1;
  }
  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }
  if (E.rx >= E.coloff + E.screencols) {
    E.coloff = E.rx - E.screencols + 1;
  }
}

void editeur_Dessine_ligne(struct abuf *ab) 
{
  int y;
  for (y = 0; y < E.screenrows; y++) {
    int filerow = y + E.rowoff;
    if (filerow >= E.numrows) {
      if (E.numrows == 0 && y == E.screenrows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
          " editor");
        if (welcomelen > E.screencols) welcomelen = E.screencols;
        int padding = (E.screencols - welcomelen) / 2;
        while (padding--) abAppend(ab, " ", 1);
        abAppend(ab, welcome, welcomelen);
      } 
    } else {
      int len = E.row[filerow].rsize - E.coloff;
      if (len < 0) len = 0;
      if (len > E.screencols) len = E.screencols;
      abAppend(ab, &E.row[filerow].render[E.coloff], len);
    }

    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }
}

// editeur la bare en bas contient le nom de fichier ou enregestrer nouveau fichier

void editeur_dessine_StatusBar(struct abuf *ab)
 {
  abAppend(ab, "\x1b[7m", 4);
  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
    E.filename ? E.filename : "[No Name]", E.numrows,
    E.dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
    E.cy + 1, E.numrows);
  if (len > E.screencols) len = E.screencols;
  abAppend(ab, status, len);
  while (len < E.screencols) {
    if (E.screencols - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    } else {
      abAppend(ab, " ", 1);
      len++;
    }
  }
  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}

void editeur_dessine_MessageBar(struct abuf *ab)
 {
  abAppend(ab, "\x1b[K", 3);
  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols) msglen = E.screencols;
  if (msglen && time(NULL) - E.statusmsg_time < 5)
    abAppend(ab, E.statusmsg, msglen);
}


void refrechir_lapage() 
{
  editeur_Scroll();

  struct abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);

  editeur_Dessine_ligne(&ab);
  editeur_dessine_StatusBar(&ab);
  editeur_dessine_MessageBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,
                                            (E.rx - E.coloff) + 1);
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

void status_editeur(const char *fmt, ...) 
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);
  E.statusmsg_time = time(NULL);
}

/*** input ***/

char *editeur_prompt(char *prompt) 
{
  size_t bufsize = 128;
  char *buf = malloc(bufsize);

  size_t buflen = 0;
  buf[0] = '\0';

  while (1) {
    status_editeur(prompt, buf);
    refrechir_lapage();

    int c = editorReadKey();
    if (c == DEL_KEY | c == BACKSPACE) {
      if (buflen != 0) buf[--buflen] = '\0';
    } else if (c == '\x1b') {
      status_editeur("");
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (buflen != 0) {
        status_editeur("");
        return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
  }
}


//bouger le curseur (deplacement de fléche )
void bouger_curseur(int key)
{
  erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0) {
        E.cx--;
      } else if (E.cy > 0) {
        E.cy--;
        E.cx = E.row[E.cy].size;
      }
      break;
    case ARROW_RIGHT:
      if (row && E.cx < row->size) {
        E.cx++;
      } else if (row && E.cx == row->size) {
        E.cy++;
        E.cx = 0;
      }
      break;
    case ARROW_UP:
      if (E.cy != 0) {
        E.cy--;
      }
      break;
    case ARROW_DOWN:
      if (E.cy < E.numrows) {
        E.cy++;
      }
      break;
  }

  row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  int rowlen = row ? row->size : 0;
  if (E.cx > rowlen) {
    E.cx = rowlen;
  }
}


//éditeur appuyez sur la touche de processus

//editor commande 
void editeur_Process_Keypress()
 {
  static int quit_times = KILO_QUIT_TIMES;
  static char mode = 'E';
  int d=editorReadKey();
  if( d == ':'){
    int c = editorReadKey();
    switch (c) {
      case   'q' :
        if (E.dirty && quit_times > 0){
          status_editeur("ATTENTION!!! Le fichier contient des modifications non enregistrées Appuyez sur :w . "" Appuyez sur : q  pour quitter.", quit_times);
           quit_times--;
          return;
         }
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
      break;

      case 'w':
        enregistrer();
      break;
    }
  }

  switch (d) {
    case 'i':
      if(mode == 'E'){
        mode ='I';
      }else{
        inserer_caractere (d);
      }
    
    break;
    case HOME_KEY:
      E.cx = 0;
    break;

    case END_KEY:
      if (E.cy < E.numrows)
        E.cx = E.row[E.cy].size;
      break;

    case BACKSPACE:
    case DEL_KEY:
      if(mode == 'I'){
       if (d == DEL_KEY) bouger_curseur(ARROW_RIGHT);
        supprimer_char();
      }
    break;
    case PAGE_UP:
    case PAGE_DOWN:
      {
        if (d == PAGE_UP) {
          E.cy = E.rowoff;
        } else if (d == PAGE_DOWN) {
          E.cy = E.rowoff + E.screenrows - 1;
          if (E.cy > E.numrows) E.cy = E.numrows;
        }

        int times = E.screenrows;
        while (times--)
          bouger_curseur(d == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
    break;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
      bouger_curseur(d);
    break;

    case '\x1b':
       mode='E';
    break;
    default:
      if(mode == 'I')
      inserer_caractere (d);
      break;
  }
  
  quit_times = KILO_QUIT_TIMES;
}

/*** initialiser l'editeur  ***/

void initEditor() 
{
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.numrows = 0;
  E.row = NULL;
  E.dirty = 0;
  E.filename = NULL;
  E.statusmsg[0] = '\0';
  E.statusmsg_time = 0;

  if (taille_fenetre(&E.screenrows, &E.screencols) == -1) effacer_fenetre("taille_fenetre");
  E.screenrows -= 2;
}
