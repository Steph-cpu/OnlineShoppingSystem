//
// Created by RuizheYang on 2025-12-08.
//

#include "ProductManager.h"
#include <fstream>
#include <iostream>
#include <limits>
using namespace std;

// Initialize ProductManager with empty product containers.
ProductManager::ProductManager() {
    nextProductID=1;
    products.resize(4); // 4 categories：Men, Women, Kids, Other
    for (auto& category:products) {
        category.resize(3); // 3 sections each: two specific + Other
    }
}

// Map Category enum to internal index [0..3].
int ProductManager::getCategoryIndex(Category cat) const {
    return static_cast<int>(cat);
}

// Map (Category, Section) to internal section index [0..2].
// Kids: Boys/Girls/Other -> 0/1/2
// Other: always uses slot 2 (Other)
// Men/Women: Eastern/Western/Other -> 0/1/2
int ProductManager::getSectionIndex(Category cat, Section sec) const {
    // Kids: Boys / Girls / Other
    if (cat == Category::Kids) {
        if (sec == Section::Boys) return 0;
        else if (sec == Section::Girls) return 1;
        else return 2; // Other
    }
    // Other: only use Other section
    if (cat == Category::Other) {
        return 2;
    }
    // Men / Women: Eastern / Western / Other
    if (sec == Section::Eastern) return 0;
    else if (sec == Section::Western) return 1;
    else return 2; // Other
}

// Get productID by product name (or -1 if not found).
int ProductManager::getProductID(const string &name) {
    auto it=nameMap.find(name);
    // check if product exists
    if (it==nameMap.end()) {
        cout<<"No product found with name: "<<name<<endl;
        return -1;  // In user class, method should check -1 for not found
    }
    return it->second;  // return productID
}

// Add new product and interactively read size stock from user and return productID if added successfully
int ProductManager::addProduct(const string &name, Category cat, Section sec, double price) {
    auto it=nameMap.find(name);
    // check if product with same name exists
    if (it!=nameMap.end()) {
        cout<<"Product with name "<<name<<" already exists with ID: "<<it->second<<endl;
        return -1; // return -1 for failure, admin should call update() instead
    }

    // check if category and section are valid and correspond, return -1 for invalid input
    if (cat == Category::Other && sec != Section::Other) {
        cout << "Category 'Other' only supports section 'Other'. Auto-change section to Other." << endl;
        sec = Section::Other;   // auto change to Other
    } else if ((cat == Category::Men || cat == Category::Women) &&
               (sec == Section::Boys || sec == Section::Girls)) {
        cout << "Invalid section for Men/Women category." << endl;
        return -1;
    } else if (cat == Category::Kids &&
               (sec == Section::Eastern || sec == Section::Western)) {
        cout << "Invalid section for Kids category." << endl;
        return -1;
    }
    vector<int> sizeStock(6,0); // initialize XS, S, M, L, XL, None to zero-stock
    // User interaction: ask if size attributes exist
    cout << "Does this product have size attributes? (1 for Yes, 0 for No): ";
    int hasSizeFlag;
    cin >> hasSizeFlag;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear newline from input buffer
    bool hasSize = (hasSizeFlag == 1);
    // Input size stock based on hasSize flag
    if (hasSize) {
        // sized product: read XS-XL stocks; None always 0
        cout << "Enter stock for each size:\n";
        cout << "XS: ";
        cin >> sizeStock[0];
        cout << "S: ";
        cin >> sizeStock[1];
        cout << "M: ";
        cin >> sizeStock[2];
        cout << "L: ";
        cin >> sizeStock[3];
        cout << "XL: ";
        cin >> sizeStock[4];
        sizeStock[5] = 0; // None not used for sized product
    } else {
        // size-less product: only None slot is used
        cout<<"Enter total stock for this product: ";
        cin>>sizeStock[5]; // store in None
        for (int i = 0; i < 5; ++i) sizeStock[i] = 0; // ensure sized slots are 0
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear newline from input buffer
    int productID=nextProductID++;  // assign and increment next productID
    Product newProduct(productID,name,cat,sec,sizeStock,price); // create new Product
    newProduct.setHasSize(hasSize); // record whether this product has sizes
    // Get category and section index
    int catIndex=getCategoryIndex(cat);
    int secIndex=getSectionIndex(cat,sec);
    products[catIndex][secIndex][productID]=newProduct; // store new product in products
    map[productID]=catIndex;    // record productID to category index in map
    nameMap[name]=productID;  // record name to productID in nameMap
    cout<<"Product added successfully with ID: "<<productID<<endl;
    return productID;   // return new productID
}

// Get non-const pointer to product by ID and return nullptr if not found
Product* ProductManager::getProduct(int productID) {
    auto it=map.find(productID);
    // check if product exists
    if (it==map.end()) {
        cout<<"Product ID "<<productID<<" not found."<<endl;
        return nullptr;
    }
    int catIndex=it->second;    // get category index from map
    // traverse all sections in category to find the product
    for (auto& secMap:products[catIndex]) {
        auto prodIt=secMap.find(productID);
        if (prodIt!=secMap.end()) {
            return &(prodIt->second);
        }
    }
    cout<<"Product ID "<<productID<<" not found in its category."<<endl;
    return nullptr;
}

// Get const pointer to product by ID and return nullptr if not found(same as non-const version)
const Product* ProductManager::getProduct(int productID) const {
    auto it = map.find(productID);
    if (it == map.end()) {
        return nullptr;
    }
    int catIndex = it->second;
    for (const auto& secMap : products[catIndex]) {
        auto prodIt = secMap.find(productID);
        if (prodIt != secMap.end()) {
            return &(prodIt->second);
        }
    }
    return nullptr;
}

// Remove product by ID, return true if removed successfully
bool ProductManager::removeProduct(int productID) {
    Product* prod=getProduct(productID);    // get non-const product pointer
    if (prod!=nullptr) {
        int catIndex=map[productID];    // get category index from map
        string oldName = prod->getProductName();    // store old name for nameMap remove
        // traverse all sections in category to find and erase the product
        for (auto& secMap : products[catIndex]) {
            // erase returns number of elements removed (0 or 1)
            if (secMap.erase(productID) > 0) {
                cout << "Product: " << oldName << " removed successfully." << endl;
                map.erase(productID);   // remove from map
                auto it = nameMap.find(oldName);
                if (it != nameMap.end() && it->second == productID) {
                    nameMap.erase(it);  // remove from nameMap
                }
                return true;
            }
        }
    }
    return false;   // return false if not found
}

// Update stock for a specific size of a product.
bool ProductManager::updateProduct(int productID, Size size, int newStock) {
    Product* prod = getProduct(productID);
    if (!prod) return false;    // check if product exists
    // check if new stock is valid(non-negative)
    if (newStock<0) {
        cout<<"Update failed: Stock can not be negative."<<endl;
        return false;
    }
    bool hasSize = prod->getHasSize();  // check whether product has size attributes
    // size rules based on hasSize flag
    if (!hasSize && size != Size::None) {
        cout << "Update failed: This product has no size; only None is allowed." << endl;
        return false;
    }
    if (hasSize && size == Size::None) {
        cout << "Update failed: Sized product does not use None stock." << endl;
        return false;
    }
    vector<int> stock=prod->getSizeStock(); // use a vector to get old stock
    int idx = static_cast<int>(size);
    // check if size index is valid
    if (idx < 0 || idx >= static_cast<int>(stock.size())) {
        cout << "Update failed: invalid size index." << endl;
        return false;
    }
    stock[idx] = newStock;  // update stock in vector
    prod->setSizeStock(stock);  // set updated stock back to product
    // output product name and new size info
    cout << "Update stock successfully for product ID: " << productID
         << " name: " << prod->getProductName()
         << " Size: " << sizeToString(size) << endl;
    return true;
}

// Update price of a product
bool ProductManager::updateProduct(int productID, double newPrice) {
    Product* prod = getProduct(productID);
    if (!prod) return false;
    // check if new price is valid(non-negative)
    if (newPrice<0) {
        cout<<"Update failed: Price can not be negative."<<endl;
        return false;
    }
    prod->setPrice(newPrice);   // call setter to set new price
    cout<<"Update price successfully for product ID: "<<productID
        <<" name: "<<prod->getProductName()
        <<" new price: "<<newPrice<<endl;
    return true;
}

// Update name of a product
bool ProductManager::updateProduct(int productID, const string &newName) {
    Product* prod = getProduct(productID);
    if (!prod) return false;
    auto it=nameMap.find(newName);
    // check if newname already exists
    if (it!=nameMap.end()) {
        cout<<"Update failed: Product name "<<newName<<" already exists with ID: "<<it->second<<endl;
        return false;
    }
    string oldName=prod->getProductName();  // store old name for nameMap update
    prod->setName(newName); // call setter to set new name
    // update nameMap: remove old name entry and add new name
    auto oldIt=nameMap.find(oldName);
    if (oldIt!=nameMap.end()&& oldIt->second==productID) {
        nameMap.erase(oldIt);
    }
    nameMap[newName]=productID;
    cout << "Update name successfully for product ID: " << productID
         << " new name: " << newName << endl;
    return true;
}

// Update category and section for a product and move it to corresponding bucket
bool ProductManager::updateProduct(int productID, Category newCat, Section newSec) {
    Product* prod = getProduct(productID);
    if (!prod) return false;
    // check if newCat and newSec are valid and correspond(same as in addProduct())
    if (newCat == Category::Other && newSec != Section::Other) {
        cout << "Category 'Other' only supports section 'Other'. Auto-change section to Other." << endl;
        newSec = Section::Other;
    } else if ((newCat == Category::Men || newCat == Category::Women) &&
               (newSec == Section::Boys || newSec == Section::Girls)) {
        cout << "Invalid section for Men/Women category." << endl;
        return false;
    } else if (newCat == Category::Kids &&
               (newSec == Section::Eastern || newSec == Section::Western)) {
        cout << "Invalid section for Kids category." << endl;
        return false;
    }
    Product oldCopy = *prod;    // make a copy of the old product to avoid dangling pointer
    // get old category and section index
    int oldCatIndex = map[productID];
    int oldSecIndex = getSectionIndex(oldCopy.getCategory(), oldCopy.getSection());
    // remove from old location
    auto &oldSecMap = products[oldCatIndex][oldSecIndex];
    auto it = oldSecMap.find(productID);
    if (it != oldSecMap.end()) {
        oldSecMap.erase(it);
    }
    // update category and section in the copy
    oldCopy.setCategory(newCat);
    oldCopy.setSection(newSec);
    // get new category and section index and insert into new location
    int newCatIndex = getCategoryIndex(newCat);
    int newSecIndex = getSectionIndex(newCat, newSec);
    products[newCatIndex][newSecIndex][productID] = oldCopy;
    map[productID] = newCatIndex;
    // output new category and section info
    cout << "Update category and section successfully for product ID: " << productID
         << " new category: " << static_cast<int>(newCat)
         << " new section: " << static_cast<int>(newSec) << endl;
    return true;
}

// Display a single product by ID.
void ProductManager::displaySingleProduct(int productID) const {
    const Product* p=getProduct(productID); // get const product pointer
    // check if product exists
    if (p==nullptr) {
        cout<<"Product ID "<<productID<<" not found."<<endl;
        return;
    }
    cout<<"Product Informations:"<<endl;
    cout << "ID: " << p->getProductID()
        << ", Name: " << p->getProductName()
        << ", Category: " << categoryToString(p->getCategory())
        << ", Section: " << sectionToString(p->getSection())
        << ", Price: " << p->getPrice()
        << ", Total Stock: " << p->getTotalStock();
    vector<int> stock = p->getSizeStock(); // get size stock
    // check if product has size attributes and display each size if so
    if (p->getHasSize()) {
        cout << ", Stock for size "
             << "XS: " << stock[0]
             << ", S: " << stock[1]
             << ", M: " << stock[2]
             << ", L: " << stock[3]
             << ", XL: " << stock[4];
    }
    cout << endl;
}

// Display all products in a specific (category, section).
void ProductManager::displayBySection(Category cat, Section sec) const {
    // check if category and section are valid and correspond(same as in addProduct())
    if (cat == Category::Other && sec != Section::Other) {
        cout << "Category 'Other' only supports section 'Other'. Auto-change section to Other." << endl;
        sec=Section::Other;
    } else if ((cat == Category::Men || cat== Category::Women) &&
           (sec == Section::Boys || sec == Section::Girls)) {
        cout << "Invalid section for Men/Women category." << endl;
        return;
    } else if (cat == Category::Kids &&
           (sec == Section::Eastern || sec == Section::Western)) {
        cout << "Invalid section for Kids category." << endl;
    return;
    }
    // get category and section index
    int catIndex=getCategoryIndex(cat);
    int secIndex=getSectionIndex(cat,sec);
    const auto& secMap=products[catIndex][secIndex];    // get section map
    // check if section map is empty
    if (secMap.empty()) {
        cout<<"No products found in category: "
            << categoryToString(cat)
            << ", section: " << sectionToString(sec) << endl;
        return;
    }
    // display all products in the section
    cout<<"All products in category: "
        << categoryToString(cat)
        << ", section: " << sectionToString(sec) << endl;
    for (const auto& pair : secMap) {
        const Product& p = pair.second;
        cout << "ID: " << p.getProductID()
             << ", Name: " << p.getProductName()
             << ", Price: " << p.getPrice()
             << ", Total Stock: " << p.getTotalStock();
        vector<int> stock=p.getSizeStock();
        // check if product has size attributes and display each size if so
        if (p.getHasSize()) {
            cout<<"Stock for size XS: "<<stock[0]
                <<", S: "<<stock[1]
                <<", M: "<<stock[2]
                <<", L: "<<stock[3]
                <<", XL: "<<stock[4];
        }
        cout<<endl;
    }
}

// Display all products in a given category with all sections inside
void ProductManager::displayByCategory(Category cat) const {
    int catIndex=getCategoryIndex(cat);  // get category index
    bool found = false;
    cout<<"Products in category: " << categoryToString(cat) << endl;
    // traverse all sections in the category
    for (const auto& secMap:products[catIndex]) {
        for (const auto& pair : secMap) {
            const Product& p = pair.second;
            cout << "ID: " << p.getProductID()
                 << ", Name: " << p.getProductName()
                 << ", Section: " << sectionToString(p.getSection())
                 << ", Price: " << p.getPrice()
                 << ", Total Stock: " << p.getTotalStock();
            vector<int> stock=p.getSizeStock();
            // check if product has size attributes and display each size if so
            if (p.getHasSize()) {
                cout<<", Stock for size XS: "<<stock[0]
                    <<", S: "<<stock[1]
                    <<", M: "<<stock[2]
                    <<", L: "<<stock[3]
                    <<", XL: "<<stock[4];
            }
            cout<<endl;
            found = true;
        }
    }
    // output if no products found in the category
    if (!found) {
        cout << "  (no products in this category)" << endl;
    }
}

// Display all products
void ProductManager::displayAllProducts() const {
    bool found = false;
    cout<<"All products in the system:"<<endl;
    for (size_t catIdx = 0; catIdx < products.size(); ++catIdx) {
        const auto& category = products[catIdx];
        Category cat = static_cast<Category>(catIdx);
        bool categoryPrinted = false;
        for (size_t secIdx = 0; secIdx < category.size(); ++secIdx) {
            const auto& secMap = category[secIdx];
            if (secMap.empty()) continue;
            Section sec;
            // determine logical Section based on category and section idx
            if (cat == Category::Kids) {
                if (secIdx == 0) sec = Section::Boys;
                else if (secIdx == 1) sec = Section::Girls;
                else sec = Section::Other;
            } else if (cat == Category::Other) {
                sec = Section::Other;
            } else { // Men/Women
                if (secIdx == 0) sec = Section::Eastern;
                else if (secIdx == 1) sec = Section::Western;
                else sec = Section::Other;
            }
            // print category header once
            if (!categoryPrinted) {
                cout << "Category: " << categoryToString(cat) << endl;
                categoryPrinted = true;
            }
            cout << "  Section: " << sectionToString(sec) << endl;
            // display all products in the section
            for (const auto& pair : secMap) {
                const Product& p = pair.second;
                cout << "    ID: " << p.getProductID()
                     << ", Name: " << p.getProductName()
                     << ", Price: " << p.getPrice()
                     << ", Total Stock: " << p.getTotalStock();
                vector<int> stock=p.getSizeStock();
                if (p.getHasSize()) {
                    cout<<", Stock for size XS: "<<stock[0]
                        <<", S: "<<stock[1]
                        <<", M: "<<stock[2]
                        <<", L: "<<stock[3]
                        <<", XL: "<<stock[4];
                }
                cout<<endl;
                found=true;
            }
        }
    }
    if (!found) {
        cout << "No products available." << endl;
    }
}

// Save all products to file: id,name,catIdx,secIdx,price,stock[6]
bool ProductManager::saveToFile(const string &filename) const {
    ofstream file(filename);
    // check if file opened successfully
    if (!file.is_open()) {
        cout<<"Failed to open file for writing: "<<filename<<endl;
        return false;
    }
    // write nextProductID first
    file << nextProductID << endl;
    // traverse to write each product record
    for (const auto& category : products) {
        for (const auto& secMap : category) {
            for (const auto& pair : secMap) {
                const Product& p = pair.second;
                // get category and section index
                Category cat = p.getCategory();
                Section sec = p.getSection();
                int catIdx = getCategoryIndex(cat);
                int secIdx = getSectionIndex(cat, sec);
                file << p.getProductID() << ","
                     << p.getProductName() << ","
                     << catIdx << ","
                     << secIdx << ","
                     << p.getPrice();
                // write size stock
                for (int stock : p.getSizeStock()) {
                    file << "," << stock;
                }
                file << endl;
            }
        }
    }
    file.close();
    cout << "Products saved successfully to " << filename << endl;
    return true;
}

// Load all products from file
bool ProductManager::loadFromFile(const string &filename) {
    ifstream file(filename);
    // check if file opened successfully
    if (!file.is_open()) {
        cout<<"Failed to open file for reading: "<<filename<<endl;
        return false;
    }
    file>>nextProductID;    // read nextProductID first
    file.ignore(numeric_limits<streamsize>::max(), '\n'); // clear newline
    products.clear();   // clear existing products
    products.resize(4); // 4 categories
    // resize each category to 3 sections
    for (auto& cat : products) {
        cat.resize(3);
    }
    // clear two maps
    map.clear();
    nameMap.clear();
    string line;
    // read each product record line by line
    while (getline(file, line)) {
        if (line.empty()) continue;     // skip empty lines
        size_t pos = 0;
        vector<string> tokens;  // to hold split fields
        string tmp = line;
        // split by comma
        while ((pos = tmp.find(',')) != string::npos) {
            tokens.push_back(tmp.substr(0, pos));
            tmp.erase(0, pos + 1);
        }
        tokens.push_back(tmp);
        // check if the format is valid
        if (tokens.size() < 11) {
            cout << "Invalid product record (too few fields), skip line: " << line << endl;
            continue;
        }
        // record each data
        int id = stoi(tokens[0]);
        string name = tokens[1];
        int catIdx = stoi(tokens[2]);
        int secIdx = stoi(tokens[3]);
        double price = stod(tokens[4]);
        if (catIdx < 0 || catIdx >= 4) {
            cout << "Invalid category index in file, skip product ID: " << id << endl;
            continue;
        }
        Category cat = static_cast<Category>(catIdx);
        // determine Section based on category and secIdx
        Section sec;
        if (cat == Category::Kids) {
            if (secIdx == 0) sec = Section::Boys;
            else if (secIdx == 1) sec = Section::Girls;
            else sec = Section::Other;
        } else if (cat == Category::Other) {
            // Other 分类统一视为 Other section
            sec = Section::Other;
        } else {
            // Men / Women
            if (secIdx == 0) sec = Section::Eastern;
            else if (secIdx == 1) sec = Section::Western;
            else sec = Section::Other;
        }
        // read size stock
        vector<int> sizeStock;
        for (size_t i = 5; i < 11 && i < tokens.size(); ++i) {
            sizeStock.push_back(stoi(tokens[i]));
        }
        // ensure sizeStock has 6 elements
        if (sizeStock.size() < 6) {
            sizeStock.resize(6, 0);
        }
        // create Product and insert into products
        Product p(id, name, cat, sec, sizeStock, price);
        int realCatIdx = getCategoryIndex(cat);
        int realSecIdx = getSectionIndex(cat, sec);
        products[realCatIdx][realSecIdx][id] = p;
        map[id] = realCatIdx;
        nameMap[name]=id;
    }
    file.close();
    cout << "Products loaded successfully from " << filename << endl;
    return true;
}
