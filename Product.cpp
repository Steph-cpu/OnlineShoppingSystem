//
// Created by RuizheYang on 2025-12-08.
//

#include "Product.h"
#include <numeric>
using namespace std;

// Constructor:initialize Product with default values
Product::Product() {
    productID=0;
    productName="";
    category=Category::Men;
    section=Section::Eastern;
    price=0.0;
    sizeStock.resize(6,0); // initialize stock for 6 sizes to 0
    hasSize = false;       // default: no size attributes
}

// Constructor with parameters: initialize Product with given values
Product::Product(int id, string name, Category cat, Section sec, const vector<int> &stock, double prc) {
    productID=id;
    productName=name;
    category=cat;
    section=sec;
    sizeStock=stock;
    price=prc;
    if (sizeStock.size()<6) sizeStock.resize(6,0);
    // auto-detect hasSize from stock data: if any XS-XL has stock, treat as sized
    bool anySized = false;
    for (int i = 0; i < 5; ++i) {
        if (sizeStock[i] != 0) { anySized = true; break; }
    }
    int noneStock = sizeStock[static_cast<int>(Size::None)];
    hasSize = anySized || (noneStock == 0); // if only None used and non-zero, then size-less
}

// Get total stock across all sizes.
int Product::getTotalStock() const {
    if (!hasSize) {
        // size-less product: only use None slot
        return sizeStock[static_cast<int>(Size::None)];
    }
    // sized product: sum XS-XL and ignore None
    return accumulate(sizeStock.begin(), sizeStock.begin() + 5, 0);
}

// Update stock for a specific size by adding quantity (can be negative to reduce stock)
bool Product::updateStock(Size size, int quantity) {
    int index = static_cast<int>(size); // convert Size enum to index
    // check if index is valid
    if (index < 0 || index >= sizeStock.size()) {
        return false;
    }
    sizeStock[index] += quantity;   // update stock
    // ensure stock does not go negative
    if (sizeStock[index] < 0) {
        sizeStock[index] -= quantity; // revert change
        return false;
    }
    return true;
}
