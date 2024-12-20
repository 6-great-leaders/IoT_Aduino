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

// Initialisation de l'écran
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  
  tft.begin();
  tft.fillScreen(ILI9341_WHITE); // Fond blanc pour commencer

  drawHeader();
  drawDirectionCircle();
  drawProgressBar(12, 0);
  drawFooter();

  rotateArrowToAngle(0, 0);
}

void loop() {
  static unsigned long lastRotate = millis();
  if (millis() - lastRotate > 10000) {
    int temps_total_course = 12;
    int progression_bar = 0;
    int distance = 30;
    char * text = "2 casserolles";
    for (int starting_angle = 0; starting_angle < 360; starting_angle += 10) {
        rotateArrowToAngle(starting_angle, starting_angle + 10);
        drawProgressBar(temps_total_course - 3, progression_bar + 25);
        drawDistance(distance - 15);
        drawArticle(text);
    }
    lastRotate = millis();
  }
}

void drawDistance(int distance) {
  tft.fillRect(10, 15, 100, 30, tft.color565(39, 194, 120)); // Rectangle vert #27C278

  tft.setCursor(10, 15); // Position du texte de distance
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print(distance);
  tft.print(" metres");
}

void drawArticle(char * text){
  tft.fillRect(180, 40, 100, 10, tft.color565(39, 194, 120)); // Rectangle vert #27C278
  tft.setCursor(180, 40); 
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print(text);
}

// Fonction pour dessiner l'en-tête
void drawHeader() {
  tft.fillRect(0, 0, 240, 50, tft.color565(39, 194, 120)); // Rectangle vert #27C278
  
  drawDistance(30);

  tft.drawCircle(200, 25, 15, ILI9341_WHITE); // Cercle autour de l'image
  tft.fillCircle(200, 25, 14, ILI9341_WHITE); // Fond blanc pour image
  // Remplacez par l'image d'œufs si disponible
  
  drawArticle("12 oeufs");
}

// Fonction pour dessiner le cercle et la flèche
void drawDirectionCircle() {
  tft.fillCircle(120, 140, 60, tft.color565(39, 194, 120)); // Cercle vert
  
  tft.fillTriangle(110, 100, 130, 100, 120, 70, ILI9341_WHITE); // Flèche blanche
}

// Fonction pour animer une rotation de la flèche
void rotateArrow() {
  for (int angle = 0; angle <= 360; angle += 10) {
    tft.fillCircle(120, 140, 60, tft.color565(39, 194, 120)); // Efface l'ancien
    tft.fillTriangle(
      120 + 30 * cos(radians(angle)),
      140 - 30 * sin(radians(angle)),
      120 + 20 * cos(radians(angle + 120)),
      140 - 20 * sin(radians(angle + 120)),
      120 + 20 * cos(radians(angle + 240)),
      140 - 20 * sin(radians(angle + 240)),
      ILI9341_WHITE
    );
    delay(50);
  }
}

// Fonction pour animer une rotation de la flèche vers un angle donné
void rotateArrowToAngle(int currentAngle, int targetAngle) {
  // Détermine la direction de rotation (horaire ou antihoraire)
  int step = (targetAngle > currentAngle) ? 10 : -10;

  // Continue la rotation jusqu'à atteindre l'angle cible
  while ((step > 0 && currentAngle <= targetAngle) || (step < 0 && currentAngle >= targetAngle)) {
    // Efface l'ancien triangle
    tft.fillCircle(120, 140, 60, tft.color565(39, 194, 120)); 

    // Calcule les positions des points du triangle pour l'angle courant
    float angleRad = radians(currentAngle);
    float x1 = 120 + 30 * cos(angleRad);
    float y1 = 140 - 30 * sin(angleRad);
    float x2 = 120 + 20 * cos(radians(currentAngle + 120));
    float y2 = 140 - 20 * sin(radians(currentAngle + 120));
    float x3 = 120 + 20 * cos(radians(currentAngle + 240));
    float y3 = 140 - 20 * sin(radians(currentAngle + 240));

    // Dessine le triangle à l'angle actuel
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_WHITE);

    // Attente pour créer l'animation
    delay(50);

    // Passe à l'angle suivant
    currentAngle += step;
  }

  // S'assure que le triangle termine exactement à l'angle cible
  if (currentAngle != targetAngle) {
    tft.fillCircle(120, 140, 60, tft.color565(39, 194, 120));

    float angleRad = radians(targetAngle);
    float x1 = 120 + 30 * cos(angleRad);
    float y1 = 140 - 30 * sin(angleRad);
    float x2 = 120 + 20 * cos(radians(targetAngle + 120));
    float y2 = 140 - 20 * sin(radians(targetAngle + 120));
    float x3 = 120 + 20 * cos(radians(targetAngle + 240));
    float y3 = 140 - 20 * sin(radians(targetAngle + 240));

    tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_WHITE);
  }
}


// Fonction pour dessiner la barre de progression
void drawProgressBar(int temps_restant, int progression_bar) {
  tft.fillRect(5, 230, 250, 20, ILI9341_WHITE); // effacer l'écriture précédente

  tft.setCursor(5, 230);
  tft.setTextColor(ILI9341_BLACK); // Couleur bleue #42BDE3
  tft.setTextSize(1);
  tft.print("Temps de course restant estime a ");
  tft.print(temps_restant);
  tft.print("min");

  tft.fillRect(20, 250, 200, 10, ILI9341_LIGHTGREY); // Fond de la barre
  tft.fillRect(20, 250, progression_bar * 2, 10, tft.color565(66, 189, 227)); // Barre bleue #42BDE3
  
}

void drawBoldText(const char* text, int x, int y, uint16_t color, uint8_t size = 1) {
  tft.setTextColor(color);
  tft.setTextSize(size);

  // Dessine le texte plusieurs fois légèrement décalé
  tft.setCursor(x, y);
  tft.print(text);

  tft.setCursor(x + 1, y); // Décalage horizontal
  tft.print(text);

  tft.setCursor(x, y + 1); // Décalage vertical
  tft.print(text);
}

// Fonction pour dessiner le pied de page
void drawFooter() {
  tft.fillRect(0, 280, 240, 40, ILI9341_LIGHTGREY); // Rectangle gris pour les boutons

  // Dessin des boutons
  tft.fillRect(10, 285, 60, 30, tft.color565(39, 194, 120)); // Bouton Carte
  tft.setCursor(20, 290);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("Carte");

  tft.fillRect(90, 285, 60, 30, ILI9341_WHITE); // Bouton Mes Achats
  tft.setCursor(100, 290);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.print("Achats");

  tft.fillRect(170, 285, 60, 30, ILI9341_WHITE); // Bouton Ma Liste
  tft.setCursor(180, 290);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.print("Liste");
}