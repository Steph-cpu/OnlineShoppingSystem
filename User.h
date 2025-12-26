#ifndef ASSIGNMENT2_USER_H
#define ASSIGNMENT2_USER_H

#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <fstream>
#include <limits>

#include "ProductManager.h"
#include "ShoppingCart.h"
#include "Transaction.h"

using namespace std;

class User {
public:
    int userID = 0;
    string username;
    string password;
    int level = 1;       // 1: Silver, 2: Gold, 3: Diamond
    bool isAdmin = false;
    double totalSpent = 0.0;

    ShoppingCart cart;
    TransactionManager txm; // user context filtering

    // Pending admin requests: username|password
    static vector<pair<string, string>> pendingAdmins;

public:
    User() : txm(0) {}

    User(int id, string name, string pwd, int lvl, bool admin, double spent)
        : userID(id), username(std::move(name)), password(std::move(pwd)),
          level(lvl), isAdmin(admin), totalSpent(spent),
          txm(id) {}

    static string usersFileName() { return "users.txt"; }

    static bool loadAll(vector<User>& users, int& nextUserID, const string& filename = usersFileName());
    static bool saveAll(const vector<User>& users, int nextUserID, const string& filename = usersFileName());
	static void createDefaultAdmin(vector<User>& users, int& nextUserID);
    static bool registerUser(vector<User>& users, int& nextUserID,
                             const string& username, const string& password,
                             bool isAdmin = false);

    static User* login(vector<User>& users, const string& username, const string& password);

    // Admin approval system
    static bool requestAdminAccess(const string& username, const string& password);
    static bool approveAdminRequest(vector<User>& users, int& nextUserID, int requestIndex);
    static bool rejectAdminRequest(int requestIndex);
    static void displayPendingRequests();
    static bool hasPendingRequests() { return !pendingAdmins.empty(); }

    // Forgot password
    static bool resetPassword(vector<User>& users,
                              const string& username,
                              const string& newPassword);

    // cart file per user
    string cartFileName() const { return ShoppingCart::getShoppingCartFileName(userID); }
    bool loadCartFromFile();
    bool saveCartToFile() const;

    // discounts (Silver/Gold/Diamond only)
    double discountRate() const;
    static string levelName(int lvl) {
        if (lvl <= 1) return "Silver";
        if (lvl == 2) return "Gold";
        return "Diamond";
    }
    void updateLevelBySpent();

    // cart operations
    void addToCart(int productID, const ProductManager& pm) { cart.addItem(productID, pm); }
    void updateCartItem(int productID, const ProductManager& pm) { cart.updateItem(productID, pm); }
    void removeFromCart(int productID) { cart.removeItem(productID); }
    void showCart(const ProductManager& pm) const { cart.displayCart(pm); }
    void clearCart() { cart.clearCart(); }

    // checkout via TransactionManager
    bool checkout(ProductManager& pm);

    // transaction displays (filtered)
    void displayTransactionSummary() { ensureTxmBound(); txm.displayTransactionSummary(); }
    void displayAllTransactions()    { ensureTxmBound(); txm.displayAllTransactions(); }
    void displayTransaction(int txID){ ensureTxmBound(); txm.displayTransaction(txID); }

private:
    static bool isUsernameValid(const string& name);
    static bool isPasswordValid(const string& pwd);

    static optional<User> parseUserLine(const string& line);
    static string toUserLine(const User& u);

    static bool usernameExists(const vector<User>& users, const string& username);
    static vector<string> split(const string& s, char delim);

    void ensureTxmBound() {
        if (txm.getUserID() != userID) txm.setUserID(userID);
    }
};

#endif // ASSIGNMENT2_USER_H
