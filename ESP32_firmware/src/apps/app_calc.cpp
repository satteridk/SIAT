#include "../../include/apps/app_calc.h"

void processarEntradaCalc() {
  if (!Serial.available()) return;
  String textoLido = Serial.readString();
  textoLido.trim();
  if (textoLido.length() == 0) return;

  String textoUpper = textoLido;
  textoUpper.toUpperCase();

  if (textoUpper == "EXIT") {
    estadoAtual = TERMINAL_CMD; // O FSM devolverá o sistema ao Terminal imediatamente 
    if (cursorX > margemEsquerda) avancarLinha();
    escreverEfeitoDigitacao("CALCULADORA DESATIVADA.", 1, ST77XX_GREEN);
    
    if (cursorX > margemEsquerda) avancarLinha();
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
  avancarLinha();

  if (parser.compile(textoLido.c_str())) {
    double resultadoCalc = parser.eval();
    String resFinal = String(resultadoCalc, 4); 
    escreverEfeitoDigitacao("= " + resFinal, 1, ST77XX_GREEN);
  } else {
    escreverEfeitoDigitacao("Erro: Expressao invalida.", 1, ST77XX_RED);
  }

  avancarLinha();
  escreverEfeitoDigitacao("calc> ", 1, ST77XX_CYAN);
  esperandoTexto = true;
}