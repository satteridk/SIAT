#include "../../include/apps/app_explorer.h"
#include "../../include/apps/app_snake.h"

String limparCaracteresEspeciais(String texto) {
  String t = texto;
  t.replace("á", "a"); t.replace("à", "a"); t.replace("â", "a"); t.replace("ã", "a");
  t.replace("é", "e"); t.replace("è", "e"); t.replace("ê", "e");
  t.replace("í", "i"); t.replace("ì", "i"); t.replace("î", "i");
  t.replace("ó", "o"); t.replace("ò", "o"); t.replace("ô", "o"); t.replace("õ", "o");
  t.replace("ú", "u"); t.replace("ù", "u"); t.replace("û", "u");
  t.replace("ç", "c");
  t.replace("Á", "A"); t.replace("À", "A"); t.replace("Â", "A"); t.replace("Ã", "A");
  t.replace("É", "E"); t.replace("È", "E"); t.replace("Ê", "E");
  t.replace("Í", "I"); t.replace("Ì", "I"); t.replace("Î", "I");
  t.replace("Ó", "O"); t.replace("Ò", "O"); t.replace("Ô", "O"); t.replace("Õ", "O");
  t.replace("Ú", "U"); t.replace("Ù", "U"); t.replace("Û", "U");
  t.replace("Ç", "C");
  
  String limpo = "";
  for (int i = 0; i < t.length(); i++) {
    unsigned char c = t[i];
    if (c >= 32 && c <= 126) limpo += (char)c;
    else if (c > 127) limpo += '_'; 
  }
  return limpo;
}

String formatarTamanhoBytes(String tamanhoStr) {
  if (tamanhoStr.length() == 0) return "";
  double bytes = tamanhoStr.toDouble(); 
  if (bytes < 1024) return String((int)bytes) + " bytes";
  else if (bytes < 1048576) return String(bytes / 1024.0, 2) + " KB";
  else if (bytes < 1073741824) return String(bytes / 1048576.0, 2) + " MB";
  else return String(bytes / 1073741824.0, 2) + " GB";
}

void acaoExplorador(MenuItem& item) {
  if (item.actionId == 1) { 
     iniciarSnake();
  } else if (item.actionId == 2) {
    Serial.println("App/Jogo Selecionado: AudioPlayer");
  } else if (item.actionId == 3 && item.node != nullptr) {
    if (item.node->isDir) {
      item.node->expanded = !item.node->expanded;
      
      if (item.node->expanded && item.node->name == "SD PICO") {
          item.node->children.clear();
          Serial.println("Explorador de arquivos: Solicitando dados ao PICO...");
          
          uartOcupada = true; // Trava o Core 0 para que ele não roube as mensagens
          Serial2.println("DADOS_SD"); 
          
          FileNode* ponteirosNivel[10]; 
          ponteirosNivel[0] = item.node;
          
          unsigned long timeout = millis();
          bool recebendo = false;
          char bufferLinha[256];
          
          while (millis() - timeout < 4000) {
              if (Serial2.available()) {
                  size_t len = Serial2.readBytesUntil('\n', bufferLinha, sizeof(bufferLinha) - 1);
                  bufferLinha[len] = '\0';
                  if(len > 0 && bufferLinha[len-1] == '\r') bufferLinha[len-1] = '\0';
                  
                  if (strcmp(bufferLinha, "INICIO_SD") == 0) {
                      recebendo = true;
                      Serial.println("\n--- CONTEUDO DO CARTAO MICROSD ---");
                      Serial.println("/SD_PICO"); 
                  } else if (strcmp(bufferLinha, "FIM_SD") == 0) {
                      Serial.println("----------------------------------");
                      break; 
                  } else if (recebendo && len > 0) {
                      if (bufferLinha[0] == 'D' || bufferLinha[0] == 'F') {
                          // Parsing ultra-rápido direto do Array
                          char* tipoStr = strtok(bufferLinha, "|");
                          char* nivelStr = strtok(NULL, "|");
                          char* nomeBruto = strtok(NULL, "|");
                          char* tamanhoStr = strtok(NULL, "|");
                          
                          if (tipoStr && nivelStr && nomeBruto) {
                              int nivel = atoi(nivelStr);
                              String nomeLimpo = limparCaracteresEspeciais(String(nomeBruto));
                              
                              for(int i = 0; i <= nivel; i++) Serial.print("  "); 
                              if(tipoStr[0] == 'D') {
                                  Serial.print("/"); Serial.println(nomeLimpo);
                              } else {
                                  String tamanhoFormatado = formatarTamanhoBytes(tamanhoStr ? String(tamanhoStr) : "0");
                                  Serial.print(nomeLimpo);
                                  Serial.print(" \t("); Serial.print(tamanhoFormatado); Serial.println(")");
                              }
                              
                              FileNode no;
                              no.name = nomeLimpo;
                              no.path = ponteirosNivel[nivel]->path + "/" + nomeLimpo;
                              no.isDir = (tipoStr[0] == 'D');
                              no.expanded = false;
                              
                              ponteirosNivel[nivel]->children.push_back(no);
                              if (no.isDir && nivel + 1 < 10) {
                                  ponteirosNivel[nivel + 1] = &(ponteirosNivel[nivel]->children.back());
                              }
                          }
                      }
                  }
              }
          }
          uartOcupada = false; // Libera o Core 0 de volta
      }
      atualizarListaMenu();
      desenharMenu();
    }
  }
}