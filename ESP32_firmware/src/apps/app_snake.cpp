#include "../../include/apps/app_snake.h"

#define MAX_SNAKE_LEN 150
// Declaradas como 'static' para ficarem isoladas da memória global e não conflitar com outros apps
static int snakeX[MAX_SNAKE_LEN];
static int snakeY[MAX_SNAKE_LEN];
static int snakeLength = 3;
static int snakeDir = 1; 
static int lastSnakeDir = 1; 
static int foodX = 0;
static int foodY = 0;
static int snakeScore = 0;
static unsigned long tempoUltimoMovimentoSnake = 0;
static int velocidadeSnake = 120;
static const int tamanhoBloco = 6;
static int gameAreaX, gameAreaY, gameAreaW, gameAreaH;

void atualizarPlacarSnake() {
  tft.fillRect(1, 1, tft.width()-2, 17, ST77XX_BLACK); 
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  
  char placarStr[20];
  sprintf(placarStr, "SCORE: %06d", snakeScore);
  
  int larguraTexto = 13 * 6; 
  int centroX = (tft.width() - larguraTexto) / 2;
  
  tft.setCursor(centroX, 6);
  tft.print(placarStr);
}

void gerarComidaSnake() {
  int maxX = gameAreaW / tamanhoBloco;
  int maxY = gameAreaH / tamanhoBloco;
  bool valido = false;
  
  while(!valido) {
    foodX = random(0, maxX);
    foodY = random(0, maxY);
    valido = true;
    for(int i = 0; i < snakeLength; i++) {
      if(snakeX[i] == foodX && snakeY[i] == foodY) {
        valido = false; 
        break;
      }
    }
  }
  
  int realX = gameAreaX + (foodX * tamanhoBloco);
  int realY = gameAreaY + (foodY * tamanhoBloco);
  tft.fillCircle(realX + tamanhoBloco/2, realY + tamanhoBloco/2, tamanhoBloco/2 - 1, ST77XX_WHITE);
}

void resetarFaseSnake() {
  snakeLength = 3;
  snakeDir = 1;
  lastSnakeDir = 1;
  velocidadeSnake = 120;
  
  int startX = (gameAreaW / tamanhoBloco) / 2;
  int startY = (gameAreaH / tamanhoBloco) / 2;
  
  for(int i = 0; i < snakeLength; i++) {
    snakeX[i] = startX - i;
    snakeY[i] = startY;
  }
  
  tft.fillRect(gameAreaX, gameAreaY, gameAreaW, gameAreaH, ST77XX_BLACK);
  gerarComidaSnake();
}

void iniciarSnake() {
  modoSnake = true;
  snakeScore = 0;
  
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
  tft.drawFastHLine(0, 19, tft.width(), ST77XX_WHITE);
  
  gameAreaX = 1;
  gameAreaY = 20;
  gameAreaW = tft.width() - 2;
  int rawH = tft.height() - 21;
  gameAreaH = rawH - (rawH % tamanhoBloco); 
  
  atualizarPlacarSnake();
  resetarFaseSnake();
  tempoUltimoMovimentoSnake = millis();
}

void sairSnake() {
  modoSnake = false;
  if (scrollLinha > 0) {
    renderizarScroll();
  } else {
    restaurarPaginaAtual();
  }
  if (modoComando) {
    if (cursorX > margemEsquerda) {
      avancarLinha();
    }
    escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
  }
}

void atualizarJogoSnake() {
  if (forcarRedrawSnake) {
    tft.fillScreen(ST77XX_BLACK);
    tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
    tft.drawFastHLine(0, 19, tft.width(), ST77XX_WHITE);
    atualizarPlacarSnake();
    
    for(int i = 0; i < snakeLength; i++) {
      int px = gameAreaX + (snakeX[i] * tamanhoBloco);
      int py = gameAreaY + (snakeY[i] * tamanhoBloco);
      tft.fillRect(px, py, tamanhoBloco - 1, tamanhoBloco - 1, ST77XX_WHITE);
    }
    int realX = gameAreaX + (foodX * tamanhoBloco);
    int realY = gameAreaY + (foodY * tamanhoBloco);
    tft.fillCircle(realX + tamanhoBloco/2, realY + tamanhoBloco/2, tamanhoBloco/2 - 1, ST77XX_WHITE);
    
    forcarRedrawSnake = false;
  }

  if (millis() - tempoUltimoMovimentoSnake < (unsigned long)velocidadeSnake) return;
  tempoUltimoMovimentoSnake = millis();
  
  lastSnakeDir = snakeDir; 

  int tailX = gameAreaX + (snakeX[snakeLength-1] * tamanhoBloco);
  int tailY = gameAreaY + (snakeY[snakeLength-1] * tamanhoBloco);
  tft.fillRect(tailX, tailY, tamanhoBloco, tamanhoBloco, ST77XX_BLACK); 

  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }

  if (snakeDir == 0) snakeY[0]--;
  else if (snakeDir == 1) snakeX[0]++;
  else if (snakeDir == 2) snakeY[0]++;
  else if (snakeDir == 3) snakeX[0]--;

  int maxX = gameAreaW / tamanhoBloco;
  int maxY = gameAreaH / tamanhoBloco;

  bool morreu = false;

  if (snakeX[0] < 0 || snakeX[0] >= maxX || snakeY[0] < 0 || snakeY[0] >= maxY) {
    morreu = true;
  }

  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      morreu = true;
    }
  }

  if (morreu) {
    delay(500); 
    snakeScore = 0; 
    atualizarPlacarSnake();
    resetarFaseSnake();
    return;
  }

  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    if (snakeLength < MAX_SNAKE_LEN) snakeLength++;
    snakeScore += 100;
    atualizarPlacarSnake();
    gerarComidaSnake();
    velocidadeSnake = max(50, velocidadeSnake - 3); 
  }

  for (int i = 0; i < snakeLength; i++) {
    int px = gameAreaX + (snakeX[i] * tamanhoBloco);
    int py = gameAreaY + (snakeY[i] * tamanhoBloco);
    tft.fillRect(px, py, tamanhoBloco - 1, tamanhoBloco - 1, ST77XX_WHITE);
  }
}

void processarEntradaSnake() {
  if (Serial.available()) {
    String inputSnake = Serial.readString();
    inputSnake.trim();
    inputSnake.toLowerCase(); 

    if (inputSnake == "exit") {
      sairSnake();
      return; 
    } 
    else if (inputSnake == "w" && lastSnakeDir != 2) snakeDir = 0;
    else if (inputSnake == "s" && lastSnakeDir != 0) snakeDir = 2;
    else if (inputSnake == "a" && lastSnakeDir != 1) snakeDir = 3;
    else if (inputSnake == "d" && lastSnakeDir != 3) snakeDir = 1;
  }
  
  if (!popupAberto) {
    atualizarJogoSnake();
  }
}