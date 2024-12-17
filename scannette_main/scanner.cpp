#include "scanner.h"

// Fonction pour se connecter au WiFi
void connectToWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnecté au WiFi !");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
}
/*
// Fonction pour effectuer un appel GET à l'URL fixe
void GetListe(HttpClient httpClient, const char* endpoint_get_liste, ProductList* productList) {
  Serial.println("Envoi de la requête GET Liste ...");

  // Construire la requête HTTP
  httpClient.beginRequest();
  httpClient.get(endpoint_get_liste);
  //httpClient->get("/scanner/articles/1/1");
  httpClient.endRequest();

  Serial.println("ici");

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
    for (size_t i = 0; i < productList->size; i++) {
      Product p = productList->products[i];
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
*/
// Fonction pour effectuer un appel GET à l'URL fixe
void GetArticle(HttpClient httpClient, const char* endpoint_get_article, ProductList* productList, String id) {
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
  MarkAsBought(id, productList);
}

// Méthode pour remplir la liste de produits à partir d'un JSON


void MarkAsBought(const String& articleId, ProductList* productList) {
    bool found = false;

    for (size_t i = 0; i < productList->size; i++) {
        if (productList->products[i].article_id == articleId) {
            productList->products[i].scanned = "true";
            found = true;
            Serial.println("Produit marqué comme acheté :");
            Serial.println("ID: " + productList->products[i].id);
            Serial.println("Nom: " + productList->products[i].name);
            break;
        }
    }

    if (!found) {
        Serial.println("Article non trouvé avec l'article_id : " + articleId);
    }
};

/*void parseJsonToProductList(const String& jsonInput, ProductList* productList) {
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
        productList->addProduct(product);
    }
}*/