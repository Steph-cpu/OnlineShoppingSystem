//
// Created by RuizheYang on 2025-12-08.
//

#ifndef ASSIGNMENT2_PRODUCT_H
#define ASSIGNMENT2_PRODUCT_H
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

// Represents available clothing sizes. None is used for size-less products
enum class Size{ XS, S, M, L, XL, None};

// Product category
enum class Category { Men, Women, Kids,Other };

/* Section (sub-category) inside each Category
   Men/Women: Eastern, Western, Other
   Kids: Boys, Girls, Other
   Other: only uses Other
*/
enum class Section { Eastern,Western,Boys,Girls,Other };

// Convert Size enum to string.
static string sizeToString(Size size) {
    switch (size) {
        case Size::XS: return "XS";
        case Size::S: return "S";
        case Size::M: return "M";
        case Size::L: return "L";
        case Size::XL: return "XL";
        case Size::None:return "None";
        default: return "Unknown";
    }
}

// Convert integer index [0..5] to Size (caller should ensure range is valid)
static Size intToSize(int index) { return static_cast<Size>(index);}

// Convert Category enum to string
static string categoryToString(Category cat) {
    switch (cat) {
        case Category::Men: return "Men";
        case Category::Women: return "Women";
        case Category::Kids: return "Kids";
        case Category::Other: return "Other";
        default: return "Unknown";
    }
}

// Convert integer index [0..3] to Category (caller should ensure range is valid)
static Category intToCategory(int index) { return static_cast<Category>(index);}

// Convert Section enum to string
static string sectionToString(Section sec) {
    switch (sec) {
        case Section::Eastern: return "Eastern";
        case Section::Western: return "Western";
        case Section::Boys: return "Boys";
        case Section::Girls: return "Girls";
        case Section::Other: return "Other";
        default: return "Unknown";
    }
}

// Convert integer index [0..4] to Section (caller should ensure range is valid)
static Section intToSection(int index) { return static_cast<Section>(index);}

// Represents a single product with name, id, category, section, size-based stock and price.
class Product {
private:
    int productID;          // unique identifier for the product
    string productName;     // name of the product
    Category category;      // product category (Men/Women/Kids/Other)
    Section section;        // product section(sub-category) within the category
    // Stock for each size index (0..4: XS-XL, 5: None).
    // For size-less(None) products, only None is used and others are 0
    vector<int> sizeStock;
    bool hasSize;           // whether this product uses size XS-XL
    double price;           // unit price
public:
    Product(); // default constructor
    Product(int id, string name, Category cat, Section sec, const vector<int>& stock, double prc); // Constructor with parameters
    // Getter fucntions
    int getProductID() const { return productID;}
    string getProductName() const { return productName;}
    Category getCategory() const {return category;}
    Section getSection() const { return section;}
    const vector<int>& getSizeStock() const { return sizeStock;}
    int getTotalStock() const ; // Get total available stock
    double getPrice() const { return price;}
    bool getHasSize() const { return hasSize; } // whether product has size attributes
    // Setter functions
    void setName(const string& name) { productName=name;}
    void setPrice(double prc){ price=prc;}
    void setCategory(Category cat) { category=cat;}
    void setSection(Section sec) { section=sec;}
    void setSizeStock(const vector<int>& stock) { sizeStock=stock;}
    void setHasSize(bool value) { hasSize = value; } // set size flag
    bool updateStock(Size size, int quantity); // Update stock for a specific size by quantity (can be negative)
};


#endif //ASSIGNMENT2_PRODUCT_H
