//
// Created by RuizheYang on 2025-12-08.
//

#include "ShoppingCart.h"

#include <fstream>
#include <iostream>
#include <numeric>
using namespace std;

// read size from user input and check if it is valid
Size ShoppingCart::inputSize() {
    // run infinite loop until valid input is received
    while (true) {
        cout<<"Please enter size (0: XS, 1: S, 2: M, 3: L, 4: XL): ";
        int idx;
        if (!(cin >> idx)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');     // clear invalid input
            cout << "Invalid input, should between 0-4." << endl;
            continue;
        }
        // check if index is within valid range
        if (idx < 0 || idx > 4) {
            cout << "Invalid input, should between 0-4." << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return static_cast<Size>(idx);
    }
}

// read quantity from user input and check if it is valid
int ShoppingCart::inputQuantity() {
    // run infinite loop until valid input is received
    while (true) {
        cout<<"Please enter quantity: ";
        int quantity;
        if (!(cin >> quantity)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');    // clear invalid input
            cout << "Invalid input, should be a positive integer." << endl;
            continue;
        }
        // check if quantity is positive
        if (quantity <= 0) {
            cout << "Quantity must be positive." << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return quantity;
    }
}

// check if the productID(product exists) and quantity are valid
bool ShoppingCart::isValid(int productID, Size size,int quantity, const ProductManager &pm) {
    // check if quantity is positive
    if (quantity<=0) {
        cout<<"Quantity must be positive."<<endl;
        return false;
    }
    const Product* p=pm.getProduct(productID);  // get all products from ProductManager
    // check if product exists
    if (p==nullptr) {
        cout<<"Product ID "<<productID<<" not found."<<endl;
        return false;
    }
    bool hasSize = p->getHasSize(); // check whether product has size attributes
    int sizeIdx=static_cast<int>(size);
    const vector<int>& stock=p->getSizeStock();     // get size stock vector
    // check if size is valid based on hasSize flag
    if (!hasSize) {
        // size-less product: only None slot is valid
        int noneStock = stock[static_cast<int>(Size::None)];    // get None stock
        if (size != Size::None) {
            cout << "Invalid size: this product has no size, only None is allowed." << endl;
            return false;
        }
        if (quantity>noneStock) {
            cout<<"Insufficient stock. Available: "<<noneStock<<endl;
            return false;
        }
        return true;
    }
    // sized product: use XS-XL only, None not allowed
    if (size == Size::None) {
        cout << "Invalid size: this product uses sizes XS-XL, not None." << endl;
        return false;
    }
    if (sizeIdx<0||sizeIdx>=static_cast<int>(stock.size())-1) {
        cout<<"Invalid size."<<endl;
        return false;
    }
    if (quantity>stock[sizeIdx]) {
        cout<<"Insufficient stock for size "<<sizeToString(size)<<". Available: "<<stock[sizeIdx]<<endl;
        return false;
    }
    return true;
}

// add item to cart
void ShoppingCart::addItem(int productID, const ProductManager &pm) {
    const Product* p=pm.getProduct(productID);
    // check if product exists
    if (p==nullptr) {
        cout<<"Product ID "<<productID<<" not found."<<endl;
        return;
    }
    const vector<int>& stock=p->getSizeStock();
    // check if product is out of stock
    if (p->getTotalStock()==0) {
        cout<<"Product ID "<<productID<<"Product name:"<<p->getProductName()<<" is out of stock."<<endl;
        return;
    }
    Size size;
    int quantity=inputQuantity();   // input quantity from user
    // determine size based on whether product has size attributes
    if (p->getHasSize()) {
        size=inputSize();
    }else {
        size=Size::None;
    }
    if (!isValid(productID,size,quantity,pm)) return;   // check if the product id, size and quantity are valid
    auto& vec=items[productID];
    if (vec.size()<6) vec.resize(6,0); // ensure vector has 6 elements
    vec[static_cast<int>(size)]+=quantity;  // add quantity to the specified size
    // output new item info in cart
    cout<<"Add successfully to cart, Product ID: "<<productID
        <<", Name: "<<p->getProductName()
        <<", Size: "<<sizeToString(size)
        <<", Quantity: "<<quantity<<endl;
}


// update item quantity in cart
void ShoppingCart::updateItem(int productID, const ProductManager &pm) {
    auto it=items.find(productID);
    // check if item exists in cart
    if (it==items.end()) {
        cout<<"Item not found in cart."<<endl;
        return;
    }
    const Product* p=pm.getProduct(productID);
    // check if product exists
   if (p==nullptr) {
       cout<<"Product ID "<<productID<<" not found."<<endl;
       return;
   }
   const vector<int>& stock=p->getSizeStock();
    // check if product is out of stock
    if (p->getTotalStock()==0) {
        cout<<"Product ID "<<productID<<"Product name:"<<p->getProductName()<<" is out of stock."<<endl;
        return;
    }
    Size size;
    int quantity=inputQuantity();   // input new quantity from user
    // determine size based on whether product has size attributes
    if (p->getHasSize()) {
        size=inputSize();
    }else {
        size=Size::None;
    }
    // check if the product id, size and quantity are valid
    if (!isValid(productID,size,quantity,pm)) return;
    items[productID][static_cast<int>(size)]=quantity;  // update new quantity in cart
    // output info
    cout<<"Update successfully in cart, Product ID: "<<productID
        <<", Name: "<<p->getProductName()
        <<", Size: "<<sizeToString(size)
        <<", New Quantity: "<<quantity<<endl;
}


// remove item from cart if it exists
void ShoppingCart::removeItem(int productID) {
    auto it=items.find(productID);
    // check if item exists in cart
    if (it==items.end()) {
        cout<<"No such item in cart, product ID:"<<productID<<endl;
        return;
    }
    // erase all sizes for the product
    items.erase(it);
    cout<<"Item removed from cart, product ID:"<<productID<<endl;
}

// calculate total price of items in cart
double ShoppingCart::calculateTotal(const ProductManager &pm) const {
    double total=0.0;
    for (const auto& pair:items) {
        // get productID and stock vector
        int productID=pair.first;
        const vector<int>& stock=pair.second;
        const Product* p=pm.getProduct(productID);
        // check if product exists
        if (p==nullptr) {
            cout<<"Product ID "<<productID<<" not found."<<endl;
            continue;
        }
        double price=p->getPrice(); // get unit price
        int quantity=accumulate(stock.begin(),stock.end(),0); // sum up quantities(stock vector) across all sizes
        total+=price*quantity;  // update total price
    }
    return total;
}

// display all items in cart with details
void ShoppingCart::displayCart(const ProductManager &pm) const {
    // check if cart is empty
    if (items.empty()) {
        cout<<"Your shopping cart is empty."<<endl;
        return;
    }
    // traverse to display each item
    cout<<"Items in your shopping cart:"<<endl;
    for (const auto& pair:items) {
        int productID=pair.first;
        const vector<int>& stock=pair.second;
        const Product* p=pm.getProduct(productID);
        // check if product exists
        if (p==nullptr) {
            cout<<"Product ID "<<productID<<" not found."<<endl;
            continue;   // skip current iteration
        }
        // display product info(id,name,unit price)
        cout<<"Product ID: "<<productID
            <<", Name: "<<p->getProductName()
            <<", Unit Price: "<<p->getPrice()<<endl;
        // display quantity of each size and subtotal
        for (int i=0;i<stock.size();++i) {
            if (stock[i]>0) {
                cout<<"Size: "<<sizeToString(static_cast<Size>(i))
                    <<", Quantity: "<<stock[i]<<", Subtotal: "<<p->getPrice()*stock[i]<<endl;
            }
        }
        cout<<"============================"<<endl;
    }
    cout<<"Total Price: "<<calculateTotal(pm)<<endl;    // display total price
}

// clear all items in cart
void ShoppingCart::clearCart() {
    items.clear();
}

// save cart items to file
bool ShoppingCart::saveToFile(const string &filename) const {
    ofstream file(filename);
    // check if file opened successfully
    if (!file.is_open()) {
        cout<<"The file can not be opened "<< endl;
        return false;
    }
    // format: productID q0 q1 q2 q3 q4 q5
    file<<items.size()<<endl; // first line is the number of items
    for (const auto& pair : items) {
        int productID = pair.first;
        const vector<int>& vec = pair.second;
        file << productID;
        // write quantities for all 6 sizes
        for (int i = 0; i < 6; ++i) {
            int q = (i < static_cast<int>(vec.size())) ? vec[i] : 0;
            file << ' ' << q;
        }
        file << endl;
    }
    file.close();
    return true;
}

// load cart items from file
bool ShoppingCart::loadFromFile(const string &filename) {
    ifstream file(filename);
    // check if file opened successfully
    if (!file.is_open()) {
        cout<<"The file can not be opened "<< endl;
        return false;
    }
    clearCart();    // clear existing items
    size_t count;
    file>>count;
    for (size_t i = 0; i < count; ++i) {
        int productID;
        if (!(file >> productID)) {
            cout << "read productID failed" << endl;
            return false;
        }
        vector<int> vec(6, 0);
        // read quantities for all 6 sizes
        for (int j = 0; j < 6; ++j) {
            if (!(file >> vec[j])) {
                cout << "read quantity failed" << endl;
                return false;
            }
        }
        items[productID] = vec; // store in items map
    }
    return true;
}
