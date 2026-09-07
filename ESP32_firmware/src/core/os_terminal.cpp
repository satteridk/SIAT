#include "../../include/os_globals.h"
#include "../../include/apps/app_explorer.h"
#include "../../include/apps/app_calc.h"

void limparCache() {
  for(int i = 0; i < MAX_LINHAS_CACHE; i++) {
    cacheLinhas[i] = "";
    cacheLinhas[i].reserve(MAX_CHARS_LINHA + 5); 
    for(int j = 0; j < MAX_CHARS_LINHA; j++) {
      cacheCores[i][j] = ST77XX_GREEN; 
    }
  }
  idxLinhaCache = 0;
  indiceLinhaInicioPagina = 0;
  scrollLinha = 0;
}

void novaLinhaCache() {
  idxLinhaCache++;
  if (idxLinhaCache >= MAX_LINHAS_CACHE) {
    for(int k = 1; k < MAX_LINHAS_CACHE; k++) {
      cacheLinhas[k-1] = cacheLinhas[k];
      for(int c = 0; c < MAX_CHARS_LINHA; c++) {
        cacheCores[k-1][c] = cacheCores[k][c];
      }
    }
    idxLinhaCache = MAX_LINHAS_CACHE - 1;
    if (indiceLinhaInicioPagina > 0) indiceLinhaInicioPagina--;
  }
  cacheLinhas[idxLinhaCache] = "";
  for(int c = 0; c < MAX_CHARS_LINHA; c++) {
    cacheCores[idxLinhaCache][c] = ST77XX_GREEN;
  }
}

void adicionarAoCache(char c, uint16_t cor) {
  int pos = cacheLinhas[idxLinhaCache].length();
  if (pos < MAX_CHARS_LINHA) {
    cacheCores[idxLinhaCache][pos] = cor;
  }
  cacheLinhas[idxLinhaCache] += c;
}

void desenharLinhaCache(int indice, int y) {
  int currX = margemEsquerda;
  int len = cacheLinhas[indice].length();
  int startIdx = 0;
  
  while(startIdx < len) {
     uint16_t cCor = (startIdx < MAX_CHARS_LINHA) ? cacheCores[indice][startIdx] : ST77XX_GREEN;
     String chunk = "";
     int j = startIdx;
     
     while (j < len && ((j < MAX_CHARS_LINHA && cacheCores[indice][j] == cCor) || (j >= MAX_CHARS_LINHA && cCor == ST77XX_GREEN))) {
        chunk += cacheLinhas[indice][j];
        j++;
     }
     tft.setTextColor(cCor);
     tft.setCursor(currX, y);
     tft.print(chunk);
     currX += chunk.length() * 6; 
     startIdx = j;
  }
}

void restaurarPaginaAtual() {
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho();
  int tempY = inicioTextoY;
  tft.setTextSize(1);
  for (int i = indiceLinhaInicioPagina; i <= idxLinhaCache; i++) {
    if (i >= 0 && i < MAX_LINHAS_CACHE) {
      desenharLinhaCache(i, tempY);
      tempY += 12;
    }
  }
}

void renderizarScroll() {
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho();
  tft.setTextSize(1);

  int maxLinhasTela = (tft.height() - 4 - inicioTextoY) / 12;
  int linhaFim = idxLinhaCache - scrollLinha;
  int linhaInicio = linhaFim - maxLinhasTela + 1;
  
  if (linhaInicio < 0) linhaInicio = 0;

  int tempY = inicioTextoY;
  for (int i = linhaInicio; i <= linhaFim; i++) {
    if (i >= 0 && i < MAX_LINHAS_CACHE) {
      desenharLinhaCache(i, tempY);
      tempY += 12; 
    }
  }
}

// O Default Argument foi removido daqui para evitar erros de C++ (fica apenas no .h)
void avancarLinha(uint16_t corRestaurar, int tamanhoFonteRestaurar) {
  cursorX = margemEsquerda;
  cursorY += 12;
  novaLinhaCache();
  
  int limiteTextoY = tft.height() - 4;
  
  if (cursorY + 8 > limiteTextoY) {
    indiceLinhaInicioPagina++;
    cursorY -= 12; 
    restaurarPaginaAtual();
    tft.setTextColor(corRestaurar);
    tft.setTextSize(tamanhoFonteRestaurar);
  }
}

String limparAcentos(const String& textoOriginal) {
  String texto = textoOriginal;
  texto.replace("á","a"); texto.replace("à","a"); texto.replace("ã","a"); texto.replace("â","a");
  texto.replace("é","e"); texto.replace("ê","e"); texto.replace("í","i");
  texto.replace("ó","o"); texto.replace("õ","o"); texto.replace("ô","o"); texto.replace("ú","u");
  texto.replace("ç","c"); 
  texto.replace("Á","A"); texto.replace("Ã","A"); texto.replace("É","E");
  texto.replace("Í","I"); texto.replace("Ó","O"); texto.replace("Õ","O"); texto.replace("Ú","U");
  texto.replace("Ç","C"); 
  return texto;
}

void processarEntradaTerminal() {
  if (!Serial.available()) return;
  
  String textoLido = Serial.readString(); 
  textoLido.trim(); 
  
  if (textoLido.length() == 0) return;

  if (telaInicialCreditos) {
    telaInicialCreditos = false;
    tft.fillScreen(ST77XX_BLACK);
    desenharCabecalho();
    cursorX = margemEsquerda;
    cursorY = inicioTextoY;
    limparCache();
    escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
  }

  if (scrollLinha == 0) {
    tft.fillRect(cursorX, cursorY, 6, 8, ST77XX_BLACK);
  }
  
  textoLido = limparAcentos(textoLido);
  String textoUpper = textoLido;
  textoUpper.toUpperCase();

  if (textoUpper == "W") {
    if (opcaoSelecionada > 0) opcaoSelecionada--;
    else opcaoSelecionada = menuAtual.size() - 1;
    marqueeOffset = 0; 
    desenharMenu();
    return; 
  }
  
  if (textoUpper == "S") {
    if (opcaoSelecionada < (int)menuAtual.size() - 1) opcaoSelecionada++;
    else opcaoSelecionada = 0; 
    marqueeOffset = 0; 
    desenharMenu();
    return;
  }
  
  if (textoUpper == "E") {
    if (menuAtual.empty()) return;
    acaoExplorador(menuAtual[opcaoSelecionada]);
    return;
  }

  if (textoUpper == "CALC" && !modoCalc) {
    modoCalc = true; 
    if (cursorX > margemEsquerda) avancarLinha();
    escreverEfeitoDigitacao("CALCULADORA ATIVADA. Digite EXIT para sair.", 1, ST77XX_GREEN);
    avancarLinha();
    escreverEfeitoDigitacao("calc> ", 1, ST77XX_CYAN);
    esperandoTexto = true;
    return; 
  }

  if (modoCalc) {
    processarEntradaCalc(textoLido);
    return; 
  }
  
  if (scrollLinha > 0 && textoUpper != "UP" && textoUpper != "DOWN") {
    scrollLinha = 0; restaurarPaginaAtual();
  }

  escreverEfeitoDigitacao(textoLido, 1, ST77XX_GREEN);
  avancarLinha();

  if (textoUpper == "CLEAR") {
    tft.fillScreen(ST77XX_BLACK); 
    desenharCabecalho(); 
    cursorX = margemEsquerda; 
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
  else if (textoUpper == "BOOTLOGO") {
    exibirBootlogoComando();
  }
  else if (textoUpper == "HELP") {
    String mensagensAjuda[] = {
      "COMANDOS DISPONIVEIS:", 
      "BOOTLOGO (Exibe a logo)",
      "CALC (Calculadora)",
      "CLEAR (Limpa a tela)",
      "HELP (Lista comandos)",
      "REBOOT (Reinicia sistema)",
      "SHUTDOWN (Desliga OS)",
      "UP / DOWN (Historico)"
    };
    int numMensagens = 8; 
    bool interrompido = false;

    for (int i = 0; i < numMensagens; i += 2) {
      tft.fillScreen(ST77XX_BLACK); 
      desenharCabecalho(); 
      cursorX = margemEsquerda; 
      cursorY = inicioTextoY;
      novaLinhaCache(); 
      indiceLinhaInicioPagina = idxLinhaCache; 
      
      for (int j = 0; j < 2; j++) {
        if (i + j < numMensagens) {
          if (Serial.available()) { interrompido = true; break; }
          escreverEfeitoDigitacao(mensagensAjuda[i + j], 1, ST77XX_GREEN);
          avancarLinha();
        }
      }
      if (interrompido) break;
      
      for (int p = 0; p < 3; p++) {
        unsigned long startWait = millis();
        while (millis() - startWait < 666) { if (Serial.available()) { interrompido = true; break; } delay(10); }
        if (interrompido) break;
        escreverEfeitoDigitacao(".", 1, ST77XX_GREEN);
        if (Serial.available()) { interrompido = true; break; }
      }
      if (interrompido) break;
    }
    
    if (!interrompido) {
      tft.fillScreen(ST77XX_BLACK); 
      desenharCabecalho(); 
      cursorX = margemEsquerda; 
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
    escreverEfeitoDigitacao("Erro: Comando nao reconhecido.", 1, ST77XX_GREEN);
  }

  if (modoComando && textoUpper != "UP" && textoUpper != "DOWN") {
    if (cursorX > margemEsquerda) {
       avancarLinha();
    }
    escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
  }
}