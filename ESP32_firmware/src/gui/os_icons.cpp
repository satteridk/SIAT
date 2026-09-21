#include "../../include/gui/os_icons.h"

// ==========================================
// Todos os icones ocupam a caixa logica 11x9 comecando em (x, y+1).
// Centro vertical padronizado em y+5 -> alinhamento perfeito entre si.
// ==========================================
void desenharIcone(int x, int y, int tipo) {
  if (tipo == ICONE_PASTA_FECHADA) {
    tft.fillRect(x, y + 2, 10, 7, COR_PASTA);
    tft.fillRect(x, y, 5, 2, COR_PASTA);
  }
  else if (tipo == ICONE_PASTA_ABERTA) {
    tft.fillRect(x, y + 2, 10, 7, COR_PASTA);
    tft.fillRect(x, y, 5, 2, COR_PASTA);
    tft.fillRect(x + 1, y + 4, 10, 6, COR_PASTA_CLARA);
  }
  else if (tipo == ICONE_CMD) {
    // Moldura centralizada: y+1 .. y+9 (centro exato em y+5)
    tft.drawRect(x, y + 1, 11, 9, ST77XX_WHITE);
    // Chevron ">" de 2px preenchendo o vao interno, sem underline
    tft.drawLine(x + 3, y + 3, x + 7, y + 5, ST77XX_WHITE);
    tft.drawLine(x + 7, y + 5, x + 3, y + 7, ST77XX_WHITE);
    tft.drawLine(x + 3, y + 4, x + 6, y + 5, ST77XX_WHITE);
    tft.drawLine(x + 6, y + 5, x + 3, y + 6, ST77XX_WHITE);
  }
  else if (tipo == ICONE_SNAKE) {
    // Corpo serpenteado em "S"
    tft.drawFastHLine(x + 1, y + 8, 6, COR_SNAKE);
    tft.drawFastVLine(x + 1, y + 6, 3, COR_SNAKE);
    tft.drawFastHLine(x + 1, y + 5, 6, COR_SNAKE);
    tft.drawFastVLine(x + 6, y + 3, 3, COR_SNAKE);
    tft.drawFastHLine(x + 4, y + 2, 3, COR_SNAKE);
    // Cabeca, olho e lingua
    tft.fillRect(x + 2, y + 1, 3, 3, COR_SNAKE);
    tft.drawPixel(x + 3, y + 2, ST77XX_BLACK);
    tft.drawPixel(x + 1, y + 2, ST77XX_RED);
    // Comida
    tft.fillCircle(x + 9, y + 7, 1, ST77XX_RED);
  }
  else if (tipo == ICONE_VOLTAR) {
    tft.fillTriangle(x + 1, y + 5, x + 5, y + 2, x + 5, y + 8, ST77XX_WHITE);
    tft.drawFastHLine(x + 5, y + 4, 5, ST77XX_WHITE);
    tft.drawFastHLine(x + 5, y + 6, 5, ST77XX_WHITE);
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
    tft.drawFastVLine(x + 3, y + 3, 5, ST77XX_WHITE);
    tft.drawFastVLine(x + 7, y + 2, 5, ST77XX_WHITE);
    tft.drawFastHLine(x + 4, y + 2, 4, ST77XX_WHITE);
    tft.fillCircle(x + 2, y + 7, 2, ST77XX_WHITE);
    tft.fillCircle(x + 6, y + 6, 2, ST77XX_WHITE);
  }
  else if (tipo == ICONE_TEXTO) {
    tft.fillRect(x + 2, y + 1, 6, 8, ST77XX_WHITE);
    tft.drawFastHLine(x + 3, y + 3, 4, ST77XX_BLACK);
    tft.drawFastHLine(x + 3, y + 5, 4, ST77XX_BLACK);
    tft.drawFastHLine(x + 3, y + 7, 2, ST77XX_BLACK);
  }
}

// Classificacao de icone por extensao (usada pelo explorador)
int iconePorNome(const String& nome, bool isDir) {
  if (isDir) return ICONE_PASTA_FECHADA;

  String u = nome;
  u.toUpperCase();

  if (u.endsWith(".MP3") || u.endsWith(".WAV") || u.endsWith(".OGG") || u.endsWith(".FLAC"))
    return ICONE_MUSICA;
  if (u.endsWith(".TXT") || u.endsWith(".DAT") || u.endsWith(".INI") ||
      u.endsWith(".CONFIG") || u.endsWith(".CSV"))
    return ICONE_TEXTO;

  return ICONE_ARQUIVO;
}