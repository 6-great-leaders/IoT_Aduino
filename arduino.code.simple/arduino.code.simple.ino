#include "localisation.h"
#include "front.h"
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// Informations de votre réseau WiFi
//const char* ssid = "iPhone de Matteo";   // Nom du réseau WiFi
const char* ssid = "Coucou";
const char* password = "coucoucou";       // Mot de passe du réseau WiFi

// Adresse IP publique du serveur
// const char* server = "34.69.136.64";    // Adresse IP fixe
const char* server = "146.148.5.29";
// const int port = 80;                   // Port du serveur
const int port = 4000;
const char* endpoint_get_liste = "/scanner/articles/1/1"; // Chemin de l'API
const char* endpoint_get_article = "/scanner/articles/1/";
String buffer = "";

WiFiClient wifiClient; // Client WiFi
HttpClient httpClient(wifiClient, server, port);

// Structure pour représenter un produit
struct Product {
    String id;
    String name;
    String article_id;
    bool suggested;
    bool scanned; // Remplace "scanned" pour correspondre à votre structure
    String positionX;
    String positionY;

    // Constructeur par défaut
    Product() : id(""), name(""), article_id(""), suggested(false), scanned(false), positionX("0"), positionY("0") {}
};

// Structure représentant une liste dynamique de produits
struct ProductList {
    Product* products;   // Tableau dynamique de produits
    size_t size;         // Nombre d'éléments dans la liste
    size_t capacity;     // Capacité actuelle du tableau
    size_t index;        // index qui montre l'article recherché actuellement

    // Constructeur pour initialiser la liste
    ProductList() : products(nullptr), size(0), capacity(0), index(0) {}

    // Méthode pour ajouter un produit à la liste
    void addProduct(const Product& product) {
        if (size == capacity) {
            // Augmenter la capacité (doublement pour efficacité)
            size_t newCapacity = (capacity == 0) ? 1 : capacity * 2;
            Product* newProducts = new Product[newCapacity];
            
            // Copier les produits existants
            for (size_t i = 0; i < size; i++) {
                newProducts[i] = products[i];
            }
            delete[] products; // Libérer l'ancien tableau
            products = newProducts;
            capacity = newCapacity;
        }
        // Ajouter le produit
        products[size] = product;
        size++;
    }

    // Méthode pour obtenir un produit par index
    Product* getProduct(size_t index) const {
        if (index < size) {
            return &products[index];
        }
        return nullptr;
    }

    // Destructeur pour libérer la mémoire
    ~ProductList() {
        delete[] products;
    }
};

ProductList productList;

// Fonction pour se connecter au WiFi
void connectToWiFi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnecté au WiFi !");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
}

// Fonction pour effectuer un appel GET à l'URL fixe
void GetListe() {
  Serial.println("Envoi de la requête GET Liste ...");

  // Construire la requête HTTP
  httpClient.beginRequest();
  httpClient.get(endpoint_get_liste);
  httpClient.endRequest();

  // Lire la réponse du serveur
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();
  Serial.print("Statut HTTP : ");
  Serial.println(statusCode);
  if (statusCode == 200) {
    Serial.println("Réponse reçue :");
    Serial.println(response);

    // Mettre à jour la liste de produits
    parseJsonToProductList(response, productList);

    // Afficher les produits mis à jour
    for (size_t i = 0; i < productList.size; i++) {
      Product p = productList.products[i];
      Serial.println("Produit :");
      Serial.println("ID: " + p.id);
      Serial.println("Nom: " + p.name);
      Serial.println("Article ID: " + p.article_id);
      Serial.println("Suggéré: " + String(p.suggested));
      Serial.println("Acheté: " + String(p.scanned));
      Serial.println("Position: (" + p.positionX + ", " + p.positionY + ")");
      Serial.println();
    }
  } else {
    Serial.println("Erreur lors de l'appel API.");
  }
}

// Fonction pour effectuer un appel GET à l'URL fixe
void GetArticle(String id) {
  Serial.println("Envoi de la requête GET Article ...");

  // Construire la requête HTTP
  httpClient.beginRequest();
  httpClient.post(endpoint_get_article + id);
  httpClient.endRequest();

  // Lire la réponse du serveur
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  // Afficher le statut et la réponse
  Serial.print("Statut HTTP : ");
  Serial.println(statusCode);
  Serial.print("Réponse : ");
  Serial.println(response);
  MarkAsBought(id);
}

size_t findNextItem() {
  for (size_t i = productList.index; i < productList.size; i++) {
    if (productList.products[i].scanned == false) {
      return i;
    }
  }
}

// Méthode pour remplir la liste de produits à partir d'un JSON


void MarkAsBought(const String& articleId) {
    bool found = false;

    for (size_t i = 0; i < productList.size; i++) {
        if (productList.products[i].article_id == articleId) {
            productList.products[i].scanned = "true";
            found = true;
            Serial.println("Produit marqué comme acheté :");
            Serial.println("ID: " + productList.products[i].id);
            Serial.println("Nom: " + productList.products[i].name);
            break;
        }
    }

    if (!found) {
        Serial.println("Article non trouvé avec l'article_id : " + articleId);
    }
};

void parseJsonToProductList(const String& jsonInput, ProductList& productList) {
    StaticJsonDocument<2048> doc; // Ajustez la taille si nécessaire
    DeserializationError error = deserializeJson(doc, jsonInput);

    // Vérifiez si la désérialisation a réussi
    if (error) {
        Serial.print("Erreur lors du parsing JSON : ");
        Serial.println(error.c_str());
        return;
    }

    // Vérifiez que le JSON est un tableau
    if (!doc.is<JsonArray>()) {
        Serial.println("Erreur : Le JSON n'est pas un tableau !");
        return;
    }

    // Parcourir les objets dans le tableau
    for (JsonObject obj : doc.as<JsonArray>()) {
        Product product;

        // Extraire et assigner les champs
        product.id = String(obj["id"].as<int>());
        product.name = obj["name"] | "";
        product.article_id = String(obj["article_id"].as<int>());
        product.suggested = obj["suggested"] | false;
        product.scanned = obj["scanned"] | false;
        product.positionX = String(obj["x"].as<int>());
        product.positionY = String(obj["y"].as<int>());

        // Ajouter le produit à la liste
        productList.addProduct(product);
    }
}

// front
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

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

bool started = false;
bool rightDirection = false;

void setup() {
  Serial.begin(9600);  // Initialisation du port USB (Serial) pour le moniteur série
  while (!Serial);     // Attente de la connexion avec le moniteur série
  
  // initialisation de la localisation
  if (!BLE.begin()) {
    Serial.println("Erreur d'initialisation BLE!");
  }
  Serial.println("Initialisation réussie. Scannage des périphériques...");
  while (!findDevices(isConnected, connectedDevices, targetAddresses)) {
    findDevices(isConnected, connectedDevices, targetAddresses);
    delay(400);
  }
  BLE.end();
  delay(200);

  // Initialisation ecran
  tft.begin();
  tft.fillScreen(ILI9341_WHITE); // Fond blanc pour commencer

  Serial.println("Connexion au WiFi...");
  Serial1.begin(9600);
  while (!Serial1);
  connectToWiFi(); // Configuration de Serial1 sur les broches appropriées
  Serial.println("Attente des données sur Serial1...");
  Serial.println("Liste de course intialisée");
  buffer = "";

  // draw welcome page
  drawWelcomeScreen(tft);
}

void loop() {
  // loop localisation
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
  rightDirection = detectRightDirection(distanceToTarget, previousDistance);
  Serial.println(rightDirection);

  // if parcours, update la fleche selon si on va dans le bon sens
  if (productList.capacity != 0) {
    drawHeader(tft);
    drawArticle(tft, productList.products[productList.index].name.c_str());
    drawDistance(tft, distanceToTarget);
    if (rightDirection == -1) {
      rotateArrowToAngle(tft, 270, 270);
    }
    else {
      rotateArrowToAngle(tft, 90, 90);
    }
  }
  while (Serial1.available()) {
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
          GetListe();
          if (productList.capacity != 0) {
            // si j'ai des produits dans ma liste de courses, je peux commencer mon parcours

            productList.index = findNextItem();

            // calculer la nouvelle position
            articlePosition[0] = productList.products[productList.index].positionX.toFloat();
            articlePosition[1] = productList.products[productList.index].positionY.toFloat();

            // calculer la nouvelle distance
            

            tft.fillScreen(ILI9341_WHITE); // Efface l'écran pour éviter les anciens contenus
            drawHeader(tft);
            drawArticle(tft, productList.products[productList.index].name.c_str());
            drawDirectionCircle(tft);
            drawDistance(tft, distanceToTarget);
            drawProgressBar(tft, productList.size - productList.index, 0);
            drawFooter(tft);
            rotateArrowToAngle(tft, 0, 0);
          }
        } else if (fct == "get_article") {
          if (productList.capacity != 0) {
            GetArticle(id);

            productList.index = findNextItem();

            if (productList.index == productList.size) {
              Serial.println("j'ai fini mes courses");
            }

            // mettre a jour la barre de progression
            // si les courses sont finies, ecran de fin ? 
            drawProgressBar(tft, productList.size - productList.index,  productList.index / productList.size * 100);

            // calculer la nouvelle position
            articlePosition[0] = productList.products[productList.index].positionX.toFloat();
            articlePosition[1] = productList.products[productList.index].positionY.toFloat();

            tft.fillScreen(ILI9341_WHITE); // Efface l'écran pour éviter les anciens contenus
            drawHeader(tft);
            drawArticle(tft, productList.products[productList.index].name.c_str());
            drawDistance(tft, distanceToTarget);
            drawDirectionCircle(tft);
            drawProgressBar(tft, productList.size - productList.index, 0);
            drawFooter(tft);
            rotateArrowToAngle(tft, 0, 0);
          } // else if checkout ? 
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

  delay(400);
}