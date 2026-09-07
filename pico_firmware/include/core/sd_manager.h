#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "../pico_globals.h"

// O Default Argument (= 0) fica apenas aqui no header
void construirCache(File dir, int nivel = 0);
void inicializarSD();
void verificarStatusSD();

#endif