#include "scanner.h"
#include "front.h"
#include "localisation.h"

// FRONT

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);


// CONNEXION SERVER

const char* ssid = "Coucou"; // Nom du réseau WiFi
const char* password = "coucoucou"; // Mot de passe du réseau WiFi

// Adresse IP publique du serveur
const char* server = "192.168.1.46";

// Port du serveur
const int port = 3444;

const char* endpoint_get_liste = "/scanner/articles/1/1"; // Chemin de l'API getliste
const char* endpoint_get_article = "/scanner/article/"; // Chemin de l'API getarticles

String buffer = "";

WiFiClient wifiClient; // Client WiFi
HttpClient httpClient(wifiClient, server, port);

ProductList productList;


// LOCALISATION

// Adresses MAC des balises cibles
const char* targetAddresses[3] = {
  "f9:15:ff:1d:3f:6e", // Adresse MAC de la balise 1
  "d3:0b:c2:cd:eb:30", // Adresse MAC de la balise 2
  "de:8a:30:26:cf:c2"  // Adresse MAC de la balise 3
};

Position beacons[3] = {
  {0.0, 0.0, 0.0},   // Balise 0
  {5.0, 0.0, 0.0},   // Balise 1
  {2.5, 4.33, 0.0}   // Balise 2
};

// Tableaux pour stocker les périphériques connectés
BLEDevice connectedDevices[3];
bool isConnected[3] = { false, false, false };
int rssi[3] = {0, 0, 0};
float distances[3] = {-1, -1, -1};

// Tableaux circulaires pour stocker les dernières mesures de RSSI
int rssiHistory[3][NUM_MEASUREMENTS] = { {0}, {0}, {0} };
int historyIndex[3] = { 0, 0, 0 }; // Indice actuel pour chaque balise
bool historyFilled[3] = { false, false, false }; // Indique si le tableau est rempli pour chaque balise

float articlePosition[3] = {1.5, 0.2, 0};

// Variables pour le filtre de Kalman
float RSSI_estimate[3] = {0, 0, 0}; // Estimation du RSSI pour chaque balise
float P[3] = {1, 1, 1};             // Incertitude de l'estimation
const float Q[] = {59.61, 83.91, 31.43};                // Bruit du modèle (ajustable)
const float R[] = {25.8, 46.29, 13.11};                  // Bruit de mesure (ajustable)

Position targetObject = {3.0, 2.0, 0.0};




void setup() {
  // initialisation console
  Serial.begin(9600);  // Initialisation du port USB (Serial) pour le moniteur série
  while (!Serial);     // Attente de la connexion avec le moniteur série

  // Initialisation ecran
  tft.begin();
  tft.fillScreen(ILI9341_WHITE); // Fond blanc pour commencer

  drawHeader(tft);
  drawDirectionCircle(tft);
  drawProgressBar(tft, 12, 0);
  drawFooter(tft);
  rotateArrowToAngle(tft, 0, 0);



  // initialisation connexion au backend
  Serial.println("Connexion au WiFi...");
  Serial1.begin(9600);
  while (!Serial1);
  connectToWiFi(ssid, password); // Configuration de Serial1 sur les broches appropriées
  Serial.println("Attente des données sur Serial1...");
  Serial.println("Liste de course intialisée");
  buffer = "";

  // initialisation de la localisation
  if (!BLE.begin()) {
    Serial.println("Erreur d'initialisation BLE!");
    while (1);
  }
  Serial.println("Initialisation réussie. Scannage des périphériques...");
}


void loop() {
  // loop localisation
  //findDevices(isConnected, connectedDevices, targetAddresses);
  //getCoordinates(isConnected, RSSI_estimate, P, Q, R, distances, connectedDevices);
//
  //Position currentPosition = calculatePosition(beacons, distances);
//
  //// Calculer la distance à l'objet cible
  //float distanceToTarget = calculateDistanceToTarget(currentPosition, targetObject);
//
  //// Afficher les résultats
  //Serial.print("Position actuelle : (");
  //Serial.print(currentPosition.x);
  //Serial.print(", ");
  //Serial.print(currentPosition.y);
  //Serial.println(")");
  //Serial.print("Distance à l'objet cible : ");
  //Serial.print(distanceToTarget);
  //Serial.println(" mètres");
//
  //static float previousDistance = -1;
  //detectRightDirection(distanceToTarget, previousDistance);

  while (Serial1.available()) {
    // loop lien backend
    char data = Serial1.read();
    Serial.print(data);

    if (data != '=') {
      buffer += data;  // Ajouter le caractère au tampon
    } else {
      Serial.println("Message complet reçu : " + buffer);
      // Une fois que '\n' est détecté, traiter la chaîne complète
      int separatorIndex = buffer.indexOf(';');
      if (separatorIndex != -1) {
        // Extraire "fct" et "id"
        String fct = buffer.substring(0, separatorIndex);           // Avant le séparateur
        String id = buffer.substring(separatorIndex + 1);          // Après le séparateur

        // Afficher les résultats
        Serial.println("Fonction : " + fct);
        Serial.println("ID : " + id);
        fct.trim();
        if (fct == "get_liste") {
          GetListe(httpClient, endpoint_get_liste, &productList);
        } else if (fct == "get_article") {
          GetArticle(httpClient, endpoint_get_article, &productList, id);
        } else {
          Serial.println("Erreur : Pas de endpoints associé !");
          buffer = "";
        }
      } else {
        Serial.println("Erreur : Pas de séparateur trouvé !");
        buffer = "";
      }
      // Réinitialiser le tampon pour recevoir une nouvelle ligne
      buffer = "";
    }
  }

  //// loop ecran
  //static unsigned long lastRotate = millis();
  //if (millis() - lastRotate > 10000) {
  //  int temps_total_course = 12;
  //  int progression_bar = 0;
  //  int distance = 30;
  //  char * text = "2 casserolles";
  //  for (int starting_angle = 0; starting_angle < 360; starting_angle += 10) {
  //      rotateArrowToAngle(tft, starting_angle, starting_angle + 10);
  //      drawProgressBar(tft, temps_total_course - 3, progression_bar + 25);
  //      drawDistance(tft, distance - 15);
  //      drawArticle(tft, text);
  //  }
  //  lastRotate = millis();
  //}
  
  
  
  //delay(400); // Petite pause pour éviter une surcharge du processeur
}