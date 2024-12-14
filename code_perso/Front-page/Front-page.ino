#include "front.h"

// Initialisation de l'écran
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  
  tft.begin();
  tft.fillScreen(ILI9341_WHITE); // Fond blanc pour commencer

  drawHeader(tft);
  drawDirectionCircle(tft);
  drawProgressBar(tft, 12, 0);
  drawFooter(tft);

  rotateArrowToAngle(tft, 0, 0);
}

void loop() {
  static unsigned long lastRotate = millis();
  if (millis() - lastRotate > 10000) {
    int temps_total_course = 12;
    int progression_bar = 0;
    int distance = 30;
    char * text = "2 casserolles";
    for (int starting_angle = 0; starting_angle < 360; starting_angle += 10) {
        rotateArrowToAngle(tft, starting_angle, starting_angle + 10);
        drawProgressBar(tft, temps_total_course - 3, progression_bar + 25);
        drawDistance(tft, distance - 15);
        drawArticle(tft, text);
    }
    lastRotate = millis();
  }
}
