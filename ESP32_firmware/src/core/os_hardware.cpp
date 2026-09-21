#include "../../include/os_globals.h"
#include "driver/uart.h"

void verificarBotaoFisico() {
  bool leitura = digitalRead(PINO_BOTAO);
  if (leitura == LOW && estadoAnteriorBotao == HIGH && millis() - ultimoDebounce > 250) {
    ultimoDebounce = millis();
    if (estadoAtual != POPUP_DESLIGAR) {
      estadoAnterior = estadoAtual;
      estadoAtual = POPUP_DESLIGAR;
      tempoAberturaPopup = millis();
      segundosRestantes = 3;
      desenharPopup(segundosRestantes);
    } else {
      desligarSistema(); 
    }
  }
  estadoAnteriorBotao = leitura;
  
  if (estadoAtual == POPUP_DESLIGAR) {
    int t = (millis() - tempoAberturaPopup) / 1000;
    if (3 - t != segundosRestantes) {
      segundosRestantes = 3 - t;
      if (segundosRestantes >= 0) desenharPopup(segundosRestantes);
      else fecharPopup();
    }
  }
}

void desligarSistema() {
  tft.fillScreen(ST77XX_BLACK); tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE);
  String l2 = "GOODBYE!";
  int yC = (tft.height() - 16) / 2; 
  tft.setCursor((tft.width() - (l2.length() * 12)) / 2, yC); tft.print(l2);
  
  delay(1500); 
  tft.fillScreen(ST77XX_BLACK); 
  digitalWrite(PINO_TELA, LOW); 
  
  while(digitalRead(PINO_BOTAO) == LOW) delay(10);
  
  rtc_gpio_pullup_en(GPIO_NUM_13);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);

  uart_set_wakeup_threshold(UART_NUM_0, 3); 
  esp_sleep_enable_uart_wakeup(UART_NUM_0);

  esp_deep_sleep_start();
}