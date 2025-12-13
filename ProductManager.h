//
// Created by RuizheYang on 2025-12-08.
//

#ifndef ASSIGNMENT2_PRODUCTMANAGER_H
#define ASSIGNMENT2_PRODUCTMANAGER_H

#include "Product.h"
#include <unordered_map>
#include <vector>
using namespace std;

// Manages all products in the system
class ProductManager {
private:
    int nextProductID;  // next available product ID for new products

    // products[categoryIndex][sectionIndex][productID] = Product
    // categoryIndex: 0..3 for Men/Women/Kids/Other
    // sectionIndex: 0..2 mapped by getSectionIndex for each category
    vector<vector<unordered_map<int, Product>>> products;
    unordered_map<int,int> map; // Maps productID to its category index.
    unordered_map<string,int> nameMap;  // Maps unique product name to productID for name search and duplicate check
    int getCategoryIndex(Category cat) const;   // Convert Category enum to container index
    int getSectionIndex(Category cat, Section sec) const;   // Convert (Category, Section) pair to the internal section index [0..2]
public:
    ProductManager();   // Default constructor:Initialize empty manager with 4 categories and 3 sections per category
    int getProductID(const string &name);   // Get productID by product name, or -1 if not found
    int addProduct(const string &name, Category cat, Section sec, double price);    // Add a new product with user-interaction for size stock input inside function
    Product* getProduct(int productID);
    const Product* getProduct(int productID) const;
    bool removeProduct(int productID);  // Remove product by ID.
    // Function overload
    bool updateProduct(int productID, Size size, int newStock); // Update stock for a specific size of a product
    bool updateProduct(int productID, double newPrice);     // Update price of a product.
    bool updateProduct(int productID, const string &newName);   // Update name of a product
    bool updateProduct(int productID, Category newCat, Section newSec); // Update category and section of a product

    // Display product information
    void displaySingleProduct(int productID) const;     // Display a single product
    void displayBySection(Category cat, Section sec) const;     // Display all products in a given section
    void displayByCategory(Category cat) const; // Display all products in a given category
    void displayAllProducts() const;     // Display all products

    bool saveToFile(const string &filename) const;  // Save all products to file
    bool loadFromFile(const string &filename);  // Load products from file
};


#endif //ASSIGNMENT2_PRODUCTMANAGER_H
