//
// Created by RuizheYang on 2025-12-08.
//

#ifndef ASSIGNMENT2_SHOPPINGCART_H
#define ASSIGNMENT2_SHOPPINGCART_H
#include "ProductManager.h"
#include <unordered_map>
#include <vector>
#include <string>
using namespace std;

// user's shopping cart: stores selected products and quantities per size.
class ShoppingCart {
private:
    unordered_map<int,vector<int> > items; // items[productID][sizeIndex], quantity per size
    static Size inputSize(); // read size from user input
    static int inputQuantity(); // read quantity from user input
    bool isValid(int productID, Size size, int quantity, const ProductManager &pm); // check if parameters are valid

public:
    void addItem(int productID, const ProductManager& pm);  // Add an item to the cart
    void updateItem(int productID, const ProductManager& pm);   // update item quantity in cart
    void removeItem(int productID);     // remove item from cart
    double calculateTotal(const ProductManager& pm) const;      // calculate total price of items in cart
    void displayCart(const ProductManager& pm) const;   // display all items in cart
    void clearCart();   // remove all items from the cart
    // Expose internal items map (read-only) for other components (e.g. transaction).
    const unordered_map<int,vector<int>>& getItems() const { return items; }
    bool saveToFile(const string &filename) const;  // save cart to file
    bool loadFromFile(const string &filename);  // load cart from file
    static string getShoppingCartFileName(int userID) {return "cart_" + to_string(userID) + ".txt"; }   // Generate cart filename based on userID
};

#endif //ASSIGNMENT2_SHOPPINGCART_H
