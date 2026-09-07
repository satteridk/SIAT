#include <Arduino.h>
#include "../../include/os_globals.h"
#include "../../include/apps/app_snake.h"

int snakeX[100], snakeY[100];
int snakeDir = 1; 
int lastSnakeDir = 1; 
int snakeLength = 3;
int foodX, foodY;
int snakeScore = 0;
unsigned long tempoUltimoMovimentoSnake = 0;
int velocidadeSnake = 150; 

int gameAreaX, gameAreaY, gameAreaW, gameAreaH;
const int tamanhoBloco = 8; 

void desenharBloco(int x, int y, uint16_t cor) {
  tft.fillRect(gameAreaX + (x * tamanhoBloco), gameAreaY + (y * tamanhoBloco), tamanhoBloco, tamanhoBloco, cor);
}

void apagarBloco(int x, int y) {
  tft.fillRect(gameAreaX + (x * tamanhoBloco), gameAreaY + (y * tamanhoBloco), tamanhoBloco, tamanhoBloco, ST77XX_BLACK);
}

void gerarComidaSnake() {
  int maxX = gameAreaW / tamanhoBloco;
  int maxY = gameAreaH / tamanhoBloco;
  
  bool posicaoValida = false;
  while (!posicaoValida) {
    foodX = random(0, maxX);
    foodY = random(0, maxY);
    posicaoValida = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        posicaoValida = false;
        break;
      }
    }
  }
  desenharBloco(foodX, foodY, ST77XX_RED);
}

void atualizarPlacarSnake() {
  tft.fillRect(0, 0, tft.width(), 18, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 5);
  tft.print("SCORE: ");
  tft.print(snakeScore);
  
  String controls = "W A S D - EXIT";
  tft.setCursor(tft.width() - (controls.length() * 6) - 5, 5);
  tft.print(controls);
}

void resetarFaseSnake() {
  snakeLength = 3;
  snakeDir = 1;
  lastSnakeDir = 1;
  int startX = (gameAreaW / tamanhoBloco) / 2;
  int startY = (gameAreaH / tamanhoBloco) / 2;
  
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = startX - i;
    snakeY[i] = startY;
  }
  gerarComidaSnake();
}

void desenharCobra() {
  for (int i = 0; i < snakeLength; i++) {
    uint16_t cor = (i == 0) ? ST77XX_GREEN : tft.color565(0, 180, 0); 
    desenharBloco(snakeX[i], snakeY[i], cor);
  }
}

void gameOverSnake() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_RED);
  tft.setCursor((tft.width() - (9 * 12)) / 2, (tft.height() / 2) - 20);
  tft.print("GAME OVER");
  
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor((tft.width() - (15 * 6)) / 2, (tft.height() / 2) + 10);
  tft.print("Score Final: ");
  tft.print(snakeScore);
  
  delay(3000); 
  iniciarSnake(); 
}

void atualizarJogoSnake() {
  if (forcarRedrawSnake) {
    forcarRedrawSnake = false;
    tft.fillScreen(ST77XX_BLACK);
    tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
    tft.drawFastHLine(0, 19, tft.width(), ST77XX_WHITE);
    atualizarPlacarSnake();
    desenharCobra();
    desenharBloco(foodX, foodY, ST77XX_RED);
  }

  if (millis() - tempoUltimoMovimentoSnake > (unsigned long)velocidadeSnake) {
    tempoUltimoMovimentoSnake = millis();
    lastSnakeDir = snakeDir; 
    
    int nextX = snakeX[0];
    int nextY = snakeY[0];
    
    if (snakeDir == 0) nextY--;      
    else if (snakeDir == 1) nextX++; 
    else if (snakeDir == 2) nextY++; 
    else if (snakeDir == 3) nextX--; 

    int maxX = gameAreaW / tamanhoBloco;
    int maxY = gameAreaH / tamanhoBloco;

    if (nextX < 0 || nextX >= maxX || nextY < 0 || nextY >= maxY) {
      gameOverSnake();
      return;
    }

    for (int i = 0; i < snakeLength; i++) {
      if (nextX == snakeX[i] && nextY == snakeY[i]) {
        gameOverSnake();
        return;
      }
    }

    apagarBloco(snakeX[snakeLength - 1], snakeY[snakeLength - 1]);

    for (int i = snakeLength - 1; i > 0; i--) {
      snakeX[i] = snakeX[i - 1];
      snakeY[i] = snakeY[i - 1];
    }
    
    snakeX[0] = nextX;
    snakeY[0] = nextY;

    if (nextX == foodX && nextY == foodY) {
      snakeScore += 10;
      if (snakeLength < 100) snakeLength++;
      atualizarPlacarSnake();
      gerarComidaSnake();
    }
    
    desenharCobra();
  }
}

void iniciarSnake() {
  estadoAtual = APP_SNAKE;
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
  estadoAtual = TERMINAL_CMD;
  if (scrollLinha > 0) {
    renderizarScroll();
  } else {
    restaurarPaginaAtual();
  }

  if (cursorX > margemEsquerda) {
    avancarLinha();
  }
  escreverEfeitoDigitacao("CMD> ", 1, ST77XX_WHITE);
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
  atualizarJogoSnake();
}