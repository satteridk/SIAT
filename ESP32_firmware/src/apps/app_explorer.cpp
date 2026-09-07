#include "../../include/apps/app_explorer.h"
#include "../../include/apps/app_snake.h"

// Tradutor de caracteres para evitar erros no Monitor Serial e no Adafruit_GFX
String limparCaracteresEspeciais(String texto) {
  String t = texto;
  
  // 1. Tenta traduzir os acentos UTF-8 mais comuns para versões sem acento
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
  
  // 2. Filtro de Segurança: Remove lixo invisível e caracteres não suportados pela tela
  String limpo = "";
  for (int i = 0; i < t.length(); i++) {
    unsigned char c = t[i];
    if (c >= 32 && c <= 126) { // Apenas caracteres ASCII imprimíveis normais
      limpo += (char)c;
    } else if (c > 127) {
      limpo += '_'; // Substitui bytes soltos/quebrados por underline
    }
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
                          Serial.println("/SD_PICO"); 
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
                              String nomeBruto = linha.substring(pos2 + 1, (pos3 == -1) ? linha.length() : pos3);
                              String nomeLimpo = limparCaracteresEspeciais(nomeBruto);
                              String tamanhoStr = (pos3 != -1) ? linha.substring(pos3 + 1) : "";
                              
                              for(int i = 0; i <= nivel; i++) Serial.print("  "); 
                              
                              if(tipo == "D") {
                                  Serial.print("/"); Serial.println(nomeLimpo);
                              } else {
                                  String tamanhoFormatado = formatarTamanhoBytes(tamanhoStr);
                                  Serial.print(nomeLimpo);
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
                    String nomeBruto = linha.substring(p2 + 1, (p3 == -1) ? linha.length() : p3);
                    String nomeLimpo = limparCaracteresEspeciais(nomeBruto);
                    
                    FileNode no;
                    no.name = nomeLimpo;
                    no.path = ponteirosNivel[nivel]->path + "/" + nomeLimpo;
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