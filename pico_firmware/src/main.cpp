#include "../include/pico_globals.h"
#include <SD.h> 
#include "../include/core/sd_manager.h"
#include "../include/core/uart_handler.h"

bool cartao_conectado = false;
bool mensagem_aguardando_exibida = false;
unsigned long tempo_anterior_serial = 0;

char cacheSD[8192] = "";
int cacheIndex = 0;

void setup() {
  Serial.begin(115200);

  Serial2.setTX(20);
  Serial2.setRX(21);
  Serial2.begin(921600); // <-- Velocidade extrema
  Serial2.setTimeout(20);
  
  unsigned long inicio_usb = millis();
  while (!Serial && (millis() - inicio_usb < 4000)) { delay(10); }
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(PINO_DETECT, INPUT_PULLUP);
  
  SPI.setRX(PINO_MISO); 
  SPI.setCS(PINO_CS);   
  SPI.setSCK(PINO_SCK); 
  SPI.setTX(PINO_MOSI); 
  SPI.begin();

  inicializarSD();
  tempo_anterior_serial = millis(); 
}

void loop() {
  verificarStatusSD();
  processarComandosUART();
}