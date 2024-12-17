#include "localisation.h"

Position calculatePosition(Position beacons[], float distances[]) {
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


int calculateAverageRSSI(int beaconIndex, bool historyFilled[], int historyIndex[], int rssiHistory[][NUM_MEASUREMENTS]) {
  int sum = 0;
  int count = historyFilled[beaconIndex] ? NUM_MEASUREMENTS : historyIndex[beaconIndex];

  for (int i = 0; i < count; i++) {
    sum += rssiHistory[beaconIndex][i];
  }

  return (count > 0) ? sum / count : 0; // Évite une division par zéro
}

// fonction qui va détecter et tenter de se connecter a un appareil
bool findDevices(bool isConnected[], BLEDevice connectedDevices[], const char* targetAddresses[]) {
  for (int attempts = 0; attempts < 5; attempts++) {
    Serial.print("Scanning for new devices...");
    if (isConnected[0] && isConnected[1] && isConnected[2]) {
      Serial.println("All devices connected !");
      return true;
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
  return false;
}

// fonction qui recupere les informations RSSI 
void getCoordinates(bool isConnected[], float RSSI_estimate[], float P[], const float Q[], const float R[], float distances[], BLEDevice connectedDevices[]) {
  for (int i = 0; i < 3; i++) {
    if (isConnected[i]) {
      BLEDevice& device = connectedDevices[i];

      // Vérifier que la connexion est toujours active
      if (!device.connected()) {
        Serial.print("Perte de connexion avec la balise ");
        Serial.println(i);
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
    }
  }
}

bool detectRightDirection(float distanceToTarget, float &previousDistance) {
  if (previousDistance > 0) { // Vérifie si une distance précédente existe
    if (distanceToTarget < previousDistance) {
      Serial.println("Vous vous rapprochez de l'objet !");
      previousDistance = distanceToTarget;
      return true;
    } else if (distanceToTarget > previousDistance) {
      Serial.println("Vous vous éloignez de l'objet.");
      previousDistance = distanceToTarget;
      return false;
    }
  }
  // Mettre à jour la distance précédente
  previousDistance = distanceToTarget;
  return true;
}
