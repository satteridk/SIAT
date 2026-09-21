#ifndef OS_GUI_H
#define OS_GUI_H

#include "../os_globals.h"

// --- Estado proprio da camada de apresentacao ---
extern String tituloAtual;
extern std::vector<MenuItem> menuAtual;
extern int opcaoSelecionada;
extern int marqueeOffset;
extern unsigned long lastMarqueeUpdate;

// --- Painel lateral ---
void atualizarListaMenu();
void desenharMenu();
void atualizarMarquee();
void executarItemMenu(MenuItem& item);

// --- Moldura e sobreposicoes ---
void desenharCabecalho();
void desenharPopup(int segundos);
void fecharPopup();

// --- Telas de boot (os_boot.cpp) ---
void animacaoDeBoot();
void exibirBootlogoComando();

#endif