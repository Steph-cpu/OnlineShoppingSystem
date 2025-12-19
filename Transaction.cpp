#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <map>
#include <limits>

// Include your existing headers
#include "User.h"
#include "ProductManager.h"
#include "ShoppingCart.h"
#include "Product.h"

using namespace std;

class Transaction {
private:
    int transactionID;
    int userID;
    string dateStr;
    double totalAmount;
    double discountRate;
    double finalAmount;

    // Structure to hold a snapshot of the purchase items
    struct TransactionDetail {
        string productName;
        string size;
        int quantity;
        double price;
        double subtotal;
    };
    vector<TransactionDetail> itemsSnapshot;

    // Helper: Get current system time as string
    string getCurrentTime() {
        time_t now = time(0);
        char* dt = ctime(&now);
        string t(dt);
        if (!t.empty()) t.pop_back(); // Remove newline character
        return t;
    }

    // Helper: Generate filename based on User ID
    string getFileName(int uid) {
        return "TransactionRecord_" + to_string(uid) + ".txt";
    }

    // Helper: Generate Unique ID per user (by counting existing records)
    int generateTransactionID(int uid) {
        ifstream file(getFileName(uid));
        if (!file.is_open()) return 1; // First transaction

        int count = 0;
        string line;
        while (getline(file, line)) {
            // Counting occurrences of "Transaction ID" to determine the next ID
            if (line.find("Transaction ID:") != string::npos) {
                count++;
            }
        }
        return count + 1;
    }

    // Core Logic: Check Stock and Resolve Conflicts
    // Returns: true if stock is okay, false if cart was modified (requires re-check)
    bool checkAndResolveStock(User& user, ProductManager& pm) {
        // Access cart items using the new getter
        const auto& cartItems = user.getCart().getItems(); 
        
        if (cartItems.empty()) return true;

        for (auto const& [productID, quantities] : cartItems) {
            Product* p = pm.getProduct(productID);
            
            // Safety check if product was deleted from database
            if (!p) {
                cout << "Error: Product ID " << productID << " no longer exists. Removing from cart." << endl;
                user.getCart().removeItem(productID); 
                return false; // Cart structure changed, restart check
            }

            const vector<int>& stock = p->getSizeStock();

            // Iterate through all 6 size slots
            for (int i = 0; i < 6; ++i) {
                int cartQty = quantities[i];
                if (cartQty > 0) {
                    // Compare Cart Quantity vs Available Stock
                    if (cartQty > stock[i]) {
                        Size s = static_cast<Size>(i);
                        cout << "\n========== STOCK WARNING ==========" << endl;
                        cout << "Product: " << p->getProductName() << " (ID: " << productID << ")" << endl;
                        cout << "Size: " << sizeToString(s) << endl;
                        cout << "Your Quantity: " << cartQty << endl;
                        cout << "Available Stock: " << stock[i] << endl;
                        cout << "-----------------------------------" << endl;
                        cout << "Please choose an action:" << endl;
                        cout << "1. Remove this item from cart" << endl;
                        cout << "2. Update quantity to match stock" << endl;
                        cout << "Enter choice (1/2): ";
                        
                        int choice;
                        cin >> choice;
                        
                        // Clear input buffer
                        if(cin.fail()){
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            choice = 1; // Default to remove on error
                        } else {
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        }

                        if (choice == 1) {
                            user.getCart().removeItem(productID);
                            cout << "Item removed." << endl;
                            return false; // Restart check
                        } else {
                            // User chose to update/fix quantity
                            cout << "Enter new quantity (Max " << stock[i] << "): ";
                            int newQty;
                            cin >> newQty;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            
                            // Validate input
                            if (newQty > stock[i]) newQty = stock[i];
                            if (newQty < 0) newQty = 0;

                            if (newQty == 0) {
                                // If 0, effectively remove that size quantity
                                user.getCart().updateQuantityDirectly(productID, s, 0);
                            } else {
                                user.getCart().updateQuantityDirectly(productID, s, newQty);
                            }
                            cout << "Quantity updated to " << newQty << "." << endl;
                            return false; // Restart check
                        }
                    }
                }
            }
        }
        return true; // All items passed the stock check
    }

public:
    Transaction() {
        transactionID = 0;
        userID = 0;
        totalAmount = 0.0;
        finalAmount = 0.0;
        discountRate = 1.0;
    }

    // Main Function to Execute Transaction
    void processTransaction(User& user, ProductManager& pm) {
        // 1. Pre-check
        if (user.getCart().getItems().empty()) {
            cout << "Checkout failed: Your cart is empty." << endl;
            return;
        }

        cout << "\nProcessing Transaction..." << endl;

        // 2. Loop to validate stock until all conflicts are resolved
        bool stockIsValid = false;
        while (!stockIsValid) {
            stockIsValid = checkAndResolveStock(user, pm);
            
            // If the cart became empty during the resolution process
            if (user.getCart().getItems().empty()) {
                cout << "Cart is empty after stock adjustments. Transaction cancelled." << endl;
                return;
            }
        }

        // 3. Prepare Transaction Data
        this->userID = user.getUserID();
        this->transactionID = generateTransactionID(userID);
        this->dateStr = getCurrentTime();
        this->itemsSnapshot.clear();

        // 4. Calculate Financials
        this->totalAmount = user.getCart().calculateTotal(pm);
        this->discountRate = user.discountRate(); // Get rate (e.g., 0.95 for 5% off)
        this->finalAmount = totalAmount * discountRate;

        // 5. Deduct Stock and Record Snapshot
        const auto& cartItems = user.getCart().getItems();
        for (auto const& [productID, quantities] : cartItems) {
            Product* p = pm.getProduct(productID);
            for (int i = 0; i < 6; ++i) {
                if (quantities[i] > 0) {
                    Size s = static_cast<Size>(i);
                    
                    // Create snapshot detail
                    TransactionDetail detail;
                    detail.productName = p->getProductName();
                    detail.size = sizeToString(s);
                    detail.quantity = quantities[i];
                    detail.price = p->getPrice();
                    detail.subtotal = p->getPrice() * quantities[i];
                    itemsSnapshot.push_back(detail);

                    // UPDATE STOCK: Pass negative value to reduce stock
                    p->updateStock(s, -quantities[i]);
                }
            }
        }

        // 6. Save Record to File
        saveToFile();

        // 7. Update User stats (Total Spent) - Assumes User class has public access or method
        // user.totalSpent += finalAmount; 
        // user.updateLevelBySpent(); 
        // Note: Your User.cpp 'checkout' handles this, but since we are writing a dedicated Transaction class, 
        // we normally update it here. For now, we focus on the Transaction logic.

        // 8. Clear Cart and Update Cart File
        user.getCart().clearCart();
        user.saveCartToFile();

        // 9. Display Success Message
        cout << "\n============================================" << endl;
        cout << " Transaction Successful! " << endl;
        cout << "============================================" << endl;
        cout << " Transaction ID : " << transactionID << endl;
        cout << " Original Total : $" << fixed << setprecision(2) << totalAmount << endl;
        cout << " Discount Rate  : " << (int)((1.0 - discountRate) * 100) << "%" << endl;
        cout << " Final Paid     : $" << finalAmount << endl;
        cout << " Invoice saved to " << getFileName(userID) << endl;
        cout << "============================================" << endl;
    }

    // Save transaction details to text file
    void saveToFile() {
        ofstream outFile(getFileName(userID), ios::app); // Append mode
        if (!outFile.is_open()) {
            cerr << "Error: Could not write to transaction file." << endl;
            return;
        }

        outFile << "##################################################" << endl;
        outFile << "Transaction ID: " << transactionID << endl;
        outFile << "Date: " << dateStr << endl;
        outFile << "User ID: " << userID << endl;
        outFile << "--------------------------------------------------" << endl;
        outFile << left << setw(20) << "Product Name" 
                << setw(8) << "Size" 
                << setw(8) << "Qty" 
                << setw(10) << "Subtotal" << endl;
        
        for (const auto& item : itemsSnapshot) {
            outFile << left << setw(20) << item.productName 
                    << setw(8) << item.size 
                    << setw(8) << item.quantity 
                    << "$" << item.subtotal << endl;
        }
        outFile << "--------------------------------------------------" << endl;
        outFile << "Total Amount:  $" << totalAmount << endl;
        outFile << "Discount Applied: -$" << (totalAmount - finalAmount) << endl;
        outFile << "Final Paid:    $" << finalAmount << endl;
        outFile << "##################################################" << endl << endl;

        outFile.close();
    }

    // Static function to display history for a specific user
    static void displayHistory(int uid) {
        string filename = "TransactionRecord_" + to_string(uid) + ".txt";
        ifstream inFile(filename);

        cout << "\n>>> Transaction History for User ID: " << uid << " <<<" << endl;
        
        if (!inFile.is_open()) {
            cout << "No transaction records found for this user." << endl;
            return;
        }

        // Print entire file content to console
        cout << inFile.rdbuf(); 
        cout << "\n>>> End of History <<<" << endl;
        inFile.close();
    }
};
