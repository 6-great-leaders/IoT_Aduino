#include <ArduinoBLE.h>

// Adresses MAC des 3 balises connues
const char* beacon1_address = "f9:15:ff:1d:3f:6e"; // Remplacez par l'adresse MAC de la première balise
const char* beacon2_address = "d3:0b:c2:cd:eb:30"; // Adresse MAC de la deuxième balise
const char* beacon3_address = "de:8a:30:26:cf:c2"; // Adresse MAC de la troisième balise


// Variables pour stocker les RSSI et distances
int rssi1 = 0, rssi2 = 0, rssi3 = 0;
float distance1 = -1, distance2 = -1, distance3 = -1;

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

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialisation du Bluetooth
  if (!BLE.begin()) {
    Serial.println("Starting BLE module failed!");
    while (1);
  }

  Serial.println("BLE module started!");
}

void loop() {
  Serial.println("Scanning for BLE devices...");
  BLE.scan();

  // Réinitialise les RSSI pour chaque balise
  rssi1 = rssi2 = rssi3 = 0;

  while (BLE.available()) {
    BLEDevice device = BLE.available();
    const char* address = device.address().c_str();


    // Vérifie si l'appareil détecté correspond à une des balises
    if (strcmp(address, beacon1_address) == 0) {
      rssi1 = device.rssi();
      distance1 = calculateDistance(rssi1);
    } else if (strcmp(address, beacon2_address) == 0) {
      rssi2 = device.rssi();
      distance2 = calculateDistance(rssi2);
    } else if (strcmp(address, beacon3_address) == 0) {
      rssi3 = device.rssi();
      distance3 = calculateDistance(rssi3);
    }
  }

  // Affiche les résultats
  Serial.println("Results:");
  Serial.print("Beacon 1 [");
  Serial.print(beacon1_address);
  Serial.print("]: RSSI = ");
  Serial.print(rssi1);
  Serial.print(" dBm, Distance = ");
  Serial.print(distance1);
  Serial.println(" meters");

  Serial.print("Beacon 2 [");
  Serial.print(beacon2_address);
  Serial.print("]: RSSI = ");
  Serial.print(rssi2);
  Serial.print(" dBm, Distance = ");
  Serial.print(distance2);
  Serial.println(" meters");

  Serial.print("Beacon 3 [");
  Serial.print(beacon3_address);
  Serial.print("]: RSSI = ");
  Serial.print(rssi3);
  Serial.print(" dBm, Distance = ");
  Serial.print(distance3);
  Serial.println(" meters");

  Serial.println("----------------------");

  // Pause entre les scans pour éviter une surcharge
  //BLE.stopScan();
  delay(2000);
}