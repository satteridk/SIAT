#ifndef OS_GLOBALS_H
#define OS_GLOBALS_H

#include "os_config.h"

// ==========================================
// NUCLEO - apenas recursos realmente transversais
// ==========================================
extern Adafruit_ST7789 tft;
extern MathParser parser;

#define COR_CINZA             tft.color565(130, 130, 130)
#define COR_FUNDO_SELECIONADO tft.color565(50, 50, 50)
#define COR_PASTA             tft.color565(255, 215, 0)
#define COR_PASTA_CLARA       tft.color565(255, 245, 150)
#define COR_SNAKE             tft.color565(0, 200, 90)

// ==========================================
// MAQUINA DE ESTADOS FINITA (FSM)
// ==========================================
enum EstadoSistema {
  TERMINAL_CMD,
  APP_CALCULADORA,
  APP_SNAKE,
  APP_EXPLORADOR,
  POPUP_DESLIGAR
};

extern EstadoSistema estadoAtual;
extern EstadoSistema estadoAnterior;

extern volatile bool uartOcupada;   // Arbitragem da UART2 entre Core 0 e Core 1
extern bool forcarRedrawSnake;      // Repintura solicitada por outro modulo

// ==========================================
// AGREGACAO DOS MODULOS
// Cada modulo declara o proprio estado e a propria API.
// ==========================================
#include "gui/os_icons.h"
#include "gui/os_gui.h"
#include "core/os_terminal.h"
#include "core/os_hardware.h"

#endif