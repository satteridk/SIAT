#include "../../include/os_globals.h"

void desenharIcone(int x, int y, int tipo) {
  if (tipo == ICONE_PASTA_FECHADA) {
    tft.fillRect(x, y + 2, 10, 7, tft.color565(255, 215, 0)); 
    tft.fillRect(x, y, 5, 2, tft.color565(255, 215, 0));
  } 
  else if (tipo == ICONE_PASTA_ABERTA) {
    tft.fillRect(x, y + 2, 10, 7, tft.color565(255, 215, 0)); 
    tft.fillRect(x, y, 5, 2, tft.color565(255, 215, 0));      
    tft.fillRect(x + 1, y + 4, 10, 6, tft.color565(255, 245, 150)); 
  } 
  else if (tipo == ICONE_JOYSTICK) {
    tft.fillRoundRect(x, y + 2, 11, 7, 2, ST77XX_WHITE);
    tft.drawFastHLine(x + 2, y + 5, 3, ST77XX_BLACK); 
    tft.drawFastVLine(x + 3, y + 4, 3, ST77XX_BLACK); 
    tft.drawPixel(x + 7, y + 6, ST77XX_RED);          
    tft.drawPixel(x + 9, y + 4, ST77XX_RED);        
  } 
  else if (tipo == ICONE_MONITOR) {
    tft.fillRect(x + 1, y + 6, 4, 4, ST77XX_WHITE); 
    tft.drawFastVLine(x + 4, y + 0, 7, ST77XX_WHITE); 
    tft.fillRect(x + 5, y + 0, 3, 2, ST77XX_WHITE); 
    tft.drawPixel(x + 8, y + 2, ST77XX_WHITE);
  }
  else if (tipo == ICONE_ARQUIVO) {
    tft.fillRect(x + 2, y + 1, 6, 8, ST77XX_WHITE);
    tft.drawFastHLine(x + 3, y + 3, 3, COR_CINZA);
    tft.drawFastHLine(x + 3, y + 5, 3, COR_CINZA);
  }
  else if (tipo == ICONE_MUSICA) {
    tft.drawFastVLine(x + 3, y + 2, 5, ST77XX_WHITE);
    tft.drawFastVLine(x + 7, y + 1, 5, ST77XX_WHITE);
    tft.drawFastHLine(x + 4, y + 1, 4, ST77XX_WHITE);
    tft.fillCircle(x + 2, y + 6, 2, ST77XX_WHITE);
    tft.fillCircle(x + 6, y + 5, 2, ST77XX_WHITE);
  }
  else if (tipo == ICONE_TEXTO) {
    tft.fillRect(x + 2, y + 1, 6, 8, ST77XX_WHITE);
    tft.drawFastHLine(x + 3, y + 3, 4, ST77XX_BLACK);
    tft.drawFastHLine(x + 3, y + 5, 4, ST77XX_BLACK);
    tft.drawFastHLine(x + 3, y + 7, 2, ST77XX_BLACK);
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

void desenharMenu() {
  int topY = 15;
  int limiteX = (tft.width() * 0.6) + 4; 
  int menuX = limiteX + 1; 
  int menuY = topY + 1;
  int menuW = tft.width() - limiteX - 4; 
  int menuH = tft.height() - topY - 4; 

  tft.fillRect(menuX, menuY, menuW, menuH, ST77XX_BLACK);
  tft.setTextSize(1);

  int maxVisible = menuH / 16;
  int startIdx = 0;
  if (opcaoSelecionada >= maxVisible) {
    startIdx = opcaoSelecionada - maxVisible + 1;
  }

  int drawY = menuY + 6;
  for (size_t i = startIdx; i < menuAtual.size(); i++) {
    if (drawY > menuY + menuH - 12) break;
    
    int baseX = menuX + 4 + (menuAtual[i].indent * 6);
    
    if (opcaoSelecionada == (int)i) {
      tft.fillRect(menuX + 2, drawY - 2, menuW - 4, 14, COR_FUNDO_SELECIONADO);
      tft.setTextColor(ST77XX_WHITE);
    } else {
      tft.setTextColor(COR_CINZA);
    }
    
    if (menuAtual[i].indent > 0) {
      tft.drawFastVLine(baseX - 4, drawY - 8, 14, COR_CINZA);
      tft.drawFastHLine(baseX - 4, drawY + 4, 6, COR_CINZA);
    }
    
    desenharIcone(baseX, drawY, menuAtual[i].icon);
    
    String txt = menuAtual[i].name;
    int maxChars = (menuW - (baseX - menuX) - 18) / 6;
    
    if (opcaoSelecionada == (int)i && (int)txt.length() > maxChars) {
      int offset = min(marqueeOffset, (int)(txt.length() - maxChars));
      txt = txt.substring(offset, offset + maxChars);
    } else if ((int)txt.length() > maxChars) {
      txt = txt.substring(0, maxChars);
    }
    
    tft.setCursor(baseX + 16, drawY + 2);
    tft.print(txt);
    
    drawY += 16;
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
  int baseX = menuX + 4 + (menuAtual[opcaoSelecionada].indent * 6);
  int maxChars = (menuW - (baseX - menuX) - 18) / 6;

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
    
    int maxVisible = menuH / 16;
    int startIdx = 0;
    if (opcaoSelecionada >= maxVisible) {
      startIdx = opcaoSelecionada - maxVisible + 1;
    }
    
    int drawY = menuY + 6 + ((opcaoSelecionada - startIdx) * 16);
    
    tft.fillRect(baseX + 16, drawY - 2, maxChars * 6, 14, COR_FUNDO_SELECIONADO);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(baseX + 16, drawY + 2);
    tft.print(renderTxt);
  }
}

void desenharCabecalho() {
  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE); tft.setCursor(margemEsquerda, 5);
  int maxCaracteres = (tft.width() - (margemEsquerda * 2)) / 6; 
  String textoCentral = " " + tituloAtual + " "; 
  int espacoRestante = maxCaracteres - textoCentral.length();
  if (espacoRestante < 0) espacoRestante = 0; 
  String linha = "";
  for(int i = 0; i < (espacoRestante / 2); i++) linha += "=";
  linha += textoCentral;
  for(int i = 0; i < (espacoRestante - (espacoRestante / 2)); i++) linha += "=";
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

  tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(x + 10, y + 10); tft.print("Para desligar,");
  tft.setCursor(x + 10, y + 25); tft.print("pressione de novo: ");
  tft.print(segundos); tft.print("s  ");    
}

void fecharPopup() {
  estadoAtual = estadoAnterior;
  if (estadoAtual == APP_SNAKE) {
    forcarRedrawSnake = true; 
  } else {
    if (scrollLinha > 0) {
      renderizarScroll();
    } else {
      restaurarPaginaAtual();
    }
  }
}

void exibirBootlogoComando() {
  tft.fillScreen(ST77XX_BLACK); 
  tft.setTextColor(ST77XX_WHITE);
  String logoTexto = "S.I.A.T";
  String logoSimbolo = "(c)"; 
  int larguraCharSize3 = 18; 
  int alturaCharSize3 = 24;  
  int alturaCharSize1 = 8;
  int larguraCharSize1 = 6;

  int wTexto = logoTexto.length() * larguraCharSize3;
  int logoX = (tft.width() - wTexto) / 2;
  int logoY = (tft.height() - alturaCharSize3) / 2 - 15; 
  
  tft.setTextSize(3);
  tft.setCursor(logoX, logoY);
  tft.print(logoTexto);

  tft.setTextSize(1);
  int simboloY = logoY + (alturaCharSize3 - alturaCharSize1);
  tft.setCursor(logoX + wTexto + 2, simboloY); 
  tft.print(logoSimbolo);

  tft.setTextSize(1);
  String footer1 = "satter's S.I.A.T";
  String footer2 = "terminal v2.6"; 
  int f1W = footer1.length() * larguraCharSize1;
  int f2W = footer2.length() * larguraCharSize1;
  int f2Y = tft.height() - alturaCharSize1 - 5; 
  int f1Y = f2Y - 12; 
  int f1X = (tft.width() - f1W) / 2;
  int f2X = (tft.width() - f2W) / 2;

  tft.setCursor(f1X, f1Y);
  tft.print(footer1);
  tft.setCursor(f2X, f2Y);
  tft.print(footer2);

  while (!Serial.available()) {
    delay(10);
  }
  while (Serial.available()) {
    Serial.read();
    delay(2);
  }

  if (scrollLinha > 0) {
    renderizarScroll();
  } else {
    restaurarPaginaAtual();
  }
}

void animacaoDeBoot() {
  tft.fillScreen(ST77XX_BLACK); 
  tft.setTextColor(ST77XX_WHITE);
  String logoTexto = "S.I.A.T";
  String logoSimbolo = "(c)"; 
  int larguraCharSize3 = 18; 
  int alturaCharSize3 = 24;  
  int alturaCharSize1 = 8;
  int larguraCharSize1 = 6;

  int wTexto = logoTexto.length() * larguraCharSize3;
  int logoX = (tft.width() - wTexto) / 2;
  int logoY = (tft.height() - alturaCharSize3) / 2 - 15; 
  
  tft.setTextSize(3);
  tft.setCursor(logoX, logoY);
  tft.print(logoTexto);

  tft.setTextSize(1);
  int simboloY = logoY + (alturaCharSize3 - alturaCharSize1);
  tft.setCursor(logoX + wTexto + 2, simboloY); 
  tft.print(logoSimbolo);

  tft.setTextSize(1);
  String footer1 = "satter's S.I.A.T";
  String footer2 = "terminal v2.6"; 
  int f1W = footer1.length() * larguraCharSize1;
  int f2W = footer2.length() * larguraCharSize1;
  int f2Y = tft.height() - alturaCharSize1 - 5; 
  int f1Y = f2Y - 12; 
  int f1X = (tft.width() - f1W) / 2;
  int f2X = (tft.width() - f2W) / 2;

  tft.setCursor(f1X, f1Y);
  tft.print(footer1);
  tft.setCursor(f2X, f2Y);
  tft.print(footer2);

  delay(2500);

  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho();

  cursorX = margemEsquerda; 
  cursorY = inicioTextoY + 12; 
  limparCache();

  tft.setTextColor(COR_CINZA);
  tft.setTextSize(1);
  tft.setCursor(margemEsquerda, cursorY);
  tft.print("Linguagem: PT-BR");
  cursorY += 12;
  tft.setCursor(margemEsquerda, cursorY);
  tft.print("Feito por: satter");
  cursorY += 12;
  tft.setCursor(margemEsquerda, cursorY);
  tft.print("Sistema Iniciado.");

  cursorX = margemEsquerda; 
  cursorY = inicioTextoY;
  escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
}

void escreverEfeitoDigitacao(const String& texto, int tamanhoFonte, uint16_t cor) {
  tft.setTextSize(tamanhoFonte); 
  tft.setTextColor(cor);
  int larguraLetra = 6 * tamanhoFonte;
  int limiteTextoX = (tft.width() * 0.6) - 4; 

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
      
      if (cursorX > margemEsquerda && (cursorX + larguraPalavra > limiteTextoX)) {
        int espacoRestanteX = limiteTextoX - cursorX;
        int caracteresQueCabem = espacoRestanteX / larguraLetra;
        int caracteresSaindo = caracteresPalavra - caracteresQueCabem;
        if (caracteresQueCabem <= 0 || caracteresSaindo < 4) {
          avancarLinha(cor, tamanhoFonte);
        }
      }
    }

    if (cursorX == margemEsquerda && letra == ' ') continue;

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