#include "../../include/apps/app_explorer.h"
#include "../../include/apps/app_snake.h"

// Função auxiliar para traduzir bytes para KB, MB e GB
String formatarTamanhoBytes(String tamanhoStr) {
  if (tamanhoStr.length() == 0) return "";
  double bytes = tamanhoStr.toDouble(); // Usa double para evitar estouro de limite de 2GB
  
  if (bytes < 1024) return String((int)bytes) + " bytes";
  else if (bytes < 1048576) return String(bytes / 1024.0, 2) + " KB";
  else if (bytes < 1073741824) return String(bytes / 1048576.0, 2) + " MB";
  else return String(bytes / 1073741824.0, 2) + " GB";
}

void acaoExplorador(MenuItem& item) {
  if (item.actionId == 1) { 
     iniciarSnake();
  } 
  else if (item.actionId == 2) {
    Serial.println("App/Jogo Selecionado: AudioPlayer");
  } 
  else if (item.actionId == 3 && item.node != nullptr) {
    if (item.node->isDir) {
      item.node->expanded = !item.node->expanded;
      
      if (item.node->expanded) {
          if (item.node->name == "SD PICO") {
              item.node->children.clear();
              
              Serial.println("Explorador de arquivos: Solicitando dados ao PICO...");
              Serial2.println("DADOS_SD"); 
              
              String dadosSD = ""; 
              unsigned long timeout = millis();
              bool recebendo = false;
              
              while (millis() - timeout < 4000) {
                  if (Serial2.available()) {
                      String linha = Serial2.readStringUntil('\n');
                      linha.trim();
                      
                      if (linha == "INICIO_SD") {
                          recebendo = true;
                          Serial.println("\n--- CONTEUDO DO CARTAO MICROSD ---");
                          Serial.println("/SD_PICO"); // Imprime o "Disco" raiz
                      } else if (linha == "FIM_SD") {
                          Serial.println("----------------------------------");
                          break; 
                      } else if (recebendo && linha.length() > 0) {
                          dadosSD += linha + "\n"; 
                          
                          if (linha.startsWith("D|") || linha.startsWith("F|")) {
                              int pos1 = linha.indexOf('|');
                              int pos2 = linha.indexOf('|', pos1 + 1);
                              int pos3 = linha.indexOf('|', pos2 + 1);
                              
                              String tipo = linha.substring(0, pos1);
                              int nivel = linha.substring(pos1 + 1, pos2).toInt();
                              String nome = linha.substring(pos2 + 1, (pos3 == -1) ? linha.length() : pos3);
                              String tamanhoStr = (pos3 != -1) ? linha.substring(pos3 + 1) : "";
                              
                              // Adiciona um espaço extra de indentação (+1) para simular que estão DENTRO do /SD_PICO
                              for(int i = 0; i <= nivel; i++) Serial.print("  "); 
                              
                              if(tipo == "D") {
                                  Serial.print("[PASTA]   /"); Serial.println(nome);
                              } else {
                                  String tamanhoFormatado = formatarTamanhoBytes(tamanhoStr);
                                  Serial.print("[ARQUIVO] "); Serial.print(nome);
                                  Serial.print(" \t("); Serial.print(tamanhoFormatado); Serial.println(")");
                              }
                          }
                      }
                  }
              }
              
              FileNode* ponteirosNivel[10]; 
              ponteirosNivel[0] = item.node;
              
              int pos = 0;
              while (pos < (int)dadosSD.length()) {
                int nextPos = dadosSD.indexOf('\n', pos);
                if (nextPos == -1) break;
                String linha = dadosSD.substring(pos, nextPos);
                linha.trim();
                
                if (linha.startsWith("D|") || linha.startsWith("F|")) {
                    int p1 = linha.indexOf('|');
                    int p2 = linha.indexOf('|', p1 + 1);
                    int p3 = linha.indexOf('|', p2 + 1);
                    
                    String tipo = linha.substring(0, p1);
                    int nivel = linha.substring(p1 + 1, p2).toInt();
                    String nome = linha.substring(p2 + 1, (p3 == -1) ? linha.length() : p3);
                    
                    FileNode no;
                    no.name = nome;
                    no.path = ponteirosNivel[nivel]->path + "/" + nome;
                    no.isDir = (tipo == "D");
                    no.expanded = false;
                    
                    ponteirosNivel[nivel]->children.push_back(no);
                    
                    if (tipo == "D" && nivel + 1 < 10) {
                        ponteirosNivel[nivel + 1] = &(ponteirosNivel[nivel]->children.back());
                    }
                }
                pos = nextPos + 1;
              }
          }
      }
      atualizarListaMenu();
      desenharMenu();
    }
  }
}