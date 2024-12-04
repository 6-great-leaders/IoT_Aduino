#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

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
  drawProgressBar();
  drawFooter();
}

void loop() {
  // Simule un 360° toutes les 10 secondes
  static unsigned long lastRotate = millis();
  if (millis() - lastRotate > 10000) {
    rotateArrow();
    lastRotate = millis();
  }
}

// Fonction pour dessiner l'en-tête
void drawHeader() {
  tft.fillRect(0, 0, 240, 50, tft.color565(39, 194, 120)); // Rectangle vert #27C278
  
  tft.setCursor(10, 15); // Position du texte de distance
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("30 metres");

  tft.drawCircle(200, 25, 15, ILI9341_WHITE); // Cercle autour de l'image
  tft.fillCircle(200, 25, 14, ILI9341_WHITE); // Fond blanc pour image
  // Remplacez par l'image d'œufs si disponible
  tft.setCursor(180, 40); 
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print("12 oeufs");
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

// Fonction pour dessiner la barre de progression
void drawProgressBar() {
  tft.setCursor(5, 230);
  tft.setTextColor(ILI9341_BLACK); // Couleur bleue #42BDE3
  tft.setTextSize(1);
  tft.print("Temps de course restant estime a 12min");

  tft.fillRect(20, 250, 200, 10, ILI9341_LIGHTGREY); // Fond de la barre
  tft.fillRect(20, 250, 120, 10, tft.color565(66, 189, 227)); // Barre bleue #42BDE3
  
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