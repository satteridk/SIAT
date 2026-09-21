#ifndef APP_EXPLORER_H
#define APP_EXPLORER_H

#include "../os_globals.h"

// Arvore espelhada do MicroSD do PICO (pertence ao explorador)
extern FileNode sistemaArquivos;

void iniciarExplorador();
void sairExplorador();
void desenharExplorador();
void processarEntradaExplorador();

#endif