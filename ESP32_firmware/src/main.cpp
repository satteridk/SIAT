#include "../include/os_globals.h"
#include "../include/apps/app_snake.h"
#include "../include/apps/app_calc.h"
#include "../include/apps/app_explorer.h"

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
MathParser parser;

int cursorX = margemEsquerda;
int cursorY = inicioTextoY;
unsigned long tempoUltimoBlink = 0;
bool cursorVisivel = false;
bool esperandoTexto = true; 
String tituloAtual = "TERMINAL V2.6";
bool modoComando = true; 
bool modoCalc = false;
bool telaInicialCreditos = true;
bool popupAberto = false;
unsigned long tempoAberturaPopup = 0;
int segundosRestantes = 3;
bool estadoAnteriorBotao = HIGH;
unsigned long ultimoDebounce = 0;
FileNode sistemaArquivos;
std::vector<MenuItem> menuAtual;
int opcaoSelecionada = 0;
int marqueeOffset = 0;
unsigned long lastMarqueeUpdate = 0;
bool modoSnake = false;
bool forcarRedrawSnake = false; 

String cacheLinhas[MAX_LINHAS_CACHE];
uint16_t cacheCores[MAX_LINHAS_CACHE][MAX_CHARS_LINHA]; 
int idxLinhaCache = 0;
int indiceLinhaInicioPagina = 0;
int scrollLinha = 0;

volatile bool uartOcupada = false;
TaskHandle_t TaskCore0;

// O MOTOR DO NÚCLEO ZERO: Cuida do hardware em background para o GUI não travar
void TarefaBackground(void *pvParameters) {
  for(;;) {
    if (!uartOcupada && Serial2.available()) {
      String recebido = Serial2.readStringUntil('\n');
      recebido.trim();
      if (recebido.length() > 0) {
        Serial.println("[CORE 0 - PICO MSG]: " + recebido);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Alimenta o Watchdog do ESP32 para não surtar
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(921600, SERIAL_8N1, 16, 17); // <-- Banda extrema habilitada
  Serial2.setTimeout(20);

  // Isola a tarefa secundária no Núcleo 0
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
  sistemaArquivos.expanded = false;
  
  atualizarListaMenu();
  desenharCabecalho();
}

void loop() {
  verificarBotaoFisico();
  if (modoSnake) {
    processarEntradaSnake();
  } else {
    processarEntradaTerminal();
    atualizarMarquee();
    
    if (!popupAberto) {
      if (millis() - tempoUltimoBlink > (unsigned long)intervaloBlink) {
        tempoUltimoBlink = millis();
        cursorVisivel = !cursorVisivel;
        if (cursorVisivel) {
          tft.fillRect(cursorX, cursorY, 6, 8, ST77XX_WHITE);
        } else {
          tft.fillRect(cursorX, cursorY, 6, 8, ST77XX_BLACK);
        }
      }
    }
  }
}