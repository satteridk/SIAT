#include "../../include/os_globals.h"
#include "../../include/apps/app_snake.h"
#include "../../include/apps/app_explorer.h"

// --- Estado proprio da GUI ---
String tituloAtual = "TERMINAL V2.6";
std::vector<MenuItem> menuAtual;
int opcaoSelecionada = 0;
int marqueeOffset = 0;
unsigned long lastMarqueeUpdate = 0;

// ==========================================
// LISTA PRINCIPAL DE APLICATIVOS
// A arvore de arquivos nao vive mais aqui: ela pertence ao explorador.
// ==========================================
void atualizarListaMenu() {
  menuAtual.clear();

  MenuItem t; t.name = "Terminal";               t.icon = ICONE_CMD;           t.actionId = ACAO_TERMINAL;
  menuAtual.push_back(t);

  MenuItem e; e.name = "Explorador de arquivos"; e.icon = ICONE_PASTA_FECHADA; e.actionId = ACAO_EXPLORER;
  menuAtual.push_back(e);

  MenuItem s; s.name = "Snake";                  s.icon = ICONE_SNAKE;         s.actionId = ACAO_SNAKE;
  menuAtual.push_back(s);

  MenuItem a; a.name = "AudioPlayer";            a.icon = ICONE_MONITOR;       a.actionId = ACAO_AUDIO;
  menuAtual.push_back(a);

  if (opcaoSelecionada >= (int)menuAtual.size()) {
    opcaoSelecionada = menuAtual.size() > 0 ? menuAtual.size() - 1 : 0;
  }
}

// Despachante unico do menu (mantem o terminal desacoplado dos apps)
void executarItemMenu(MenuItem& item) {
  switch (item.actionId) {
    case ACAO_TERMINAL:
      restaurarTerminal();
      break;
    case ACAO_EXPLORER:
      iniciarExplorador();
      break;
    case ACAO_SNAKE:
      iniciarSnake();
      break;
    case ACAO_AUDIO:
      Serial.println("App/Jogo Selecionado: AudioPlayer");
      break;
    default:
      break;
  }
}

void desenharMenu() {
  int topY = 15;
  int limiteX = (tft.width() * 0.6) + 4;
  int menuX = limiteX + 1;
  int menuY = topY + 1;
  int menuW = tft.width() - limiteX - 4;
  int menuH = tft.height() - topY - 4;

  tft.fillRect(menuX, menuY, menuW, menuH, ST77XX_BLACK);
  tft.setTextSize(1);

  int maxVisible = menuH / ALTURA_ITEM_MENU;
  int startIdx = 0;
  if (opcaoSelecionada >= maxVisible) startIdx = opcaoSelecionada - maxVisible + 1;

  int drawY = menuY + 6;
  for (size_t i = startIdx; i < menuAtual.size(); i++) {
    if (drawY > menuY + menuH - 12) break;

    int baseX = menuX + 4;

    if (opcaoSelecionada == (int)i) {
      tft.fillRect(menuX + 2, drawY - 2, menuW - 4, 14, COR_FUNDO_SELECIONADO);
      tft.setTextColor(ST77XX_WHITE);
    } else {
      tft.setTextColor(COR_CINZA);
    }

    desenharIcone(baseX, drawY, menuAtual[i].icon);

    String txt = menuAtual[i].name;
    int maxChars = (menuW - 4 - 18) / LARGURA_CHAR;

    if (opcaoSelecionada == (int)i && (int)txt.length() > maxChars) {
      int offset = min(marqueeOffset, (int)(txt.length() - maxChars));
      txt = txt.substring(offset, offset + maxChars);
    } else if ((int)txt.length() > maxChars) {
      txt = txt.substring(0, maxChars);
    }

    tft.setCursor(baseX + 16, drawY + 2);
    tft.print(txt);

    drawY += ALTURA_ITEM_MENU;
  }
}

void atualizarMarquee() {
  if (opcaoSelecionada >= (int)menuAtual.size()) return;

  int topY = 15;
  int limiteX = (tft.width() * 0.6) + 4;
  int menuX = limiteX + 1;
  int menuY = topY + 1;
  int menuW = tft.width() - limiteX - 4;
  int menuH = tft.height() - topY - 4;

  String txt = menuAtual[opcaoSelecionada].name;
  int baseX = menuX + 4;
  int maxChars = (menuW - 4 - 18) / LARGURA_CHAR;

  if ((int)txt.length() <= maxChars) {
    marqueeOffset = 0;
    return;
  }

  if (millis() - lastMarqueeUpdate > 300) {
    lastMarqueeUpdate = millis();
    marqueeOffset++;

    if (marqueeOffset > (int)txt.length() - maxChars + 4) marqueeOffset = 0;

    int renderOffset = min(marqueeOffset, (int)(txt.length() - maxChars));
    String renderTxt = txt.substring(renderOffset, renderOffset + maxChars);

    int maxVisible = menuH / ALTURA_ITEM_MENU;
    int startIdx = 0;
    if (opcaoSelecionada >= maxVisible) startIdx = opcaoSelecionada - maxVisible + 1;

    int drawY = menuY + 6 + ((opcaoSelecionada - startIdx) * ALTURA_ITEM_MENU);

    tft.fillRect(baseX + 16, drawY - 2, maxChars * LARGURA_CHAR, 14, COR_FUNDO_SELECIONADO);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(baseX + 16, drawY + 2);
    tft.print(renderTxt);
  }
}

void desenharCabecalho() {
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(margemEsquerda, 5);

  int maxCaracteres = (tft.width() - (margemEsquerda * 2)) / LARGURA_CHAR;
  String textoCentral = " " + tituloAtual + " ";
  int espacoRestante = maxCaracteres - textoCentral.length();
  if (espacoRestante < 0) espacoRestante = 0;

  String linha = "";
  for (int i = 0; i < (espacoRestante / 2); i++) linha += "=";
  linha += textoCentral;
  for (int i = 0; i < (espacoRestante - (espacoRestante / 2)); i++) linha += "=";
  tft.print(linha);

  int topY = 15;
  tft.drawRect(2, topY, tft.width() - 4, tft.height() - topY - 2, ST77XX_WHITE);
  int limiteX = (tft.width() * 0.6) + 4;
  tft.drawFastVLine(limiteX, topY, tft.height() - topY - 2, ST77XX_WHITE);

  desenharMenu();
}

void desenharPopup(int segundos) {
  int larg = 190, alt = 60;
  int x = (tft.width() - larg) / 2;
  int y = (tft.height() - alt) / 2;

  if (segundos == 3) {
    tft.fillRect(x, y, larg, alt, ST77XX_BLACK);
    tft.drawRect(x, y, larg, alt, ST77XX_WHITE);
  }

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(x + 10, y + 10); tft.print("Para desligar,");
  tft.setCursor(x + 10, y + 25); tft.print("pressione de novo: ");
  tft.print(segundos); tft.print("s  ");
}

void fecharPopup() {
  estadoAtual = estadoAnterior;

  if (estadoAtual == APP_SNAKE) {
    forcarRedrawSnake = true;
  } else if (estadoAtual == APP_EXPLORADOR) {
    desenharExplorador();
  } else {
    if (scrollLinha > 0) renderizarScroll();
    else restaurarPaginaAtual();
  }
}