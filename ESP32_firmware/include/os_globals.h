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

enum EstadoSistema {
  MENU_PRINCIPAL,
  APP_TERMINAL,
  APP_CALCULADORA,
  APP_SNAKE,
  POPUP_DESLIGAR
};
extern EstadoSistema estadoAtual;
extern EstadoSistema estadoAnterior;

extern unsigned long tempoAberturaPopup;
extern int segundosRestantes;
extern bool estadoAnteriorBotao;
extern unsigned long ultimoDebounce;

extern FileNode sistemaArquivos;
extern std::vector<MenuItem> menuAtual;
extern int opcaoSelecionada;
extern int marqueeOffset;
extern unsigned long lastMarqueeUpdate;

extern bool forcarRedrawSnake; 

extern String cacheLinhas[MAX_LINHAS_CACHE];
extern uint16_t cacheCores[MAX_LINHAS_CACHE][MAX_CHARS_LINHA]; 
extern int idxLinhaCache;
extern int indiceLinhaInicioPagina;
extern int scrollLinha;

extern volatile bool uartOcupada;

// GUI
void desenharMenu(bool telaDividida = false);
void atualizarMarquee(bool telaDividida = false);
void desenharCabecalho(bool telaDividida = false);
void desenharPopup(int segundos);
void fecharPopup();
void animacaoDeBoot();
void escreverEfeitoDigitacao(const String& texto, int tamanhoFonte, uint16_t cor);
void desenharIcone(int x, int y, int tipo);
void atualizarListaMenu();
void processarEntradaMenu();

// Terminal
void iniciarTerminal();
void sairTerminal();
void limparCache();
void novaLinhaCache();
void adicionarAoCache(char c, uint16_t cor);
void desenharLinhaCache(int indice, int y);
void restaurarPaginaAtual();
void renderizarScroll();
void avancarLinha(uint16_t corRestaurar = ST77XX_GREEN, int tamanhoFonteRestaurar = 2);
String limparAcentos(const String& textoOriginal);
void processarEntradaTerminal();

// Hardware
void desligarSistema();
void verificarBotaoFisico();

#endif