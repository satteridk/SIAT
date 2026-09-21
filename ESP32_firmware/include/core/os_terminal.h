#ifndef OS_TERMINAL_H
#define OS_TERMINAL_H

#include "../os_globals.h"

// --- Estado proprio do console ---
extern int cursorX;
extern int cursorY;
extern unsigned long tempoUltimoBlink;
extern bool cursorVisivel;
extern bool esperandoTexto;
extern bool telaInicialCreditos;

// --- Buffer 2D de historico ---
extern String cacheLinhas[MAX_LINHAS_CACHE];
extern uint16_t cacheCores[MAX_LINHAS_CACHE][MAX_CHARS_LINHA];
extern int idxLinhaCache;
extern int indiceLinhaInicioPagina;
extern int scrollLinha;

// --- Entrada serial nao bloqueante (compartilhada por todos os apps) ---
bool lerLinhaSerial(String& destino);

// --- Cache ---
void limparCache();
void novaLinhaCache();
void adicionarAoCache(char c, uint16_t cor);
void desenharLinhaCache(int indice, int y);

// --- Renderizacao ---
void restaurarPaginaAtual();
void renderizarScroll();
void avancarLinha(uint16_t corRestaurar = ST77XX_GREEN, int tamanhoFonteRestaurar = 1);
void escreverEfeitoDigitacao(const String& texto, int tamanhoFonte, uint16_t cor);
void restaurarTerminal();

// --- Parser ---
String limparAcentos(const String& textoOriginal);
void processarEntradaTerminal();

#endif