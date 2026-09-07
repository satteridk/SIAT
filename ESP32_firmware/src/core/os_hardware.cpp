#include "../../include/os_globals.h"

void verificarBotaoFisico() {
  bool leitura = digitalRead(PINO_BOTAO);
  if (leitura == LOW && estadoAnteriorBotao == HIGH && millis() - ultimoDebounce > 250) {
    ultimoDebounce = millis();
    if (!popupAberto) {
      popupAberto = true; 
      tempoAberturaPopup = millis(); 
      segundosRestantes = 3;
      desenharPopup(segundosRestantes);
    } else {
      desligarSistema(); 
    }
  }
  estadoAnteriorBotao = leitura;

  if (popupAberto) {
    int t = (millis() - tempoAberturaPopup) / 1000;
    if (3 - t != segundosRestantes) {
      segundosRestantes = 3 - t;
      if (segundosRestantes >= 0) desenharPopup(segundosRestantes);
      else fecharPopup();
    }
  }
}

void desligarSistema() {
  tft.fillScreen(ST77XX_BLACK); tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
  String l1 = "------------------------------------";
  String l2 = "GOODBYE!";
  String l3 = "------------------------------------";
  
  int yC = (tft.height() - 30) / 2; 
  
  tft.setCursor((tft.width() - (l1.length() * 6)) / 2, yC); tft.print(l1);
  tft.setCursor((tft.width() - (l2.length() * 6)) / 2, yC + 10); tft.print(l2);
  tft.setCursor((tft.width() - (l3.length() * 6)) / 2, yC + 20); tft.print(l3); 
  
  delay(1500); 
  
  tft.fillScreen(ST77XX_BLACK);  
  digitalWrite(PINO_TELA, HIGH); 
  
  while(digitalRead(PINO_BOTAO) == LOW) delay(10);
  
  rtc_gpio_pullup_en(GPIO_NUM_13);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);
  esp_deep_sleep_start();
}