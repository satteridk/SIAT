#include "../../include/core/sd_manager.h"

void imprimirCacheSerial() {
  Serial.println("\n--- CACHE GERADO DO MICROSD ---");
  Serial.println(cacheSD);
  Serial.println("-------------------------------");
}

void construirCache(File dir, int nivel) {
  if (nivel == 0) { 
    cacheSD[0] = '\0'; 
    cacheIndex = 0; 
  } 
  
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;

    char linha[256];
    if (entry.isDirectory()) {
      snprintf(linha, sizeof(linha), "D|%d|%s\n", nivel, entry.name());
      if (cacheIndex + strlen(linha) < sizeof(cacheSD)) {
        strcpy(&cacheSD[cacheIndex], linha);
        cacheIndex += strlen(linha);
      }
      construirCache(entry, nivel + 1); 
    } else {
      snprintf(linha, sizeof(linha), "F|%d|%s|%u\n", nivel, entry.name(), (unsigned int)entry.size());
      if (cacheIndex + strlen(linha) < sizeof(cacheSD)) {
        strcpy(&cacheSD[cacheIndex], linha);
        cacheIndex += strlen(linha);
      }
    }
    entry.close();
  }
}

void inicializarSD() {
  if (digitalRead(PINO_DETECT) == LOW) {
    Serial.println("\nCartao detectado no boot. Inicializando SD...");
    if (SD.begin(PINO_CS)) {
      cartao_conectado = true;
      mensagem_aguardando_exibida = false;
      
      File root = SD.open("/");
      if (root) {
        Serial.println("Lendo cartao e gerando cache...");
        construirCache(root, 0); 
        root.close();
        imprimirCacheSerial();
        Serial.println("(aguardando ESP32...)");
      }
    } else {
      Serial.println("[ERRO] Falha na comunicacao SPI com o cartao.");
    }
  }
}

void verificarStatusSD() {
  unsigned long tempo_atual = millis();
  bool cartao_inserido_fisicamente = (digitalRead(PINO_DETECT) == LOW);

  if (cartao_conectado && !cartao_inserido_fisicamente) {
    cartao_conectado = false;
    mensagem_aguardando_exibida = false;
    SD.end();
    cacheSD[0] = '\0'; cacheIndex = 0;
    Serial.println("\n[ALERTA] Cartao MicroSD foi removido fisicamente!");
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (!cartao_conectado) {
    if (tempo_atual % 2000 < 500) digitalWrite(LED_BUILTIN, HIGH);
    else digitalWrite(LED_BUILTIN, LOW);

    if (tempo_atual - tempo_anterior_serial >= 3000) {
      tempo_anterior_serial = tempo_atual;
      if (cartao_inserido_fisicamente) {
        if (SD.begin(PINO_CS)) {
          cartao_conectado = true;
          mensagem_aguardando_exibida = false;
          Serial.println("[SUCESSO] Cartao MicroSD reconhecido!");
          File root = SD.open("/");
          if (root) {
            Serial.println("Lendo cartao e gerando cache...");
            construirCache(root, 0);
            root.close();
            imprimirCacheSerial();
            Serial.println("(aguardando ESP32...)");
          }
        }
      } else {
        if (!mensagem_aguardando_exibida) {
          Serial.println("Aguardando insercao do cartao MicroSD...");
          mensagem_aguardando_exibida = true;
        }
      }
    }
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}