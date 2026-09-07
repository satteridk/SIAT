#include "../../include/core/sd_manager.h"

// Função auxiliar para exibir a árvore no Serial do Pico
void imprimirCacheSerial() {
  Serial.println("\n--- CONTEUDO DO CARTAO MICROSD ---");
  int pos = 0;
  while (pos < (int)cacheSD.length()) {
    int nextPos = cacheSD.indexOf('\n', pos);
    if (nextPos == -1) break;
    String linha = cacheSD.substring(pos, nextPos);
    linha.trim();
    
    if (linha.startsWith("D|") || linha.startsWith("F|")) {
      int pos1 = linha.indexOf('|');
      int pos2 = linha.indexOf('|', pos1 + 1);
      int pos3 = linha.indexOf('|', pos2 + 1);
      
      String tipo = linha.substring(0, pos1);
      int nivel = linha.substring(pos1 + 1, pos2).toInt();
      String nome = linha.substring(pos2 + 1, (pos3 == -1) ? linha.length() : pos3);
      String tamanho = (pos3 != -1) ? linha.substring(pos3 + 1) : "";
      
      for(int i = 0; i < nivel; i++) Serial.print("  "); 
      if(tipo == "D") {
        Serial.print("[PASTA]   /"); Serial.println(nome);
      } else {
        Serial.print("[ARQUIVO] "); Serial.print(nome);
        Serial.print(" \t("); Serial.print(tamanho); Serial.println(" bytes)");
      }
    }
    pos = nextPos + 1;
  }
  Serial.println("----------------------------------");
}

void construirCache(File dir, int nivel) {
  if(nivel == 0) cacheSD = ""; 
  
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;

    if (entry.isDirectory()) {
      cacheSD += "D|" + String(nivel) + "|" + entry.name() + "\n";
      construirCache(entry, nivel + 1); 
    } else {
      cacheSD += "F|" + String(nivel) + "|" + entry.name() + "|" + entry.size() + "\n";
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
        
        imprimirCacheSerial(); // <-- Exibe no boot
        
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
    cacheSD = ""; 
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
            
            imprimirCacheSerial(); // <-- Exibe ao inserir o cartão
            
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