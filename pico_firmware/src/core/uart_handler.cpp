#include "../../include/core/uart_handler.h"

void processarComandosUART() {
  if (Serial2.available()) {
    String recebido = Serial2.readStringUntil('\n');
    recebido.trim();
    if (recebido.length() > 0) {
      if (recebido == "DADOS_SD") {
         Serial.println("Solicitacao de DADOS_SD recebida. Enviando cache...");
         Serial2.println("INICIO_SD");
         Serial2.print(cacheSD);
         Serial2.println("FIM_SD");
      } else {
         Serial.println("ESP32 MANDOU: " + recebido);
      }
    }
  }
}