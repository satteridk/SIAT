#include "../include/os_globals.h"
#include "../include/apps/app_snake.h"
#include "../include/apps/app_calc.h"
#include "../include/apps/app_explorer.h"

// ==========================================
// DEFINICAO DOS RECURSOS TRANSVERSAIS
// ==========================================
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
MathParser parser;

EstadoSistema estadoAtual    = TERMINAL_CMD;
EstadoSistema estadoAnterior = TERMINAL_CMD;

volatile bool uartOcupada = false;
bool forcarRedrawSnake = false;

TaskHandle_t TaskCore0;

// Task assincrona travada no Core 0: drena a UART2 quando nenhum app a reivindica
void TarefaBackground(void *pvParameters) {
  for (;;) {
    if (!uartOcupada && Serial2.available()) {
      String recebido = Serial2.readStringUntil('\n');
      recebido.trim();
      if (recebido.length() > 0) {
        Serial.println("[CORE 0 - PICO MSG]: " + recebido);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(SERIAL_TIMEOUT_MS);          // Zero lag: 10ms em vez dos 1000ms default

  Serial2.begin(UART_PICO_BAUD, SERIAL_8N1, UART_PICO_RX, UART_PICO_TX);
  Serial2.setTimeout(UART_TIMEOUT_MS);

  xTaskCreatePinnedToCore(TarefaBackground, "TaskCore0", 4096, NULL, 1, &TaskCore0, 0);

  pinMode(PINO_BOTAO, INPUT_PULLUP);
  pinMode(PINO_TELA, OUTPUT);
  digitalWrite(PINO_TELA, LOW);

  tft.init(240, 280);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  animacaoDeBoot();

  sistemaArquivos.name = "SD PICO";
  sistemaArquivos.path = "/";
  sistemaArquivos.isDir = true;

  atualizarListaMenu();
  desenharCabecalho();
}

void loop() {
  verificarBotaoFisico();

  // Roteador central da FSM
  switch (estadoAtual) {
    case TERMINAL_CMD:
      processarEntradaTerminal();
      break;
    case APP_CALCULADORA:
      processarEntradaCalc();
      break;
    case APP_SNAKE:
      processarEntradaSnake();
      break;
    case APP_EXPLORADOR:
      processarEntradaExplorador();
      break;
    case POPUP_DESLIGAR:
      // Apps congelam; o relogio do popup roda isolado em verificarBotaoFisico()
      break;
  }

  // Efeitos visuais apenas fora dos apps fullscreen
  if (estadoAtual == TERMINAL_CMD || estadoAtual == APP_CALCULADORA) {
    atualizarMarquee();
    if (millis() - tempoUltimoBlink > (unsigned long)intervaloBlink) {
      tempoUltimoBlink = millis();
      cursorVisivel = !cursorVisivel;
      tft.fillRect(cursorX, cursorY, LARGURA_CHAR, 8,
                   cursorVisivel ? ST77XX_WHITE : ST77XX_BLACK);
    }
  }
}