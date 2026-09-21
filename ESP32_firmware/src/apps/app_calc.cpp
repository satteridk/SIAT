#include "../../include/apps/app_calc.h"

void processarEntradaCalc() {
  if (!Serial.available()) return;
  String textoLido = Serial.readStringUntil('\n');
  textoLido.trim();
  if (textoLido.length() == 0) return;

  String textoUpper = textoLido;
  textoUpper.toUpperCase();

  if (textoUpper == "EXIT") {
    estadoAtual = APP_TERMINAL; 
    int startX = (tft.width() / 2) + 6;
    if (cursorX > startX) avancarLinha(ST77XX_GREEN, 1);
    escreverEfeitoDigitacao("CALC DESATIVADA.", 1, ST77XX_GREEN);
    
    avancarLinha(ST77XX_WHITE, 1);
    escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
    
    esperandoTexto = true;
    return;
  }
  if (textoUpper == "UP") {
    int maxScroll = idxLinhaCache;
    if (scrollLinha < maxScroll) {
      scrollLinha += 3;
      if (scrollLinha > maxScroll) scrollLinha = maxScroll;
      renderizarScroll();
    }
    return;
  }
  if (textoUpper == "DOWN") {
    if (scrollLinha > 0) {
      scrollLinha -= 3;
      if (scrollLinha <= 0) {
        scrollLinha = 0;
        restaurarPaginaAtual();
      } else {
        renderizarScroll();
      }
    }
    return;
  }
  if (scrollLinha > 0 && textoUpper != "UP" && textoUpper != "DOWN") {
     scrollLinha = 0;
     restaurarPaginaAtual();
   }

  escreverEfeitoDigitacao(textoLido, 1, ST77XX_WHITE);
  avancarLinha(ST77XX_GREEN, 1);

  if (parser.compile(textoLido.c_str())) {
    double resultadoCalc = parser.eval();
    String resFinal = String(resultadoCalc, 4); 
    escreverEfeitoDigitacao("= " + resFinal, 1, ST77XX_GREEN);
  } else {
    escreverEfeitoDigitacao("Erro: Expressao invalida.", 1, ST77XX_RED);
  }

  avancarLinha(ST77XX_CYAN, 1);
  escreverEfeitoDigitacao("calc> ", 1, ST77XX_CYAN);
  esperandoTexto = true;
}