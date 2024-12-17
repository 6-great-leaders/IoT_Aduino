#include "localisation.h"

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
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Erreur d'initialisation BLE!");
    while (1);
  }

  Serial.println("Initialisation réussie. Scannage des périphériques...");
}

void loop() {
  findDevices(isConnected, connectedDevices, targetAddresses);
  getCoordinates(isConnected, RSSI_estimate, P, Q, R, distances, connectedDevices);

  Position currentPosition = calculatePosition(beacons, distances);

  // Calculer la distance à l'objet cible
  float distanceToTarget = calculateDistanceToTarget(currentPosition, targetObject);

  // Afficher les résultats
  Serial.print("Position actuelle : (");
  Serial.print(currentPosition.x);
  Serial.print(", ");
  Serial.print(currentPosition.y);
  Serial.println(")");

  Serial.print("Distance à l'objet cible : ");
  Serial.print(distanceToTarget);
  Serial.println(" mètres");

  static float previousDistance = -1;
  detectRightDirection(distanceToTarget, previousDistance);

  delay(400); // Petite pause pour éviter une surcharge du processeur
}