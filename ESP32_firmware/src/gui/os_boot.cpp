#include "../../include/os_globals.h"

// Rotina unica de pintura da logo (evita duplicacao entre boot e comando)
static void pintarLogo() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  String logoTexto = "S.I.A.T";
  String logoSimbolo = "(c)";
  int larguraCharSize3 = 18;
  int alturaCharSize3  = 24;
  int alturaCharSize1  = 8;
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

  String footer1 = "satter's S.I.A.T";
  String footer2 = "terminal v2.6";
  int f1W = footer1.length() * larguraCharSize1;
  int f2W = footer2.length() * larguraCharSize1;
  int f2Y = tft.height() - alturaCharSize1 - 5;
  int f1Y = f2Y - 12;

  tft.setCursor((tft.width() - f1W) / 2, f1Y);
  tft.print(footer1);
  tft.setCursor((tft.width() - f2W) / 2, f2Y);
  tft.print(footer2);
}

void exibirBootlogoComando() {
  pintarLogo();

  while (!Serial.available()) delay(10);
  while (Serial.available()) { Serial.read(); delay(2); }

  if (scrollLinha > 0) renderizarScroll();
  else restaurarPaginaAtual();
}

void animacaoDeBoot() {
  pintarLogo();
  delay(2500);

  tft.fillScreen(ST77XX_BLACK);
  desenharCabecalho();

  cursorX = margemEsquerda;
  cursorY = inicioTextoY + ALTURA_LINHA;
  limparCache();

  tft.setTextColor(COR_CINZA);
  tft.setTextSize(1);
  tft.setCursor(margemEsquerda, cursorY);
  tft.print("Linguagem: PT-BR");
  cursorY += ALTURA_LINHA;
  tft.setCursor(margemEsquerda, cursorY);
  tft.print("Feito por: satter");
  cursorY += ALTURA_LINHA;
  tft.setCursor(margemEsquerda, cursorY);
  tft.print("Sistema Iniciado.");

  cursorX = margemEsquerda;
  cursorY = inicioTextoY;
  escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
}