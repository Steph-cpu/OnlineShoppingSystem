#include "Transaction.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <limits>
#include <algorithm>

using namespace std;

// ==================== TransactionItem ====================

TransactionItem::TransactionItem()
    : productID(0), productName(""), category(Category::Men), 
      section(Section::Eastern), unitPrice(0.0), subtotal(0.0) {
    quantities.resize(6, 0);
}

TransactionItem::TransactionItem(int id, const string& name, Category cat, Section sec,
                                  double price, const vector<int>& qtys)
    : productID(id), productName(name), category(cat), section(sec),
      unitPrice(price), quantities(qtys) {
    if (quantities.size() < 6) quantities.resize(6, 0);
    // Calculate subtotal
    int totalQty = accumulate(quantities.begin(), quantities.end(), 0);
    subtotal = unitPrice * totalQty;
}

// ==================== Transaction ====================

Transaction::Transaction()
    : transactionID(0), userID(0), rawTotal(0.0), 
      discountRate(1.0), finalTotal(0.0), timestamp(""), userLevel(1) {}

Transaction::Transaction(int txID, int uID, const vector<TransactionItem>& itms,
                        double raw, double rate, double final_, 
                        const string& time, int level)
    : transactionID(txID), userID(uID), items(itms), rawTotal(raw),
      discountRate(rate), finalTotal(final_), timestamp(time), userLevel(level) {}

void Transaction::displayInvoice() const {
    cout << "\n";
    cout << "================================================================" << endl;
    cout << "                    TRANSACTION INVOICE                         " << endl;
    cout << "================================================================" << endl;
    cout << "  Transaction ID: " << transactionID 
         << "          User ID: " << userID << endl;
    cout << "  Date/Time: " << timestamp << endl;
    cout << "  Customer Level: " << userLevel << endl;
    cout << "----------------------------------------------------------------" << endl;
    cout << "                      ITEMS PURCHASED                           " << endl;
    cout << "----------------------------------------------------------------" << endl;

    int itemNum = 1;
    for (const auto& item : items) {
        cout << "  Item #" << itemNum++ << endl;
        cout << "  Product ID: " << item.productID << endl;
        cout << "  Name: " << item.productName << endl;
        cout << "  Category: " << categoryToString(item.category) 
             << " | Section: " << sectionToString(item.section) << endl;
        cout << "  Unit Price: $" << fixed << setprecision(2) << item.unitPrice << endl;
        cout << "  Quantities: ";
        
        bool first = true;
        for (int i = 0; i < 6; ++i) {
            if (item.quantities[i] > 0) {
                if (!first) cout << ", ";
                cout << sizeToString(static_cast<Size>(i)) << "=" << item.quantities[i];
                first = false;
            }
        }
        cout << endl;
        cout << "  Subtotal: $" << fixed << setprecision(2) << item.subtotal << endl;
        cout << "  --------------------------------------------------------------" << endl;
    }

    cout << "----------------------------------------------------------------" << endl;
    cout << "                       PAYMENT SUMMARY                          " << endl;
    cout << "----------------------------------------------------------------" << endl;
    cout << "  Raw Total:       $" << fixed << setprecision(2) << rawTotal << endl;
    cout << "  Discount Rate:    " << fixed << setprecision(0) << (discountRate * 100) << "%" << endl;
    cout << "  Discount Amount: $" << fixed << setprecision(2) << (rawTotal - finalTotal) << endl;
    cout << "  =============================================================" << endl;
    cout << "  FINAL TOTAL:     $" << fixed << setprecision(2) << finalTotal << endl;
    cout << "================================================================" << endl;
    cout << "\n";
}

string Transaction::serialize() const {
    ostringstream oss;
    // First line: basic transaction info
    // Format: TX|transactionID|userID|rawTotal|discountRate|finalTotal|timestamp|userLevel|itemCount
    oss << "TX|" << transactionID << "|" << userID << "|" 
        << fixed << setprecision(2) << rawTotal << "|"
        << discountRate << "|" << finalTotal << "|"
        << timestamp << "|" << userLevel << "|" << items.size() << "\n";

    // Subsequent lines: each item
    // Format: ITEM|productID|productName|category|section|unitPrice|q0|q1|q2|q3|q4|q5|subtotal
    for (const auto& item : items) {
        oss << "ITEM|" << item.productID << "|" << item.productName << "|"
            << static_cast<int>(item.category) << "|" << static_cast<int>(item.section) << "|"
            << fixed << setprecision(2) << item.unitPrice << "|";
        for (int i = 0; i < 6; ++i) {
            oss << item.quantities[i];
            if (i < 5) oss << "|";
        }
        oss << "|" << fixed << setprecision(2) << item.subtotal << "\n";
    }

    return oss.str();
}

optional<Transaction> Transaction::deserialize(const vector<string>& lines) {
    if (lines.empty()) return nullopt;

    // Parse first line
    istringstream iss(lines[0]);
    string token;
    vector<string> parts;
    while (getline(iss, token, '|')) {
        parts.push_back(token);
    }

    if (parts.size() != 9 || parts[0] != "TX") return nullopt;

    try {
        int txID = stoi(parts[1]);
        int uID = stoi(parts[2]);
        double raw = stod(parts[3]);
        double rate = stod(parts[4]);
        double final_ = stod(parts[5]);
        string time = parts[6];
        int level = stoi(parts[7]);
        int itemCount = stoi(parts[8]);

        vector<TransactionItem> items;
        
        // Parse item lines
        for (size_t i = 1; i < lines.size() && i <= static_cast<size_t>(itemCount); ++i) {
            istringstream itemIss(lines[i]);
            vector<string> itemParts;
            string itemToken;
            while (getline(itemIss, itemToken, '|')) {
                itemParts.push_back(itemToken);
            }

            if (itemParts.size() != 13 || itemParts[0] != "ITEM") continue;

            TransactionItem item;
            item.productID = stoi(itemParts[1]);
            item.productName = itemParts[2];
            item.category = static_cast<Category>(stoi(itemParts[3]));
            item.section = static_cast<Section>(stoi(itemParts[4]));
            item.unitPrice = stod(itemParts[5]);
            
            item.quantities.resize(6);
            for (int j = 0; j < 6; ++j) {
                item.quantities[j] = stoi(itemParts[6 + j]);
            }
            item.subtotal = stod(itemParts[12]);

            items.push_back(item);
        }

        return Transaction(txID, uID, items, raw, rate, final_, time, level);
    } catch (...) {
        return nullopt;
    }
}

// ==================== TransactionManager ====================

TransactionManager::TransactionManager() 
    : userID(0), nextTransactionID(1) {}

TransactionManager::TransactionManager(int uID) 
    : userID(uID), nextTransactionID(1) {
    loadFromFile();
}

void TransactionManager::setUserID(int uID) {
    userID = uID;
    nextTransactionID = 1;
    transactions.clear();
    loadFromFile();
}

string TransactionManager::getFileName() const {
    return "transactions_user_" + to_string(userID) + ".txt";
}

string TransactionManager::getCurrentTimestamp() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    
    ostringstream oss;
    oss << (1900 + ltm->tm_year) << "-"
        << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
        << setfill('0') << setw(2) << ltm->tm_mday << " "
        << setfill('0') << setw(2) << ltm->tm_hour << ":"
        << setfill('0') << setw(2) << ltm->tm_min << ":"
        << setfill('0') << setw(2) << ltm->tm_sec;
    
    return oss.str();
}

double TransactionManager::getDiscountRate(int level, bool isAdmin) {
    // Discount rules consistent with User class
    if (isAdmin) return 0.80;          // Admin: 20% discount
    if (level <= 1) return 1.00;       // Silver: no discount
    if (level == 2) return 0.98;       // Gold: 2% discount
    if (level == 3) return 0.95;       // Diamond: 5% discount
    return 0.90;                        // Level 4+: 10% discount
}

string TransactionManager::getLevelName(int level) {
    switch (level) {
        case 1: return "Silver";
        case 2: return "Gold";
        case 3: return "Diamond";
        case 4: return "Platinum";
        default: return "VIP";
    }
}

bool TransactionManager::loadFromFile() {
    transactions.clear();
    nextTransactionID = 1;

    ifstream fin(getFileName());
    if (!fin.is_open()) {
        // File doesn't exist, first time use
        return true;
    }

    // First line: nextTransactionID
    string firstLine;
    if (getline(fin, firstLine)) {
        try {
            nextTransactionID = stoi(firstLine);
            if (nextTransactionID < 1) nextTransactionID = 1;
        } catch (...) {
            nextTransactionID = 1;
        }
    }

    // Read all transaction records
    string line;
    vector<string> currentTxLines;

    while (getline(fin, line)) {
        if (line.empty()) continue;

        if (line.substr(0, 3) == "TX|") {
            // If there's previous transaction data, parse it first
            if (!currentTxLines.empty()) {
                auto tx = Transaction::deserialize(currentTxLines);
                if (tx.has_value()) {
                    transactions.push_back(*tx);
                }
                currentTxLines.clear();
            }
            currentTxLines.push_back(line);
        } else if (line.substr(0, 5) == "ITEM|") {
            currentTxLines.push_back(line);
        }
    }

    // Process last transaction
    if (!currentTxLines.empty()) {
        auto tx = Transaction::deserialize(currentTxLines);
        if (tx.has_value()) {
            transactions.push_back(*tx);
        }
    }

    fin.close();
    return true;
}

bool TransactionManager::saveToFile() const {
    ofstream fout(getFileName());
    if (!fout.is_open()) {
        cout << "Failed to open transaction file for writing: " << getFileName() << endl;
        return false;
    }

    // First line: nextTransactionID
    fout << nextTransactionID << "\n";

    // Write all transaction records
    for (const auto& tx : transactions) {
        fout << tx.serialize();
    }

    fout.close();
    return true;
}

map<int, vector<pair<Size, int>>> TransactionManager::checkStock(
    const ShoppingCart& cart, const ProductManager& pm) const {
    
    map<int, vector<pair<Size, int>>> shortages;
    const auto& cartItems = cart.getItems();

    for (const auto& [productID, qtyVec] : cartItems) {
        const Product* p = pm.getProduct(productID);
        if (!p) {
            // Product doesn't exist, record all quantities as shortage
            vector<pair<Size, int>> itemShortages;
            for (int i = 0; i < 6; ++i) {
                int qty = (i < static_cast<int>(qtyVec.size())) ? qtyVec[i] : 0;
                if (qty > 0) {
                    itemShortages.push_back({static_cast<Size>(i), qty});
                }
            }
            if (!itemShortages.empty()) {
                shortages[productID] = itemShortages;
            }
            continue;
        }

        const auto& stock = p->getSizeStock();
        vector<pair<Size, int>> itemShortages;

        for (int i = 0; i < 6; ++i) {
            int qty = (i < static_cast<int>(qtyVec.size())) ? qtyVec[i] : 0;
            if (qty <= 0) continue;

            int available = (i < static_cast<int>(stock.size())) ? stock[i] : 0;
            if (qty > available) {
                // Record shortage amount
                itemShortages.push_back({static_cast<Size>(i), qty - available});
            }
        }

        if (!itemShortages.empty()) {
            shortages[productID] = itemShortages;
        }
    }

    return shortages;
}

bool TransactionManager::checkAndResolveStock(ShoppingCart& cart, ProductManager& pm) {
    while (true) {
        auto shortages = checkStock(cart, pm);
        
        if (shortages.empty()) {
            return true;  // Stock is sufficient, can proceed
        }

        // Display stock shortage information
        cout << "\n========== STOCK SHORTAGE ALERT ==========" << endl;
        cout << "The following items have insufficient stock:" << endl;

        for (const auto& [productID, itemShortages] : shortages) {
            const Product* p = pm.getProduct(productID);
            string productName = p ? p->getProductName() : "Unknown Product";
            
            cout << "\nProduct ID: " << productID << " - " << productName << endl;
            
            for (const auto& [size, shortage] : itemShortages) {
                int available = 0;
                if (p) {
                    const auto& stock = p->getSizeStock();
                    int idx = static_cast<int>(size);
                    available = (idx < static_cast<int>(stock.size())) ? stock[idx] : 0;
                }
                cout << "  Size: " << sizeToString(size) 
                     << " - Short by: " << shortage
                     << " (Available: " << available << ")" << endl;
            }
        }

        cout << "\n============================================" << endl;
        cout << "Please choose an option:" << endl;
        cout << "1. Update item quantity" << endl;
        cout << "2. Remove item from cart" << endl;
        cout << "3. Cancel checkout" << endl;
        cout << "Enter choice (1-3): ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input." << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                // Update item quantity
                cout << "Enter Product ID to update: ";
                int updateID;
                if (!(cin >> updateID)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid Product ID." << endl;
                    continue;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                
                cart.updateItem(updateID, pm);
                break;
            }
            case 2: {
                // Remove item
                cout << "Enter Product ID to remove: ";
                int removeID;
                if (!(cin >> removeID)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid Product ID." << endl;
                    continue;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                
                cart.removeItem(removeID);
                
                // Check if cart is empty
                if (cart.getItems().empty()) {
                    cout << "Cart is now empty. Checkout cancelled." << endl;
                    return false;
                }
                break;
            }
            case 3:
                cout << "Checkout cancelled." << endl;
                return false;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
}

bool TransactionManager::processTransaction(ShoppingCart& cart, ProductManager& pm,
                                            int userLevel, bool isAdmin) {
    // Check if cart is empty
    if (cart.getItems().empty()) {
        cout << "Transaction failed: Cart is empty." << endl;
        return false;
    }

    // Check and resolve stock issues
    if (!checkAndResolveStock(cart, pm)) {
        return false;
    }

    const auto& cartItems = cart.getItems();

    // Build transaction item list
    vector<TransactionItem> txItems;
    double rawTotal = 0.0;

    for (const auto& [productID, qtyVec] : cartItems) {
        const Product* p = pm.getProduct(productID);
        if (!p) {
            cout << "Transaction failed: Product not found, ID=" << productID << endl;
            return false;
        }

        // Check if there's actual purchase quantity
        int totalQty = accumulate(qtyVec.begin(), qtyVec.end(), 0);
        if (totalQty <= 0) continue;

        TransactionItem item(
            productID,
            p->getProductName(),
            p->getCategory(),
            p->getSection(),
            p->getPrice(),
            qtyVec
        );

        txItems.push_back(item);
        rawTotal += item.subtotal;
    }

    if (txItems.empty()) {
        cout << "Transaction failed: No valid items to purchase." << endl;
        return false;
    }

    // Calculate discount
    double rate = getDiscountRate(userLevel, isAdmin);
    double finalTotal = rawTotal * rate;

    // Deduct stock
    for (const auto& [productID, qtyVec] : cartItems) {
        Product* p = pm.getProduct(productID);
        if (!p) continue;

        for (int i = 0; i < 6; ++i) {
            int qty = (i < static_cast<int>(qtyVec.size())) ? qtyVec[i] : 0;
            if (qty <= 0) continue;

            if (!p->updateStock(static_cast<Size>(i), -qty)) {
                cout << "Transaction failed: Stock deduction error. ProductID=" 
                     << productID << endl;
                return false;
            }
        }
    }

    // Create transaction record
    string timestamp = getCurrentTimestamp();
    Transaction newTx(nextTransactionID, userID, txItems, rawTotal, 
                      rate, finalTotal, timestamp, userLevel);

    // Save transaction record
    transactions.push_back(newTx);
    nextTransactionID++;

    // Save to file
    saveToFile();

    // Save product stock updates
    pm.saveToFile();

    // Display invoice
    cout << "\n========== TRANSACTION SUCCESSFUL ==========" << endl;
    newTx.displayInvoice();

    // Clear cart
    cart.clearCart();

    cout << "Thank you for your purchase!" << endl;
    cout << "Your member level: " << getLevelName(userLevel) << endl;

    return true;
}

void TransactionManager::displayAllTransactions() const {
    if (transactions.empty()) {
        cout << "\nNo transaction records found for User ID: " << userID << endl;
        return;
    }

    cout << "\n================================================================" << endl;
    cout << "       ALL TRANSACTION RECORDS - User ID: " << userID << endl;
    cout << "================================================================" << endl;

    for (const auto& tx : transactions) {
        tx.displayInvoice();
        cout << endl;
    }

    // Display statistics
    cout << "================================================================" << endl;
    cout << "                        STATISTICS                              " << endl;
    cout << "================================================================" << endl;
    cout << "  Total Transactions: " << getTransactionCount() << endl;
    cout << "  Total Spent:        $" << fixed << setprecision(2) << getTotalSpent() << endl;
    cout << "  Average per Order:  $" << fixed << setprecision(2) << getAverageSpent() << endl;
    cout << "================================================================" << endl;
}

void TransactionManager::displayTransactionSummary() const {
    if (transactions.empty()) {
        cout << "\nNo transaction records found." << endl;
        return;
    }

    cout << "\n======== TRANSACTION SUMMARY - User ID: " << userID << " ========" << endl;
    cout << left << setw(8) << "TX ID" 
         << setw(22) << "Date/Time" 
         << setw(8) << "Items"
         << setw(12) << "Raw Total"
         << setw(10) << "Discount"
         << setw(12) << "Final" << endl;
    cout << string(72, '-') << endl;

    for (const auto& tx : transactions) {
        cout << left << setw(8) << tx.getTransactionID()
             << setw(22) << tx.getTimestamp()
             << setw(8) << tx.getItems().size()
             << "$" << setw(11) << fixed << setprecision(2) << tx.getRawTotal()
             << setw(10) << fixed << setprecision(0) << (tx.getDiscountRate() * 100) << "%"
             << "$" << setw(11) << fixed << setprecision(2) << tx.getFinalTotal() << endl;
    }

    cout << string(72, '-') << endl;
    cout << "Total Transactions: " << getTransactionCount() 
         << " | Total Spent: $" << fixed << setprecision(2) << getTotalSpent() << endl;
}

const Transaction* TransactionManager::findTransaction(int transactionID) const {
    for (const auto& tx : transactions) {
        if (tx.getTransactionID() == transactionID) {
            return &tx;
        }
    }
    return nullptr;
}

void TransactionManager::displayTransaction(int transactionID) const {
    const Transaction* tx = findTransaction(transactionID);
    if (tx) {
        tx->displayInvoice();
    } else {
        cout << "Transaction ID " << transactionID << " not found." << endl;
    }
}

vector<const Transaction*> TransactionManager::findByDateRange(
    const string& startDate, const string& endDate) const {
    
    vector<const Transaction*> result;
    for (const auto& tx : transactions) {
        string txDate = tx.getTimestamp().substr(0, 10);  // Extract date part only
        if (txDate >= startDate && txDate <= endDate) {
            result.push_back(&tx);
        }
    }
    return result;
}

vector<const Transaction*> TransactionManager::findByAmountRange(
    double minAmount, double maxAmount) const {
    
    vector<const Transaction*> result;
    for (const auto& tx : transactions) {
        double amount = tx.getFinalTotal();
        if (amount >= minAmount && amount <= maxAmount) {
            result.push_back(&tx);
        }
    }
    return result;
}

double TransactionManager::getTotalSpent() const {
    double total = 0.0;
    for (const auto& tx : transactions) {
        total += tx.getFinalTotal();
    }
    return total;
}

double TransactionManager::getAverageSpent() const {
    if (transactions.empty()) return 0.0;
    return getTotalSpent() / static_cast<double>(transactions.size());
}
