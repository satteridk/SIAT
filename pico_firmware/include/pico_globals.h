#ifndef PICO_GLOBALS_H
#define PICO_GLOBALS_H

#include "pico_config.h"

extern bool cartao_conectado;
extern bool mensagem_aguardando_exibida;
extern unsigned long tempo_anterior_serial;

// Cache substituído por buffer estático seguro de 8KB
extern char cacheSD[8192];
extern int cacheIndex;

#endif