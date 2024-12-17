// front.h
#ifndef FRONT_H
#define FRONT_H

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define _USE_MATH_DEFINES // Assure l'utilisation de M_PI pour Pi
#include <math.h>

// Configuration des broches pour l'écran
#define TFT_CS    7
#define TFT_DC    11
#define TFT_MOSI  8
#define TFT_CLK   SCK
#define TFT_RST   6
#define TFT_MISO  10

void drawWelcomeScreen(Adafruit_ILI9341 tft);

void drawDistance(Adafruit_ILI9341 tft, float distance);

void drawArticle(Adafruit_ILI9341 tft, const char * text);

void drawHeader(Adafruit_ILI9341 tft);

void drawDirectionCircle(Adafruit_ILI9341 tft);

void rotateArrow(Adafruit_ILI9341 tft);

void rotateArrowToAngle(Adafruit_ILI9341 tft, int currentAngle, int targetAngle);

void drawProgressBar(Adafruit_ILI9341 tft, int temps_restant, int progression_bar);

void drawBoldText(Adafruit_ILI9341 tft, const char* text, int x, int y, uint16_t color, uint8_t size);

void drawFooter(Adafruit_ILI9341 tft);

void drawEndScreen(Adafruit_ILI9341 tft);

#endif