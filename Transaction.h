#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <ctime>

#include "Product.h"
#include "ProductManager.h"
#include "ShoppingCart.h"

using namespace std;

// Single transaction item (records purchase info for one product)
struct TransactionItem {
    int productID;
    string productName;
    Category category;
    Section section;
    double unitPrice;
    vector<int> quantities;  // quantities for 6 sizes (XS, S, M, L, XL, None)
    double subtotal;         // subtotal for this item

    TransactionItem();
    TransactionItem(int id, const string& name, Category cat, Section sec,
                    double price, const vector<int>& qtys);
};

// Complete transaction record
class Transaction {
private:
    int transactionID;              // transaction ID (global unique in TransactionRecord.txt)
    int userID;                     // user ID
    vector<TransactionItem> items;  // list of purchased items
    double rawTotal;                // original total price
    double discountRate;            // discount rate (0.0-1.0)
    double finalTotal;              // final price after discount
    string timestamp;               // transaction timestamp
    int userLevel;                  // user level at time of transaction

public:
    // Constructors
    Transaction();
    Transaction(int txID, int uID, const vector<TransactionItem>& itms,
                double raw, double rate, double final_, const string& time, int level);

    // Getters
    int getTransactionID() const { return transactionID; }
    int getUserID() const { return userID; }
    const vector<TransactionItem>& getItems() const { return items; }
    double getRawTotal() const { return rawTotal; }
    double getDiscountRate() const { return discountRate; }
    double getFinalTotal() const { return finalTotal; }
    string getTimestamp() const { return timestamp; }
    int getUserLevel() const { return userLevel; }

    // Display transaction details (invoice format)
    void displayInvoice() const;

    // Serialization/deserialization (for file I/O)
    string serialize() const;
    static optional<Transaction> deserialize(const vector<string>& lines);
};

// Transaction manager (handles all transactions; userID used for filtering/display)
// userID >= 1 : user view (filter own records)
// userID == -1: admin view (no filter)
class TransactionManager {
private:
    int userID;                         // current user context (or -1 for admin)
    int nextTransactionID;              // next transaction ID (global)
    vector<Transaction> transactions;   // all loaded transaction records

    // Global transaction record file name (requirement)
    string getFileName() const;

    // Get current timestamp string
    static string getCurrentTimestamp();

    // Get discount rate based on user level (Silver/Gold/Diamond only)
    static double getDiscountRate(int level, bool isAdmin);

    // Get level name string (Silver/Gold/Diamond only)
    static string getLevelName(int level);

    bool allowTx(const Transaction& tx) const {
        return (userID == -1) || (tx.getUserID() == userID);
    }

public:
    // Constructors
    TransactionManager();
    explicit TransactionManager(int uID);

    // Set user ID context
    void setUserID(int uID);
    int getUserID() const { return userID; }

    // File I/O
    bool loadFromFile();
    bool saveToFile() const;

    // Check if stock is sufficient, return shortage info
    // Returns: map<productID, vector<pair<size, shortage>>>
    map<int, vector<pair<Size, int>>> checkStock(const ShoppingCart& cart,
                                                 const ProductManager& pm) const;

    // Check and resolve stock issues (interactive)
    // Returns true if checkout can proceed, false if user cancels
    bool checkAndResolveStock(ShoppingCart& cart, ProductManager& pm);

    // Process transaction (checkout)
    bool processTransaction(ShoppingCart& cart, ProductManager& pm,
                            int userLevel, bool isAdmin);

    // Display (filtered by userID unless admin)
    void displayAllTransactions() const;
    void displayTransactionSummary() const;

    // Find (filtered by userID unless admin)
    const Transaction* findTransaction(int transactionID) const;
    void displayTransaction(int transactionID) const;

    // Find transactions by date range (filtered by userID unless admin)
    vector<const Transaction*> findByDateRange(const string& startDate,
                                               const string& endDate) const;

    // Find transactions by amount range (filtered by userID unless admin)
    vector<const Transaction*> findByAmountRange(double minAmount,
                                                 double maxAmount) const;

    // Stats (filtered by userID unless admin)
    int getTransactionCount() const;
    double getTotalSpent() const;
    double getAverageSpent() const;

    // Get all transactions (read-only; NOTE: contains all loaded txs)
    const vector<Transaction>& getAllTransactions() const { return transactions; }
};

#endif // TRANSACTION_H