#ifndef ASSIGNMENT2_USER_H
#define ASSIGNMENT2_USER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <iostream>
#include <fstream>
#include <limits>

#include "ProductManager.h"
#include "ShoppingCart.h"

using namespace std;

class User {
public:
    // ---------- basic data ----------
    int userID = 0;
    string username;
    string password;     // 作业场景下明文存储即可；真实系统应哈希
    int level = 1;       // 1..N
    bool isAdmin = false;
    double totalSpent = 0.0; // 用于升级等级

    // 每个用户一个购物车
    ShoppingCart cart;

public:
    User() = default;
    User(int id, string name, string pwd, int lvl, bool admin, double spent)
        : userID(id), username(std::move(name)), password(std::move(pwd)),
          level(lvl), isAdmin(admin), totalSpent(spent) {}

    // -------------------- User file I/O (single file for all users) --------------------
    // users file format (one line):
    // userID|username|password|level|isAdmin(0/1)|totalSpent
    static string usersFileName() { return "users.txt"; }

    static bool loadAll(vector<User>& users, int& nextUserID, const string& filename = usersFileName());
    static bool saveAll(const vector<User>& users, int nextUserID, const string& filename = usersFileName());

    // -------------------- Register / Login --------------------
    // register: create a new user with unique userID and username
    static bool registerUser(vector<User>& users, int& nextUserID,
                             const string& username, const string& password,
                             bool isAdmin = false);

    // login: return pointer to user in users vector (so you can modify and save back)
    static User* login(vector<User>& users, const string& username, const string& password);

    // -------------------- Cart binding --------------------
    string cartFileName() const { return ShoppingCart::getShoppingCartFileName(userID); }
    bool loadCartFromFile();
    bool saveCartToFile() const;

    // -------------------- Discount / Level --------------------
    double discountRate() const;          // e.g. 0.95 means 5% off
    void updateLevelBySpent();            // based on totalSpent

    // -------------------- Helper actions delegating to ShoppingCart --------------------
    void addToCart(int productID, const ProductManager& pm) { cart.addItem(productID, pm); }
    void updateCartItem(int productID, const ProductManager& pm) { cart.updateItem(productID, pm); }
    void removeFromCart(int productID) { cart.removeItem(productID); }
    void showCart(const ProductManager& pm) const { cart.displayCart(pm); }
    void clearCart() { cart.clearCart(); }

    // -------------------- Checkout (simple version; can be replaced by Transaction class) --------------------
    // behavior:
    // 1) check stock for every item in cart
    // 2) deduct stock from ProductManager
    // 3) compute total & apply discount
    // 4) append a simple record to user's transaction file
    // 5) clear cart + save cart + update user spent/level
    bool checkout(ProductManager& pm);

    // user transaction record file (one user one file)
    string transactionFileName() const { return "tx_" + to_string(userID) + ".txt"; }

private:
    static bool isUsernameValid(const string& name);
    static bool isPasswordValid(const string& pwd);

    static optional<User> parseUserLine(const string& line);
    static string toUserLine(const User& u);

    static bool usernameExists(const vector<User>& users, const string& username);

    // Safe split by delimiter
    static vector<string> split(const string& s, char delim);
};

#endif // ASSIGNMENT2_USER_H
