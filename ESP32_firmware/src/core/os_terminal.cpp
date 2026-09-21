#include "../../include/os_globals.h"
#include "../../include/apps/app_explorer.h"

void iniciarTerminal() {
  estadoAtual = APP_TERMINAL;
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho(true);
  desenharMenu(true);
  limparCache();
  cursorX = (tft.width() / 2) + 6;
  cursorY = 32;
  escreverEfeitoDigitacao("CMD> ", 2, ST77XX_WHITE);
}

void sairTerminal() {
  estadoAtual = MENU_PRINCIPAL;
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho(false);
  desenharMenu(false);
}

void limparCache() {
  for (int i = 0; i < MAX_LINHAS_CACHE; i++) {
    cacheLinhas[i] = "";
    for (int j = 0; j < MAX_CHARS_LINHA; j++) {
      cacheCores[i][j] = ST77XX_WHITE;
    }
  }
  idxLinhaCache = 0;
  indiceLinhaInicioPagina = 0;
  scrollLinha = 0;
}

void novaLinhaCache() {
  if (idxLinhaCache < MAX_LINHAS_CACHE - 1) {
    idxLinhaCache++;
  } else {
    for (int i = 0; i < MAX_LINHAS_CACHE - 1; i++) {
      cacheLinhas[i] = cacheLinhas[i + 1];
      for (int j = 0; j < MAX_CHARS_LINHA; j++) {
        cacheCores[i][j] = cacheCores[i + 1][j];
      }
    }
    cacheLinhas[MAX_LINHAS_CACHE - 1] = "";
    for (int j = 0; j < MAX_CHARS_LINHA; j++) {
      cacheCores[MAX_LINHAS_CACHE - 1][j] = ST77XX_WHITE;
    }
  }
}

void adicionarAoCache(char c, uint16_t cor) {
  int len = cacheLinhas[idxLinhaCache].length();
  if (len < MAX_CHARS_LINHA - 1) {
    cacheLinhas[idxLinhaCache] += c;
    cacheCores[idxLinhaCache][len] = cor;
  }
}

void desenharLinhaCache(int indice, int y) {
  int startX = (estadoAtual == APP_TERMINAL || estadoAtual == APP_CALCULADORA) ? (tft.width() / 2) + 6 : margemEsquerda;
  int x = startX;
  tft.setTextSize(2);
  for (size_t i = 0; i < cacheLinhas[indice].length(); i++) {
    tft.setTextColor(cacheCores[indice][i]);
    tft.setCursor(x, y);
    tft.print(cacheLinhas[indice][i]);
    x += 12;
  }
}

void restaurarPaginaAtual() {
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho(estadoAtual == APP_TERMINAL || estadoAtual == APP_CALCULADORA);
  if (estadoAtual == APP_TERMINAL || estadoAtual == APP_CALCULADORA) desenharMenu(true);
  
  int areaY = tft.height() - 32 - 10;
  int maxLinhasVisiveis = areaY / 20;
  int linhaInicioRender = idxLinhaCache - maxLinhasVisiveis + 1;
  if (linhaInicioRender < indiceLinhaInicioPagina) {
    linhaInicioRender = indiceLinhaInicioPagina;
  }
  if (linhaInicioRender < 0) linhaInicioRender = 0;

  int y = 32;
  for (int i = linhaInicioRender; i <= idxLinhaCache; i++) {
    desenharLinhaCache(i, y);
    y += 20;
  }
  int startX = (estadoAtual == APP_TERMINAL || estadoAtual == APP_CALCULADORA) ? (tft.width() / 2) + 6 : margemEsquerda;
  cursorX = startX;
  if (cacheLinhas[idxLinhaCache].length() > 0) {
      cursorX = startX + (cacheLinhas[idxLinhaCache].length() * 12);
  }
  cursorY = y - 20;
}

void renderizarScroll() {
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho(estadoAtual == APP_TERMINAL || estadoAtual == APP_CALCULADORA);
  if (estadoAtual == APP_TERMINAL || estadoAtual == APP_CALCULADORA) desenharMenu(true);

  int areaY = tft.height() - 32 - 10;
  int maxLinhasVisiveis = areaY / 20;
  
  int linhaFimScroll = idxLinhaCache - scrollLinha;
  if (linhaFimScroll < 0) linhaFimScroll = 0;

  int linhaInicioScroll = linhaFimScroll - maxLinhasVisiveis + 1;
  if (linhaInicioScroll < 0) linhaInicioScroll = 0;

  int y = 32;
  for (int i = linhaInicioScroll; i <= linhaFimScroll; i++) {
    desenharLinhaCache(i, y);
    y += 20;
  }

  int barraX = tft.width() - 10;
  tft.fillRect(barraX, 32, 4, areaY, COR_CINZA);
  
  int totalLinhas = idxLinhaCache + 1;
  if (totalLinhas > maxLinhasVisiveis) {
      float proporcao = (float)maxLinhasVisiveis / totalLinhas;
      int alturaBarra = proporcao * areaY;
      if (alturaBarra < 10) alturaBarra = 10; 
      
      float progresso = 1.0 - ((float)scrollLinha / (totalLinhas - maxLinhasVisiveis));
      int maxYBarra = (tft.height() - 10) - alturaBarra;
      int yBarra = 32 + (progresso * (maxYBarra - 32));
      
      tft.fillRect(barraX - 1, yBarra, 6, alturaBarra, ST77XX_WHITE);
  }
}

void avancarLinha(uint16_t corRestaurar, int tamanhoFonteRestaurar) {
  cursorY += 20; 
  int startX = (estadoAtual == APP_TERMINAL || estadoAtual == APP_CALCULADORA) ? (tft.width() / 2) + 6 : margemEsquerda;
  cursorX = startX; 
  novaLinhaCache();
  
  if (cursorY > tft.height() - 24) {
    restaurarPaginaAtual(); 
    cursorY = tft.height() - 24; 
  }
  tft.setTextColor(corRestaurar); 
}

String limparAcentos(const String& textoOriginal) {
  String texto = textoOriginal;
  texto.replace("á", "a"); texto.replace("à", "a"); texto.replace("â", "a"); texto.replace("ã", "a");
  texto.replace("é", "e"); texto.replace("è", "e"); texto.replace("ê", "e");
  texto.replace("í", "i"); texto.replace("ì", "i"); texto.replace("î", "i");
  texto.replace("ó", "o"); texto.replace("ò", "o"); texto.replace("ô", "o"); texto.replace("õ", "o");
  texto.replace("ú", "u"); texto.replace("ù", "u"); texto.replace("û", "u");
  texto.replace("ç", "c");
  texto.replace("Á", "A"); texto.replace("À", "A"); texto.replace("Â", "A"); texto.replace("Ã", "A");
  texto.replace("É", "E"); texto.replace("È", "E"); texto.replace("Ê", "E");
  texto.replace("Í", "I"); texto.replace("Ì", "I"); texto.replace("Î", "I");
  texto.replace("Ó", "O"); texto.replace("Ò", "O"); texto.replace("Ô", "O"); texto.replace("Õ", "O");
  texto.replace("Ú", "U"); texto.replace("Ù", "U"); texto.replace("Û", "U");
  texto.replace("Ç", "C");
  return texto;
}

void processarEntradaTerminal() {
  if (!Serial.available()) return;
  String textoLido = Serial.readStringUntil('\n'); 
  textoLido.trim(); 
  if (textoLido.length() == 0) return;

  if (scrollLinha == 0) {
    tft.fillRect(cursorX, cursorY, 12, 16, ST77XX_BLACK);
  }

  textoLido = limparAcentos(textoLido);
  String textoUpper = textoLido;
  textoUpper.toUpperCase();

  if (textoUpper == "EXIT") {
    sairTerminal();
    return; 
  }

  if (textoUpper == "CALC" && estadoAtual != APP_CALCULADORA) {
    estadoAtual = APP_CALCULADORA; 
    int startX = (tft.width() / 2) + 6;
    if (cursorX > startX) avancarLinha(ST77XX_GREEN, 2);
    escreverEfeitoDigitacao("CALC ATIVADA. EXIT para sair.", 2, ST77XX_GREEN);
    avancarLinha(ST77XX_CYAN, 2);
    escreverEfeitoDigitacao("calc> ", 2, ST77XX_CYAN);
    esperandoTexto = true;
    return; 
  }

  if (scrollLinha > 0 && textoUpper != "UP" && textoUpper != "DOWN") {
    scrollLinha = 0; restaurarPaginaAtual();
  }

  escreverEfeitoDigitacao(textoLido, 2, ST77XX_GREEN);
  avancarLinha(ST77XX_GREEN, 2);

  if (textoUpper == "CLEAR") {
    tft.fillScreen(ST77XX_BLACK); 
    desenharCabecalho(true); 
    desenharMenu(true);
    cursorX = (tft.width() / 2) + 6; 
    cursorY = inicioTextoY;
    limparCache();
  } 
  else if (textoUpper == "UP") {
    int maxScroll = idxLinhaCache;
    if (scrollLinha < maxScroll) {
      scrollLinha += 3;
      if (scrollLinha > maxScroll) scrollLinha = maxScroll;
      renderizarScroll();
    }
  }
  else if (textoUpper == "DOWN") {
    if (scrollLinha > 0) {
      scrollLinha -= 3;
      if (scrollLinha <= 0) {
        scrollLinha = 0;
        restaurarPaginaAtual();
      } else {
        renderizarScroll();
      }
    }
  }
  else if (textoUpper == "HELP") {
    String mensagensAjuda[] = {
      "COMANDOS DISPONIVEIS:", 
      "CALC (Calculadora)",
      "CLEAR (Limpa a tela)",
      "EXIT (Volta ao Menu)",
      "HELP (Lista comandos)",
      "REBOOT (Reinicia OS)",
      "SHUTDOWN (Desliga OS)",
      "UP / DOWN (Rolagem)"
    };
    int numMensagens = 8; 
    bool interrompido = false;
    for (int i = 0; i < numMensagens; i += 2) {
      tft.fillScreen(ST77XX_BLACK); 
      desenharCabecalho(true);
      desenharMenu(true); 
      cursorX = (tft.width() / 2) + 6; 
      cursorY = inicioTextoY;
      novaLinhaCache(); 
      indiceLinhaInicioPagina = idxLinhaCache;       
      for (int j = 0; j < 2; j++) {
        if (i + j < numMensagens) {
          if (Serial.available()) { interrompido = true; break; }
          escreverEfeitoDigitacao(mensagensAjuda[i + j], 2, ST77XX_GREEN);
          avancarLinha(ST77XX_GREEN, 2);
        }
      }
      if (interrompido) break;
      
      for (int p = 0; p < 3; p++) {
        unsigned long startWait = millis();
        while (millis() - startWait < 666) { if (Serial.available()) { interrompido = true; break; } delay(10); }
        if (interrompido) break;
        escreverEfeitoDigitacao(".", 2, ST77XX_GREEN);
        if (Serial.available()) { interrompido = true; break; }
      }
      if (interrompido) break;
    }
    
    if (!interrompido) {
      tft.fillScreen(ST77XX_BLACK); 
      desenharCabecalho(true);
      desenharMenu(true); 
      cursorX = (tft.width() / 2) + 6; 
      cursorY = inicioTextoY;
      novaLinhaCache(); 
      indiceLinhaInicioPagina = idxLinhaCache;
    }
  }
  else if (textoUpper == "SHUTDOWN") {
    desligarSistema();
  }
  else if (textoUpper == "REBOOT") {
    tft.fillScreen(ST77XX_BLACK);
    ESP.restart();
  }
  else {
    escreverEfeitoDigitacao("Erro: Comando nao reconhecido.", 2, ST77XX_GREEN);
  }

  if (textoUpper != "UP" && textoUpper != "DOWN") {
    int startX = (tft.width() / 2) + 6;
    if (cursorX > startX) {
       avancarLinha(ST77XX_WHITE, 2);
    }
    escreverEfeitoDigitacao("CMD> ", 2, ST77XX_WHITE);
  }
}