#include "../../include/apps/app_explorer.h"

FileNode sistemaArquivos;

// --- Estado interno do navegador (encapsulado) ---
static FileNode* pilhaDir[EXPLORER_MAX_NIVEL];
static int  profundidade   = 0;
static int  selExplorer    = 0;
static int  topoExplorer   = 0;
static bool arvoreCarregada = false;
static String statusRodape = "";

// ==========================================
// UTILITARIOS
// ==========================================
static String limparCaracteresEspeciais(const String& texto) {
  String t = texto;
  t.replace("á","a"); t.replace("à","a"); t.replace("â","a"); t.replace("ã","a");
  t.replace("é","e"); t.replace("è","e"); t.replace("ê","e");
  t.replace("í","i"); t.replace("ì","i"); t.replace("î","i");
  t.replace("ó","o"); t.replace("ò","o"); t.replace("ô","o"); t.replace("õ","o");
  t.replace("ú","u"); t.replace("ù","u"); t.replace("û","u");
  t.replace("ç","c");
  t.replace("Á","A"); t.replace("À","A"); t.replace("Â","A"); t.replace("Ã","A");
  t.replace("É","E"); t.replace("È","E"); t.replace("Ê","E");
  t.replace("Í","I"); t.replace("Ì","I"); t.replace("Î","I");
  t.replace("Ó","O"); t.replace("Ò","O"); t.replace("Ô","O"); t.replace("Õ","O");
  t.replace("Ú","U"); t.replace("Ù","U"); t.replace("Û","U");
  t.replace("Ç","C");

  String limpo = "";
  for (unsigned int i = 0; i < t.length(); i++) {
    unsigned char c = t[i];
    if (c >= 32 && c <= 126) limpo += (char)c;
    else if (c > 127) limpo += '_';
  }
  return limpo;
}

static String formatarTamanhoBytes(uint32_t bytes) {
  double b = (double)bytes;
  if (b < 1024.0)       return String((uint32_t)b) + "B";
  if (b < 1048576.0)    return String(b / 1024.0, 1) + "K";
  if (b < 1073741824.0) return String(b / 1048576.0, 1) + "M";
  return String(b / 1073741824.0, 1) + "G";
}

// Resolve o pai por INDICE em vez de ponteiro salvo.
// Imune a realocacao do std::vector durante o push_back (corrige corrupcao).
static FileNode* resolverPai(int nivel, int* idxNivel) {
  FileNode* no = &sistemaArquivos;
  for (int i = 0; i < nivel; i++) {
    if (idxNivel[i] < 0 || idxNivel[i] >= (int)no->children.size()) return no;
    no = &no->children[idxNivel[i]];
  }
  return no;
}

static int linhasVisiveis() {
  int v = (tft.height() - 20 - 18) / ALTURA_LINHA;
  return v > 1 ? v : 1;
}

static String caminhoAtual() {
  String c = "/";
  for (int i = 1; i <= profundidade; i++) {
    c += pilhaDir[i]->name;
    c += "/";
  }
  return c;
}

// ==========================================
// PONTE UART COM O PICO
// ==========================================
static void carregarArvoreSD() {
  sistemaArquivos.children.clear();
  Serial.println("Explorador: solicitando dados ao PICO...");

  uartOcupada = true;                 // Blinda a leitura contra a task do Core 0
  while (Serial2.available()) Serial2.read();
  Serial2.println("DADOS_SD");

  int idxNivel[EXPLORER_MAX_NIVEL];
  for (int i = 0; i < EXPLORER_MAX_NIVEL; i++) idxNivel[i] = -1;

  unsigned long timeout = millis();
  bool recebendo = false;
  char bufferLinha[256];

  while (millis() - timeout < 4000) {
    if (!Serial2.available()) continue;

    size_t len = Serial2.readBytesUntil('\n', bufferLinha, sizeof(bufferLinha) - 1);
    bufferLinha[len] = '\0';
    if (len > 0 && bufferLinha[len - 1] == '\r') bufferLinha[len - 1] = '\0';

    if (strcmp(bufferLinha, "INICIO_SD") == 0) { recebendo = true; continue; }
    if (strcmp(bufferLinha, "FIM_SD") == 0) break;
    if (!recebendo || len == 0) continue;
    if (bufferLinha[0] != 'D' && bufferLinha[0] != 'F') continue;

    // Parsing in-place via ponteiros (zero alocacao de String no laco)
    char* tipoStr    = strtok(bufferLinha, "|");
    char* nivelStr   = strtok(NULL, "|");
    char* nomeBruto  = strtok(NULL, "|");
    char* tamanhoStr = strtok(NULL, "|");
    if (!tipoStr || !nivelStr || !nomeBruto) continue;

    int nivel = atoi(nivelStr);
    if (nivel < 0 || nivel >= EXPLORER_MAX_NIVEL) continue;

    FileNode* pai = resolverPai(nivel, idxNivel);

    FileNode no;
    no.name  = limparCaracteresEspeciais(String(nomeBruto));
    no.isDir = (tipoStr[0] == 'D');
    no.size  = (tamanhoStr && !no.isDir) ? (uint32_t)strtoul(tamanhoStr, NULL, 10) : 0;
    no.path  = (pai->path.endsWith("/") ? pai->path : pai->path + "/") + no.name;

    pai->children.push_back(no);
    if (no.isDir) idxNivel[nivel] = (int)pai->children.size() - 1;
  }

  uartOcupada = false;
  arvoreCarregada = true;
  Serial.println("Explorador: arvore construida em RAM.");
}

// ==========================================
// RENDERIZACAO FULLSCREEN (ESTILO uLaunchELF)
// ==========================================
void desenharExplorador() {
  int w = tft.width();
  int h = tft.height();

  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, w, h, ST77XX_WHITE);
  tft.drawFastHLine(1, 15, w - 2, ST77XX_WHITE);
  tft.setTextSize(1);

  // Cabecalho: caminho atual
  String cam = caminhoAtual();
  int maxc = (w - 12) / LARGURA_CHAR;
  if ((int)cam.length() > maxc) cam = "..." + cam.substring(cam.length() - (maxc - 3));
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 4);
  tft.print(cam);

  FileNode* dir = pilhaDir[profundidade];
  int total = (int)dir->children.size() + 1;     // +1 = entrada ".."
  int vis = linhasVisiveis();

  if (selExplorer < topoExplorer) topoExplorer = selExplorer;
  if (selExplorer >= topoExplorer + vis) topoExplorer = selExplorer - vis + 1;
  if (topoExplorer < 0) topoExplorer = 0;

  int y = 20;
  for (int i = topoExplorer; i < total && i < topoExplorer + vis; i++) {
    bool sel = (i == selExplorer);
    if (sel) tft.fillRect(2, y - 2, w - 4, ALTURA_LINHA, COR_FUNDO_SELECIONADO);
    tft.setTextColor(sel ? ST77XX_WHITE : COR_CINZA);

    if (i == 0) {
      // Diretorio especial no topo absoluto
      desenharIcone(6, y - 1, ICONE_VOLTAR);
      tft.setCursor(22, y);
      tft.print("..");
      tft.setCursor(w - 6 - (6 * LARGURA_CHAR), y);
      tft.print(profundidade > 0 ? "<VOLTA>" : "<SAIR> ");
    } else {
      FileNode& n = dir->children[i - 1];
      desenharIcone(6, y - 1, iconePorNome(n.name, n.isDir));

      String info = n.isDir ? String("<DIR>") : formatarTamanhoBytes(n.size);
      int colInfo = w - 6 - (info.length() * LARGURA_CHAR);
      int maxNome = (colInfo - 24) / LARGURA_CHAR;

      String nome = n.name;
      if (maxNome > 1 && (int)nome.length() > maxNome) nome = nome.substring(0, maxNome - 1) + ">";

      tft.setCursor(22, y);
      tft.print(nome);
      tft.setCursor(colInfo, y);
      tft.print(info);
    }
    y += ALTURA_LINHA;
  }

  // Rodape
  tft.drawFastHLine(1, h - 14, w - 2, ST77XX_WHITE);
  tft.setTextColor(COR_CINZA);
  tft.setCursor(5, h - 11);
  tft.print(statusRodape.length() ? statusRodape : String("W/S  E:Abrir  EXIT:Sair"));

  String cont = String(selExplorer + 1) + "/" + String(total);
  tft.setCursor(w - 5 - (cont.length() * LARGURA_CHAR), h - 11);
  tft.print(cont);
}

// ==========================================
// CICLO DE VIDA
// ==========================================
void iniciarExplorador() {
  estadoAtual = APP_EXPLORADOR;
  statusRodape = "";

  if (!arvoreCarregada) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, (tft.height() / 2) - 4);
    tft.print("Lendo MicroSD do PICO...");
    carregarArvoreSD();
  }

  profundidade = 0;
  pilhaDir[0] = &sistemaArquivos;
  selExplorer = 0;
  topoExplorer = 0;
  desenharExplorador();
}

void sairExplorador() {
  statusRodape = "";
  restaurarTerminal();
}

// ==========================================
// ENTRADA
// ==========================================
void processarEntradaExplorador() {
  String entrada;
  if (!lerLinhaSerial(entrada)) return;

  String cmd = entrada;
  cmd.toUpperCase();

  FileNode* dir = pilhaDir[profundidade];
  int total = (int)dir->children.size() + 1;

  if (cmd == "EXIT") { sairExplorador(); return; }

  if (cmd == "R") {                 // Recarrega a arvore do cartao
    arvoreCarregada = false;
    iniciarExplorador();
    return;
  }

  if (cmd == "W") {
    selExplorer = (selExplorer > 0) ? selExplorer - 1 : total - 1;
    statusRodape = "";
    desenharExplorador();
    return;
  }

  if (cmd == "S") {
    selExplorer = (selExplorer < total - 1) ? selExplorer + 1 : 0;
    statusRodape = "";
    desenharExplorador();
    return;
  }

  if (cmd == "E") {
    // Entrada ".." no topo absoluto: sobe um nivel ou sai na raiz
    if (selExplorer == 0) {
      if (profundidade > 0) {
        profundidade--;
        selExplorer = 0;
        topoExplorer = 0;
        statusRodape = "";
        desenharExplorador();
      } else {
        sairExplorador();
      }
      return;
    }

    FileNode& n = dir->children[selExplorer - 1];

    if (n.isDir) {
      if (profundidade + 1 < EXPLORER_MAX_NIVEL) {
        pilhaDir[profundidade + 1] = &n;
        profundidade++;
        selExplorer = 0;
        topoExplorer = 0;
        statusRodape = "";
        desenharExplorador();
      }
    } else {
      Serial.println("[ARQUIVO] " + n.path + "  (" + formatarTamanhoBytes(n.size) + ")");
      statusRodape = n.name + " " + formatarTamanhoBytes(n.size);
      desenharExplorador();
    }
    return;
  }
}