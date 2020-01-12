
#include "editor.h"


int main(int argc, char *argv[])
 {
  clear();
  enableRawMode();
  initEditor();
  if (argc >= 2)
  {
    ouvrir_editeur(argv[1]);
  }

  status_editeur("HELP: i = inserer | Esc = mode normale | :w = save | :q = quit ");

  while (1) 
  {
    refrechir_lapage();
    editeur_Process_Keypress();
  }

  return 0;
}
