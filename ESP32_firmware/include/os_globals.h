#ifndef OS_GLOBALS_H
#define OS_GLOBALS_H

#include "os_config.h"

extern Adafruit_ST7789 tft;
extern MathParser parser;

#define COR_CINZA tft.color565(130, 130, 130)
#define COR_FUNDO_SELECIONADO tft.color565(50, 50, 50) 

extern int cursorX;
extern int cursorY;
extern unsigned long tempoUltimoBlink;
extern bool cursorVisivel;
extern bool esperandoTexto; 
extern String tituloAtual;
extern bool modoComando; 
extern bool modoCalc;
extern bool telaInicialCreditos;

extern bool popupAberto;
extern unsigned long tempoAberturaPopup;
extern int segundosRestantes;
extern bool estadoAnteriorBotao;
extern unsigned long ultimoDebounce;

extern FileNode sistemaArquivos;
extern std::vector<MenuItem> menuAtual;
extern int opcaoSelecionada;

extern int marqueeOffset;
extern unsigned long lastMarqueeUpdate;

extern bool modoSnake;
extern bool forcarRedrawSnake; 

extern String cacheLinhas[MAX_LINHAS_CACHE];
extern uint16_t cacheCores[MAX_LINHAS_CACHE][MAX_CHARS_LINHA]; 
extern int idxLinhaCache;
extern int indiceLinhaInicioPagina;
extern int scrollLinha;

// Nova trava de segurança para sincronizar os dois processadores
extern volatile bool uartOcupada;

// GUI
void desenharMenu();
void atualizarMarquee();
void desenharCabecalho();
void desenharPopup(int segundos);
void fecharPopup();
void exibirBootlogoComando();
void animacaoDeBoot();
void escreverEfeitoDigitacao(const String& texto, int tamanhoFonte, uint16_t cor);
void desenharIcone(int x, int y, int tipo);
void atualizarListaMenu();

// Terminal
void limparCache();
void novaLinhaCache();
void adicionarAoCache(char c, uint16_t cor);
void desenharLinhaCache(int indice, int y);
void restaurarPaginaAtual();
void renderizarScroll();
// O Default Argument (= ST77XX_GREEN) fica OBRIGATORIAMENTE apenas na declaração (.h)
void avancarLinha(uint16_t corRestaurar = ST77XX_GREEN, int tamanhoFonteRestaurar = 1);
String limparAcentos(const String& textoOriginal);
void processarEntradaTerminal();

// Hardware
void desligarSistema();
void verificarBotaoFisico();

#endif