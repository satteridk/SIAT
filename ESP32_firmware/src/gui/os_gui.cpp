#include "../../include/os_globals.h"
#include "../../include/apps/app_explorer.h"

void desenharIcone(int x, int y, int tipo) {
  if (tipo == ICONE_PASTA_FECHADA) {
    tft.fillRect(x, y + 4, 20, 14, tft.color565(255, 215, 0)); 
    tft.fillRect(x, y, 10, 4, tft.color565(255, 215, 0));
  } 
  else if (tipo == ICONE_PASTA_ABERTA) {
    tft.fillRect(x, y + 4, 20, 14, tft.color565(255, 215, 0)); 
    tft.fillRect(x, y, 10, 4, tft.color565(255, 215, 0));      
    tft.fillRect(x + 2, y + 8, 20, 12, tft.color565(255, 245, 150)); 
  } 
  else if (tipo == ICONE_JOYSTICK) {
    tft.fillRoundRect(x, y + 4, 22, 14, 4, ST77XX_WHITE);
    tft.drawFastHLine(x + 4, y + 10, 6, ST77XX_BLACK); 
    tft.drawFastVLine(x + 6, y + 8, 6, ST77XX_BLACK); 
    tft.fillRect(x + 14, y + 12, 2, 2, ST77XX_RED);          
    tft.fillRect(x + 18, y + 8, 2, 2, ST77XX_RED);        
  } 
  else if (tipo == ICONE_MONITOR) {
    tft.fillRect(x + 2, y + 12, 8, 8, ST77XX_WHITE); 
    tft.fillRect(x + 8, y + 0, 2, 14, ST77XX_WHITE); 
    tft.fillRect(x + 10, y + 0, 6, 4, ST77XX_WHITE); 
  }
  else if (tipo == ICONE_ARQUIVO) {
    tft.fillRect(x + 4, y + 2, 12, 16, ST77XX_WHITE);
    tft.drawFastHLine(x + 6, y + 6, 6, COR_CINZA);
    tft.drawFastHLine(x + 6, y + 10, 6, COR_CINZA);
  }
  else if (tipo == ICONE_MUSICA) {
    tft.drawFastVLine(x + 6, y + 4, 10, ST77XX_WHITE);
    tft.drawFastVLine(x + 14, y + 2, 10, ST77XX_WHITE);
    tft.drawFastHLine(x + 8, y + 2, 8, ST77XX_WHITE);
    tft.fillCircle(x + 4, y + 12, 4, ST77XX_WHITE);
    tft.fillCircle(x + 12, y + 10, 4, ST77XX_WHITE);
  }
  else if (tipo == ICONE_TEXTO) {
    tft.fillRect(x + 4, y + 2, 12, 16, ST77XX_WHITE);
    tft.fillRect(x + 6, y + 6, 8, 2, ST77XX_BLACK);
    tft.fillRect(x + 6, y + 10, 8, 2, ST77XX_BLACK);
    tft.fillRect(x + 6, y + 14, 4, 2, ST77XX_BLACK);
  }
}

void adicionarNoMenu(FileNode& dir, int indent) {
  for (size_t i = 0; i < dir.children.size(); i++) {
    MenuItem item;
    item.name = dir.children[i].name;
    
    if (dir.children[i].isDir) {
      item.icon = dir.children[i].expanded ? ICONE_PASTA_ABERTA : ICONE_PASTA_FECHADA;
    } else {
      String nomeUpper = dir.children[i].name;
      nomeUpper.toUpperCase();
      
      if (nomeUpper.endsWith(".MP3") || nomeUpper.endsWith(".WAV") || nomeUpper.endsWith(".OGG") || nomeUpper.endsWith(".FLAC")) {
        item.icon = ICONE_MUSICA;
      } else if (nomeUpper.endsWith(".TXT") || nomeUpper.endsWith(".DAT") || nomeUpper.endsWith(".INI") || nomeUpper.endsWith(".CONFIG") || nomeUpper.endsWith(".CSV")) {
        item.icon = ICONE_TEXTO;
      } else {
        item.icon = ICONE_ARQUIVO; 
      }
    }
    
    item.actionId = 3;
    item.node = &dir.children[i];
    item.indent = indent;
    menuAtual.push_back(item);
    
    if (dir.children[i].isDir && dir.children[i].expanded) {
      adicionarNoMenu(dir.children[i], indent + 1);
    }
  }
}

void atualizarListaMenu() {
  menuAtual.clear();
  
  MenuItem t; t.name = "Terminal"; t.icon = ICONE_TEXTO; t.actionId = 4; t.indent = 0; t.node = nullptr;
  menuAtual.push_back(t);
  
  MenuItem exp; 
  exp.name = sistemaArquivos.name; 
  exp.icon = sistemaArquivos.expanded ? ICONE_PASTA_ABERTA : ICONE_PASTA_FECHADA; 
  exp.actionId = 3; 
  exp.indent = 0; 
  exp.node = &sistemaArquivos;
  menuAtual.push_back(exp);

  if (sistemaArquivos.expanded) {
    adicionarNoMenu(sistemaArquivos, 1);
  }

  MenuItem s; s.name = "Snake"; s.icon = ICONE_JOYSTICK; s.actionId = 1; s.indent = 0; s.node = nullptr;
  menuAtual.push_back(s);
  MenuItem a; a.name = "AudioPlayer"; a.icon = ICONE_MONITOR; a.actionId = 2; a.indent = 0; a.node = nullptr;
  menuAtual.push_back(a);

  if (opcaoSelecionada >= (int)menuAtual.size()) {
    opcaoSelecionada = menuAtual.size() > 0 ? menuAtual.size() - 1 : 0;
  }
}

void desenharMenu(bool telaDividida) {
  int topY = 25;
  int limiteX = tft.width() / 2; 
  int menuX = 2; 
  int menuY = topY + 1;
  int menuW = telaDividida ? limiteX - 4 : tft.width() - 4; 
  int menuH = tft.height() - topY - 4; 

  tft.fillRect(menuX, menuY, menuW, menuH, ST77XX_BLACK);
  tft.setTextSize(2);

  int itemHeight = 32;
  int maxVisible = menuH / itemHeight;
  int startIdx = 0;
  if (opcaoSelecionada >= maxVisible) {
    startIdx = opcaoSelecionada - maxVisible + 1;
  }

  int drawY = menuY + 8;
  for (size_t i = startIdx; i < menuAtual.size(); i++) {
    if (drawY > menuY + menuH - 24) break;
    
    int baseX = menuX + 8 + (menuAtual[i].indent * 12);
    
    if (opcaoSelecionada == (int)i) {
      tft.fillRect(menuX + 4, drawY - 4, menuW - 8, 28, COR_FUNDO_SELECIONADO);
      tft.setTextColor(ST77XX_WHITE);
    } else {
      tft.setTextColor(COR_CINZA);
    }
    
    if (menuAtual[i].indent > 0) {
      tft.drawFastVLine(baseX - 8, drawY - 16, 28, COR_CINZA);
      tft.drawFastHLine(baseX - 8, drawY + 8, 12, COR_CINZA);
    }
    
    desenharIcone(baseX, drawY, menuAtual[i].icon);
    
    String txt = menuAtual[i].name;
    int maxChars = (menuW - (baseX - menuX) - 36) / 12;
    
    if (opcaoSelecionada == (int)i && (int)txt.length() > maxChars) {
      int offset = min(marqueeOffset, (int)(txt.length() - maxChars));
      txt = txt.substring(offset, offset + maxChars);
    } else if ((int)txt.length() > maxChars) {
      txt = txt.substring(0, maxChars);
    }
    
    tft.setCursor(baseX + 32, drawY + 4);
    tft.print(txt);
    
    drawY += itemHeight;
  }
}

void atualizarMarquee(bool telaDividida) {
  if (opcaoSelecionada >= (int)menuAtual.size()) return;
  int topY = 25;
  int limiteX = tft.width() / 2; 
  int menuX = 2; 
  int menuY = topY + 1;
  int menuW = telaDividida ? limiteX - 4 : tft.width() - 4; 
  int menuH = tft.height() - topY - 4; 

  String txt = menuAtual[opcaoSelecionada].name;
  int baseX = menuX + 8 + (menuAtual[opcaoSelecionada].indent * 12);
  int maxChars = (menuW - (baseX - menuX) - 36) / 12;

  if ((int)txt.length() <= maxChars) {
    marqueeOffset = 0;
    return;
  }

  if (millis() - lastMarqueeUpdate > 300) {
    lastMarqueeUpdate = millis();
    marqueeOffset++;
    
    if (marqueeOffset > (int)txt.length() - maxChars + 4) {
      marqueeOffset = 0;
    }
    
    int renderOffset = min(marqueeOffset, (int)(txt.length() - maxChars));
    String renderTxt = txt.substring(renderOffset, renderOffset + maxChars);
    
    int itemHeight = 32;
    int maxVisible = menuH / itemHeight;
    int startIdx = 0;
    if (opcaoSelecionada >= maxVisible) {
      startIdx = opcaoSelecionada - maxVisible + 1;
    }
    
    int drawY = menuY + 8 + ((opcaoSelecionada - startIdx) * itemHeight);
    
    tft.fillRect(baseX + 32, drawY - 4, maxChars * 12, 28, COR_FUNDO_SELECIONADO);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(baseX + 32, drawY + 4);
    tft.print(renderTxt);
  }
}

void desenharCabecalho(bool telaDividida) {
  tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE); tft.setCursor(margemEsquerda, 5);
  int maxCaracteres = tft.width() / 12; 
  String textoCentral = " " + tituloAtual + " "; 
  int espacoRestante = maxCaracteres - textoCentral.length();
  if (espacoRestante < 0) espacoRestante = 0; 
  String linha = "";
  for(int i = 0; i < (espacoRestante / 2); i++) linha += "=";
  linha += textoCentral;
  for(int i = 0; i < (espacoRestante - (espacoRestante / 2)); i++) linha += "=";
  tft.print(linha);

  int topY = 25; 
  tft.drawRect(2, topY, tft.width() - 4, tft.height() - topY - 2, ST77XX_WHITE);
  if (telaDividida) {
    int limiteX = tft.width() / 2;
    tft.drawFastVLine(limiteX, topY, tft.height() - topY - 2, ST77XX_WHITE);
  }
  desenharMenu(telaDividida);
}

void desenharPopup(int segundos) {
  int larg = 240, alt = 80;
  int x = (tft.width() - larg) / 2;
  int y = (tft.height() - alt) / 2;

  if (segundos == 3) {
    tft.fillRect(x, y, larg, alt, ST77XX_BLACK);
    tft.drawRect(x, y, larg, alt, ST77XX_WHITE);
  }

  tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(x + 10, y + 15); tft.print("Para desligar,");
  tft.setCursor(x + 10, y + 45); tft.print("pressione: ");
  tft.print(segundos); tft.print("s ");    
}

void fecharPopup() {
  estadoAtual = estadoAnterior;
  if (estadoAtual == APP_SNAKE) {
    forcarRedrawSnake = true; 
  } else if (estadoAtual == MENU_PRINCIPAL) {
    desenharCabecalho(false);
    desenharMenu(false);
  } else {
    if (scrollLinha > 0) {
      renderizarScroll();
    } else {
      restaurarPaginaAtual();
    }
  }
}

void animacaoDeBoot() {
  tft.fillScreen(ST77XX_BLACK); 
  tft.setTextColor(ST77XX_WHITE);
  String logoTexto = "S.I.A.T";
  String logoSimbolo = "(c)"; 
  
  int larguraCharSize6 = 36; 
  int alturaCharSize6 = 48;  
  int alturaCharSize2 = 16;
  int larguraCharSize2 = 12;

  int wTexto = logoTexto.length() * larguraCharSize6;
  int logoX = (tft.width() - wTexto) / 2;
  int logoY = (tft.height() - alturaCharSize6) / 2 - 20; 
  
  tft.setTextSize(6);
  tft.setCursor(logoX, logoY);
  tft.print(logoTexto);

  tft.setTextSize(2);
  int simboloY = logoY + (alturaCharSize6 - alturaCharSize2);
  tft.setCursor(logoX + wTexto + 4, simboloY); 
  tft.print(logoSimbolo);

  tft.setTextSize(2);
  String footer1 = "satter's S.I.A.T";
  String footer2 = "terminal v2.7"; 
  int f1W = footer1.length() * larguraCharSize2;
  int f2W = footer2.length() * larguraCharSize2;
  int f2Y = tft.height() - alturaCharSize2 - 10; 
  int f1Y = f2Y - 24; 
  int f1X = (tft.width() - f1W) / 2;
  int f2X = (tft.width() - f2W) / 2;

  tft.setCursor(f1X, f1Y);
  tft.print(footer1);
  tft.setCursor(f2X, f2Y);
  tft.print(footer2);

  delay(2500);

  estadoAtual = MENU_PRINCIPAL;
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho(false);
  desenharMenu(false);
}

void processarEntradaMenu() {
  if (!Serial.available()) return;
  String textoLido = Serial.readStringUntil('\n'); 
  textoLido.trim(); 
  if (textoLido.length() == 0) return;
  
  textoLido.toUpperCase();
  
  if (textoLido == "W") {
    if (opcaoSelecionada > 0) opcaoSelecionada--;
    else opcaoSelecionada = menuAtual.size() - 1;
    marqueeOffset = 0; 
    desenharMenu(false);
  }
  else if (textoLido == "S") {
    if (opcaoSelecionada < (int)menuAtual.size() - 1) opcaoSelecionada++;
    else opcaoSelecionada = 0; 
    marqueeOffset = 0; 
    desenharMenu(false);
  }
  else if (textoLido == "E") {
    if (menuAtual.empty()) return;
    MenuItem& item = menuAtual[opcaoSelecionada];
    if (item.actionId == 4) {
      iniciarTerminal();
    } else {
      acaoExplorador(item);
    }
  }
}

void escreverEfeitoDigitacao(const String& texto, int tamanhoFonte, uint16_t cor) {
  tft.setTextSize(tamanhoFonte); 
  tft.setTextColor(cor);
  int larguraLetra = 6 * tamanhoFonte;
  int limiteTextoX = tft.width() - 4; 
  int startX = (estadoAtual == APP_TERMINAL || estadoAtual == APP_CALCULADORA) ? (tft.width() / 2) + 6 : margemEsquerda;

  for (size_t i = 0; i < texto.length(); i++) {
    if (Serial.available()) return;
    char letra = texto[i];
    if (letra < 0 || letra > 127) continue; 
    
    if (letra != ' ' && (i == 0 || texto[i - 1] == ' ')) {
      int caracteresPalavra = 0;
      for (size_t j = i; j < texto.length() && texto[j] != ' '; j++) {
        if (texto[j] >= 0 && texto[j] <= 127) caracteresPalavra++;
      }
      int larguraPalavra = caracteresPalavra * larguraLetra;
      
      if (cursorX > startX && (cursorX + larguraPalavra > limiteTextoX)) {
        int espacoRestanteX = limiteTextoX - cursorX;
        int caracteresQueCabem = espacoRestanteX / larguraLetra;
        int caracteresSaindo = caracteresPalavra - caracteresQueCabem;
        if (caracteresQueCabem <= 0 || caracteresSaindo < 4) {
          avancarLinha(cor, tamanhoFonte);
        }
      }
    }

    if (cursorX == startX && letra == ' ') continue;

    if (cursorX + larguraLetra > limiteTextoX) {
      if (letra == ' ') {
        avancarLinha(cor, tamanhoFonte);
        continue; 
      } 
      else {
        tft.setCursor(cursorX, cursorY);
        tft.print('-'); 
        adicionarAoCache('-', cor);         
        avancarLinha(cor, tamanhoFonte);        
        tft.setCursor(cursorX, cursorY);
        tft.print('-'); 
        adicionarAoCache('-', cor); 
        cursorX += larguraLetra;
      }
    }

    tft.setCursor(cursorX, cursorY); 
    tft.print(letra); 
    adicionarAoCache(letra, cor);
    cursorX += larguraLetra;
    
    int tempoEspera = (letra == '.') ? 100 : ((letra != ' ') ? 10 : 0);
    if (tempoEspera > 0) {
      unsigned long startWait = millis();
      while (millis() - startWait < (unsigned long)tempoEspera) {
        if (Serial.available()) return;
        delay(1);
      }
    }
  }
}