#include "scanner.h"

// Informations de votre réseau WiFi
//const char* ssid = "iPhone de Matteo";   // Nom du réseau WiFi
const char* ssid = "Coucou";
const char* password = "coucoucou";       // Mot de passe du réseau WiFi

// Adresse IP publique du serveur
const char* server = "192.168.1.46";

// Port du serveur
const int port = 3444;

const char* endpoint_get_liste = "/scanner/articles/1/1"; // Chemin de l'API
const char* endpoint_get_article = "/scanner/article/";

String buffer = "";

WiFiClient wifiClient; // Client WiFi
HttpClient httpClient(wifiClient, server, port);

ProductList productList;

void setup() {
  Serial.begin(9600);  // Initialisation du port USB (Serial) pour le moniteur série
  while (!Serial);     // Attente de la connexion avec le moniteur série
  Serial.println("Connexion au WiFi...");
  Serial1.begin(9600);
  while (!Serial1);
  connectToWiFi(ssid, password); // Configuration de Serial1 sur les broches appropriées
  Serial.println("Attente des données sur Serial1...");
  Serial.println("Liste de course intialisée");
  buffer = "";
}

void loop() {
  while (Serial1.available()) {
    char data = Serial1.read();
    Serial.print(data);

    if (data != '\\') {
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
}

