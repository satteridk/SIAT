#include "../../include/os_globals.h"
#include "../../include/apps/app_explorer.h"

// --- Estado proprio do console ---
int cursorX = margemEsquerda;
int cursorY = inicioTextoY;
unsigned long tempoUltimoBlink = 0;
bool cursorVisivel = false;
bool esperandoTexto = true;
bool telaInicialCreditos = true;

String cacheLinhas[MAX_LINHAS_CACHE];
uint16_t cacheCores[MAX_LINHAS_CACHE][MAX_CHARS_LINHA];
int idxLinhaCache = 0;
int indiceLinhaInicioPagina = 0;
int scrollLinha = 0;

// ==========================================
// ENTRADA SERIAL NAO BLOQUEANTE
// readStringUntil('\n') + timeout de 10ms = zero lag.
// ==========================================
bool lerLinhaSerial(String& destino) {
  if (!Serial.available()) return false;
  destino = Serial.readStringUntil('\n');
  destino.trim();
  return destino.length() > 0;
}

// ==========================================
// CACHE 2D
// ==========================================
void limparCache() {
  for (int i = 0; i < MAX_LINHAS_CACHE; i++) {
    cacheLinhas[i] = "";
    for (int j = 0; j < MAX_CHARS_LINHA; j++) cacheCores[i][j] = ST77XX_WHITE;
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
      for (int j = 0; j < MAX_CHARS_LINHA; j++) cacheCores[i][j] = cacheCores[i + 1][j];
    }
    cacheLinhas[MAX_LINHAS_CACHE - 1] = "";
    for (int j = 0; j < MAX_CHARS_LINHA; j++) cacheCores[MAX_LINHAS_CACHE - 1][j] = ST77XX_WHITE;
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
  int x = margemEsquerda;
  tft.setTextSize(1);
  for (size_t i = 0; i < cacheLinhas[indice].length(); i++) {
    tft.setTextColor(cacheCores[indice][i]);
    tft.setCursor(x, y);
    tft.print(cacheLinhas[indice][i]);
    x += LARGURA_CHAR;
  }
}

// ==========================================
// RENDERIZACAO
// ==========================================
void restaurarPaginaAtual() {
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho();

  int maxLinhasVisiveis = (tft.height() - inicioTextoY - 20) / ALTURA_LINHA;
  int linhaInicioRender = idxLinhaCache - maxLinhasVisiveis + 1;
  if (linhaInicioRender < indiceLinhaInicioPagina) linhaInicioRender = indiceLinhaInicioPagina;
  if (linhaInicioRender < 0) linhaInicioRender = 0;

  int y = inicioTextoY;
  for (int i = linhaInicioRender; i <= idxLinhaCache; i++) {
    desenharLinhaCache(i, y);
    y += ALTURA_LINHA;
  }

  cursorX = margemEsquerda;
  if (cacheLinhas[idxLinhaCache].length() > 0) {
    cursorX = margemEsquerda + (cacheLinhas[idxLinhaCache].length() * LARGURA_CHAR);
  }
  cursorY = y - ALTURA_LINHA;
}

void renderizarScroll() {
  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho();

  int maxLinhasVisiveis = (tft.height() - inicioTextoY - 20) / ALTURA_LINHA;

  int linhaFimScroll = idxLinhaCache - scrollLinha;
  if (linhaFimScroll < 0) linhaFimScroll = 0;

  int linhaInicioScroll = linhaFimScroll - maxLinhasVisiveis + 1;
  if (linhaInicioScroll < 0) linhaInicioScroll = 0;

  int y = inicioTextoY;
  for (int i = linhaInicioScroll; i <= linhaFimScroll; i++) {
    desenharLinhaCache(i, y);
    y += ALTURA_LINHA;
  }

  tft.fillRect((tft.width() * 0.6) - 10, inicioTextoY, 4, tft.height() - inicioTextoY - 20, COR_CINZA);

  int totalLinhas = idxLinhaCache + 1;
  if (totalLinhas > maxLinhasVisiveis) {
    float proporcao = (float)maxLinhasVisiveis / totalLinhas;
    int alturaBarra = proporcao * (tft.height() - inicioTextoY - 20);
    if (alturaBarra < 10) alturaBarra = 10;

    float progresso = 1.0 - ((float)scrollLinha / (totalLinhas - maxLinhasVisiveis));
    int maxYBarra = (tft.height() - 20) - alturaBarra;
    int yBarra = inicioTextoY + (progresso * (maxYBarra - inicioTextoY));

    tft.fillRect((tft.width() * 0.6) - 11, yBarra, 6, alturaBarra, ST77XX_WHITE);
  }
}

void avancarLinha(uint16_t corRestaurar, int tamanhoFonteRestaurar) {
  cursorY += (8 * tamanhoFonteRestaurar) + 4;
  cursorX = margemEsquerda;
  novaLinhaCache();

  if (cursorY > tft.height() - 20) {
    restaurarPaginaAtual();
    cursorY = tft.height() - 24;
  }
  tft.setTextColor(corRestaurar);
}

// Motor de digitacao (pertence ao terminal, nao a GUI)
void escreverEfeitoDigitacao(const String& texto, int tamanhoFonte, uint16_t cor) {
  tft.setTextSize(tamanhoFonte);
  tft.setTextColor(cor);

  int larguraLetra = LARGURA_CHAR * tamanhoFonte;
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
      } else {
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

// Retorno padronizado ao console (usado por qualquer app fullscreen)
void restaurarTerminal() {
  estadoAtual = TERMINAL_CMD;
  if (scrollLinha > 0) renderizarScroll();
  else restaurarPaginaAtual();

  if (cursorX > margemEsquerda) avancarLinha();
  escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
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

// ==========================================
// PARSER MESTRE DE COMANDOS
// ==========================================
void processarEntradaTerminal() {
  String textoLido;
  if (!lerLinhaSerial(textoLido)) return;

  if (telaInicialCreditos) {
    telaInicialCreditos = false;
    tft.fillScreen(ST77XX_BLACK);
    desenharCabecalho();
    cursorX = margemEsquerda;
    cursorY = inicioTextoY;
    limparCache();
    escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
  }

  if (scrollLinha == 0) tft.fillRect(cursorX, cursorY, LARGURA_CHAR, 8, ST77XX_BLACK);

  textoLido = limparAcentos(textoLido);
  String textoUpper = textoLido;
  textoUpper.toUpperCase();

  // --- Navegacao do painel lateral ---
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
    executarItemMenu(menuAtual[opcaoSelecionada]);
    return;
  }

  // --- Transicao de estado: calculadora ---
  if (textoUpper == "CALC" && estadoAtual != APP_CALCULADORA) {
    estadoAtual = APP_CALCULADORA;
    if (cursorX > margemEsquerda) avancarLinha();
    escreverEfeitoDigitacao("CALCULADORA ATIVADA. Digite EXIT para sair.", 1, ST77XX_GREEN);
    avancarLinha();
    escreverEfeitoDigitacao("calc> ", 1, ST77XX_CYAN);
    esperandoTexto = true;
    return;
  }

  if (scrollLinha > 0 && textoUpper != "UP" && textoUpper != "DOWN") {
    scrollLinha = 0;
    restaurarPaginaAtual();
  }

  escreverEfeitoDigitacao(textoLido, 1, ST77XX_GREEN);
  avancarLinha();

  // --- Comandos de sistema ---
  if (textoUpper == "CLEAR") {
    tft.fillScreen(ST77XX_BLACK);
    desenharCabecalho();
    cursorX = margemEsquerda;
    cursorY = inicioTextoY;
    limparCache();
  }
  else if (textoUpper == "EXPLORER" || textoUpper == "EXPLORADOR") {
    iniciarExplorador();
    return;
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
      if (scrollLinha <= 0) { scrollLinha = 0; restaurarPaginaAtual(); }
      else renderizarScroll();
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
      "EXPLORER (Arquivos do SD)",
      "HELP (Lista comandos)",
      "REBOOT (Reinicia sistema)",
      "SHUTDOWN (Desliga OS)",
      "UP / DOWN (Historico)"
    };
    int numMensagens = 9;
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

  if (textoUpper != "UP" && textoUpper != "DOWN") {
    if (cursorX > margemEsquerda) avancarLinha();
    escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
  }
}