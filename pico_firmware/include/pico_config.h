#ifndef PICO_CONFIG_H
#define PICO_CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h> 

// Pinos do Cartão SD (SPI0)
const int PINO_MISO   = 0; 
const int PINO_CS     = 1; 
const int PINO_SCK    = 2; 
const int PINO_MOSI   = 3; 
const int PINO_DETECT = 4; 

#endif