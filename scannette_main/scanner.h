// scanner.h
#ifndef SCANNER_H
#define SCANNER_H

#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

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

void connectToWiFi(const char* ssid, const char* password);

void GetListe(HttpClient httpClient, const char* endpoint_get_liste, ProductList* productList);

void GetArticle(HttpClient httpClient, const char* endpoint_get_article, ProductList* productList, String id);

void MarkAsBought(const String& articleId, ProductList* productList);

void parseJsonToProductList(const String& jsonInput, ProductList* productList);

#endif