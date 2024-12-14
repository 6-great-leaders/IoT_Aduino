#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// Informations de votre réseau WiFi
//const char* ssid = "iPhone de Matteo";   // Nom du réseau WiFi
const char* ssid = "Freebox-C17C5D";
const char* password = "3z69nqmfkzq2b5rfqrbr37";       // Mot de passe du réseau WiFi

// Adresse IP publique du serveur
// const char* server = "34.69.136.64";    // Adresse IP fixe
const char* server = "192.168.1.46";
// const int port = 80;                   // Port du serveur
const int port = 3444;
const char* endpoint_get_liste = "/scanner/articles/1/1"; // Chemin de l'API
const char* endpoint_get_article = "/scanner/article/";
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

    // Constructeur pour initialiser la liste
    ProductList() : products(nullptr), size(0), capacity(0) {}

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

void setup() {
  Serial.begin(9600);  // Initialisation du port USB (Serial) pour le moniteur série
  while (!Serial);     // Attente de la connexion avec le moniteur série
  Serial.println("Connexion au WiFi...");
  Serial1.begin(9600);
  while (!Serial1);
  connectToWiFi(); // Configuration de Serial1 sur les broches appropriées
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
          GetListe();
        } else if (fct == "get_article") {
          GetArticle(id);
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
  httpClient.get(endpoint_get_article + id);
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