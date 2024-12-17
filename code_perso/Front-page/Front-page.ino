#include "front.h"

// Initialisation de l'écran
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// Variable pour indiquer si le QR Code a été scanné
bool qrCodeScanned = false;

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(ILI9341_WHITE); // Effacer l'écran au démarrage

  // Affiche l'écran d'accueil
  drawWelcomeScreen(&tft);
}

void loop() {
  if (!qrCodeScanned) {
    // Ici, on va insérer le code pour vérifier le scan du QR Code
    if (checkQRCodeScan()) { // Fonction de mattéo qui retourne true (ou le json, voir avec lui) quand QR Code est scanné
      qrCodeScanned = true;
      tft.fillScreen(ILI9341_WHITE); // Efface l'écran pour éviter les anciens contenus
      drawHeader(&tft);
      drawDirectionCircle(&tft);
      drawProgressBar(&tft, 12, 0);
      drawFooter(&tft);

      rotateArrowToAngle(&tft, 0, 0)
    }
  }
  /*rajouter un truc de quand la barre atteint 100% :
  if (progression_bar >= 100) {
      drawEndScreen(&tft);
      while (true); // Arrête la boucle une fois l'écran final affiché
    }
  */
}
