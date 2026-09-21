#ifndef OS_CONFIG_H
#define OS_CONFIG_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <driver/rtc_io.h>
#include <MathParser_Arduino.h>
#include <vector>

// --- MAPEAMENTO DE HARDWARE ---
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2
#define TFT_MOSI  23
#define TFT_SCLK  18
#define PINO_BOTAO 13
#define PINO_TELA  15

// --- BARRAMENTO SERIAL ---
#define SERIAL_TIMEOUT_MS  10       // Zero lag no terminal USB
#define UART_TIMEOUT_MS    20
#define UART_PICO_BAUD     921600
#define UART_PICO_RX       16
#define UART_PICO_TX       17

// --- METRICAS DE LAYOUT ---
const int margemEsquerda = 5;
const int inicioTextoY   = 20;
const int intervaloBlink = 400;

#define ALTURA_LINHA      12
#define ALTURA_ITEM_MENU  16
#define LARGURA_CHAR      6

// --- ATLAS DE ICONES ---
#define ICONE_PASTA_FECHADA 0
#define ICONE_PASTA_ABERTA  1
#define ICONE_MONITOR       3
#define ICONE_ARQUIVO       5
#define ICONE_MUSICA        6
#define ICONE_TEXTO         7
#define ICONE_CMD           8
#define ICONE_SNAKE         9
#define ICONE_VOLTAR        10

// --- ACOES DO MENU PRINCIPAL ---
#define ACAO_SNAKE     1
#define ACAO_AUDIO     2
#define ACAO_EXPLORER  3
#define ACAO_TERMINAL  4

// --- CACHE DE TEXTO ---
const int MAX_LINHAS_CACHE = 200;
#define MAX_CHARS_LINHA 50

// --- EXPLORADOR ---
#define EXPLORER_MAX_NIVEL 10

struct FileNode {
  String name;
  String path;
  bool isDir;
  uint32_t size;
  std::vector<FileNode> children;

  FileNode() : isDir(false), size(0) {}
};

struct MenuItem {
  String name;
  int icon;
  int actionId;
  FileNode* node;
  int indent;

  MenuItem() : icon(ICONE_ARQUIVO), actionId(0), node(nullptr), indent(0) {}
};

#endif