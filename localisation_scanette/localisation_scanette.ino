#include <ArduinoBLE.h>

// Adresses MAC des balises cibles
const char* targetAddresses[3] = {
  "f9:15:ff:1d:3f:6e", // Adresse MAC de la balise 1
  "d3:0b:c2:cd:eb:30", // Adresse MAC de la balise 2
  "de:8a:30:26:cf:c2"  // Adresse MAC de la balise 3
};

// Positions fixes des balises (en mètres)
struct Position {
  float x;
  float y;
  float z;
};

Position targetObject = {3.0, 2.0, 0.0};

Position beacons[3] = {
  {0.0, 0.0, 0.0},   // Balise 1
  {5.0, 0.0, 0.0},   // Balise 2
  {2.5, 4.33, 0.0}   // Balise 3 (forme un triangle équilatéral)
};

// Tableaux pour stocker les périphériques connectés
BLEDevice connectedDevices[3];
bool isConnected[3] = { false, false, false };
int rssi[3] = {0, 0, 0};
float distances[3] = {-1, -1, -1};

#define NUM_MEASUREMENTS 15 // Nombre de mesures pour la moyenne glissante

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


Position calculatePosition() {
  float x1 = beacons[0].x, y1 = beacons[0].y;
  float x2 = beacons[1].x, y2 = beacons[1].y;
  float x3 = beacons[2].x, y3 = beacons[2].y;

  float r1 = distances[0]; // Distance à la balise 1
  float r2 = distances[1]; // Distance à la balise 2
  float r3 = distances[2]; // Distance à la balise 3

  // Résolution des équations de trilatération
  float A = 2 * (x2 - x1);
  float B = 2 * (y2 - y1);
  float C = r1 * r1 - r2 * r2 - x1 * x1 - y1 * y1 + x2 * x2 + y2 * y2;

  float D = 2 * (x3 - x2);
  float E = 2 * (y3 - y2);
  float F = r2 * r2 - r3 * r3 - x2 * x2 - y2 * y2 + x3 * x3 + y3 * y3;

  // Résolution pour x et y
  float x = (C * E - F * B) / (E * A - B * D);
  float y = (C * D - A * F) / (B * D - A * E);

  return {x, y, 0.0}; // Retourne la position estimée
}

float calculateDistanceToTarget(Position pos1, Position pos2) {
  return sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2) + pow(pos1.z - pos2.z, 2));
}

// Fonction pour convertir le RSSI en distance approximative
float calculateDistance(int rssi) {
  const int txPower = -59; // Ajustez selon votre balise
  if (rssi == 0) {
    return -1.0; // Distance inconnue
  }

  float ratio = rssi * 1.0 / txPower;
  if (ratio < 1.0) {
    return pow(ratio, 10);
  } else {
    return (0.89976) * pow(ratio, 7.7095) + 0.111;
  }
}

int calculateAverageRSSI(int beaconIndex) {
  int sum = 0;
  int count = historyFilled[beaconIndex] ? NUM_MEASUREMENTS : historyIndex[beaconIndex];

  for (int i = 0; i < count; i++) {
    sum += rssiHistory[beaconIndex][i];
  }

  return (count > 0) ? sum / count : 0; // Évite une division par zéro
}

// fonction qui va détecter et tenter de se connecter a un appareil
void findDevices() {
  for (int attempts = 0; attempts < 5; attempts++) {
    //Serial.print("Scanning for new devices...");
    if (isConnected[0] && isConnected[1] && isConnected[2]) {
      //Serial.println("All devices connected !");
      break;
    }
    BLE.scan();
    BLEDevice device = BLE.available();
    if (device) {
      for (int i = 0; i < 3; i++) {
        if (!isConnected[i] && strcmp(device.address().c_str(), targetAddresses[i]) == 0) {
          Serial.print("Connexion à la balise ");
          Serial.println(targetAddresses[i]);

          BLE.stopScan();
          if (device.connect()) {
            Serial.println("Connexion réussie!");
            connectedDevices[i] = device;
            isConnected[i] = true;
          } else {
            Serial.println("Échec de connexion.");
          }
        }
      }
    }
  }
}

// fonction qui recupere les informations RSSI 
void getCoordinates() {
  for (int i = 0; i < 3; i++) {
    if (isConnected[i]) {
      BLEDevice& device = connectedDevices[i];

      // Vérifier que la connexion est toujours active
      if (!device.connected()) {
        Serial.print("Perte de connexion avec la balise ");
        Serial.println(targetAddresses[i]);
        isConnected[i] = false;
        continue;
      }
      

      int currentRSSI = device.rssi();

      // Étape 1 : Prédiction
      float P_prior = P[i] + Q[i]; // Augmenter l'incertitude
      float K = P_prior / (P_prior + R[i]); // Calcul du gain de Kalman

      // Étape 2 : Mise à jour
      RSSI_estimate[i] = RSSI_estimate[i] + K * (currentRSSI - RSSI_estimate[i]);
      P[i] = (1 - K) * P_prior;

      // Convertir le RSSI lissé en distance
      distances[i] = calculateDistance(RSSI_estimate[i]);

      // Afficher les résultats
      Serial.print("Beacon ");
      Serial.print(i);
      Serial.print(" [");
      Serial.print(targetAddresses[i]);
      Serial.print("]: RSSI (Kalman) = ");
      Serial.print(RSSI_estimate[i]);
      Serial.print(" dBm, Distance = ");
      Serial.print(distances[i]);
      Serial.println(" meters");
    }
  }
}

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

  findDevices();
  getCoordinates();

  Position currentPosition = calculatePosition();

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

  // Détecter si on se rapproche de l'objet
  static float previousDistance = -1;
  if (previousDistance > 0) {
    if (distanceToTarget < previousDistance) {
      Serial.println("Vous vous rapprochez de l'objet !");
    } else if (distanceToTarget > previousDistance) {
      Serial.println("Vous vous éloignez de l'objet.");
    }
  }
  previousDistance = distanceToTarget;


  delay(400); // Petite pause pour éviter une surcharge du processeur
}