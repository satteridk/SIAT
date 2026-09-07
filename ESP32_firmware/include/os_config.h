#ifndef OS_CONFIG_H
#define OS_CONFIG_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <driver/rtc_io.h>
#include <MathParser_Arduino.h>
#include <vector>

#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2
#define TFT_MOSI  23
#define TFT_SCLK  18
#define PINO_BOTAO 13
#define PINO_TELA  15 

const int margemEsquerda = 5;
const int inicioTextoY = 20;
const int intervaloBlink = 400; 

#define ICONE_PASTA_FECHADA 0
#define ICONE_PASTA_ABERTA 1
#define ICONE_JOYSTICK 2
#define ICONE_MONITOR 3
#define ICONE_ARQUIVO 5

const int MAX_LINHAS_CACHE = 200;
#define MAX_CHARS_LINHA 50 

struct FileNode {
  String name;
  String path;
  bool isDir;
  bool expanded;
  std::vector<FileNode> children;
};

struct MenuItem {
  String name;
  int icon;
  int actionId; 
  FileNode* node;
  int indent;
};

#endif