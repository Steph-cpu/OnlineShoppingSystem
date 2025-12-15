#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <fstream>

#include "ProductManager.h"
#include "ShoppingCart.h"
#include "User.h"

using namespace std;

// -------------------- input helpers --------------------
static void clearBadInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        cout << prompt;
        int x;
        if (!(cin >> x)) {
            clearBadInput();
            cout << "Invalid input. Please enter an integer.\n";
            continue;
        }
        clearBadInput();
        if (x < lo || x > hi) {
            cout << "Out of range. Please enter between " << lo << " and " << hi << ".\n";
            continue;
        }
        return x;
    }
}

static double readDouble(const string& prompt, double loInclusive) {
    while (true) {
        cout << prompt;
        double x;
        if (!(cin >> x)) {
            clearBadInput();
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        clearBadInput();
        if (x < loInclusive) {
            cout << "Value must be >= " << loInclusive << ".\n";
            continue;
        }
        return x;
    }
}

static string readLine(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

// -------------------- enum choices --------------------
static Category chooseCategory() {
    cout << "Choose Category:\n";
    cout << "  0) Men\n  1) Women\n  2) Kids\n  3) Other\n";
    int c = readInt("Enter: ", 0, 3);
    return static_cast<Category>(c);
}

static Section chooseSection(Category cat) {
    if (cat == Category::Kids) {
        cout << "Choose Section (Kids):\n";
        cout << "  0) Boys\n  1) Girls\n  2) Other\n";
        int s = readInt("Enter: ", 0, 2);
        if (s == 0) return Section::Boys;
        if (s == 1) return Section::Girls;
        return Section::Other;
    }
    if (cat == Category::Other) {
        cout << "Section is fixed to Other for Category=Other.\n";
        return Section::Other;
    }
    // Men / Women
    cout << "Choose Section:\n";
    cout << "  0) Eastern\n  1) Western\n  2) Other\n";
    int s = readInt("Enter: ", 0, 2);
    if (s == 0) return Section::Eastern;
    if (s == 1) return Section::Western;
    return Section::Other;
}

static Size chooseSizeXSXL() {
    cout << "Choose Size:\n";
    cout << "  0) XS\n  1) S\n  2) M\n  3) L\n  4) XL\n";
    int s = readInt("Enter: ", 0, 4);
    return static_cast<Size>(s);
}

// -------------------- small UI helpers --------------------
static void pauseEnter() {
    cout << "Press ENTER to continue...";
    string dummy;
    getline(cin, dummy);
}

static void showUserInfo(const User& u) {
    cout << "\n=== User Info ===\n";
    cout << "userID: " << u.userID << "\n";
    cout << "username: " << u.username << "\n";
    cout << "level: " << u.level << "\n";
    cout << "isAdmin: " << (u.isAdmin ? "Yes" : "No") << "\n";
    cout << "totalSpent: " << u.totalSpent << "\n";
    cout << "discountRate: " << u.discountRate() << "\n";
}

// -------------------- admin menu actions --------------------
static void adminMenu(ProductManager& pm) {
    const string productFile = "products.txt";

    while (true) {
        cout << "\n===== ADMIN MENU =====\n";
        cout << "1) Display all products\n";
        cout << "2) Display by category\n";
        cout << "3) Display by section\n";
        cout << "4) Display single product\n";
        cout << "5) Add product (interactive stock input)\n";
        cout << "6) Remove product\n";
        cout << "7) Update product price\n";
        cout << "8) Update product stock (size or None)\n";
        cout << "9) Update product name\n";
        cout << "10) Update product category/section\n";
        cout << "11) Save products to file\n";
        cout << "12) Load products from file\n";
        cout << "0) Back\n";

        int op = readInt("Choose: ", 0, 12);

        if (op == 0) return;

        switch (op) {
            case 1: {
                pm.displayAllProducts();
                pauseEnter();
                break;
            }
            case 2: {
                Category cat = chooseCategory();
                pm.displayByCategory(cat);
                pauseEnter();
                break;
            }
            case 3: {
                Category cat = chooseCategory();
                Section sec = chooseSection(cat);
                pm.displayBySection(cat, sec);
                pauseEnter();
                break;
            }
            case 4: {
                int id = readInt("Enter productID: ", 1, 1000000000);
                pm.displaySingleProduct(id);
                pauseEnter();
                break;
            }
            case 5: {
                string name = readLine("Enter product name (no comma recommended): ");
                Category cat = chooseCategory();
                Section sec = chooseSection(cat);
                double price = readDouble("Enter price: ", 0.0);
                int newID = pm.addProduct(name, cat, sec, price); // will prompt stock inside
                cout << "addProduct result ID = " << newID << "\n";
                pauseEnter();
                break;
            }
            case 6: {
                int id = readInt("Enter productID to remove: ", 1, 1000000000);
                bool ok = pm.removeProduct(id);
                cout << (ok ? "Removed.\n" : "Remove failed.\n");
                pauseEnter();
                break;
            }
            case 7: {
                int id = readInt("Enter productID: ", 1, 1000000000);
                double price = readDouble("Enter new price: ", 0.0);
                bool ok = pm.updateProduct(id, price);
                cout << (ok ? "Updated.\n" : "Update failed.\n");
                pauseEnter();
                break;
            }
            case 8: {
                int id = readInt("Enter productID: ", 1, 1000000000);
                Product* p = pm.getProduct(id);
                if (!p) {
                    cout << "Product not found.\n";
                    pauseEnter();
                    break;
                }
                Size sz = Size::None;
                if (p->getHasSize()) {
                    sz = chooseSizeXSXL();
                } else {
                    cout << "This product has no sizes; using None.\n";
                    sz = Size::None;
                }
                int newStock = readInt("Enter new stock (>=0): ", 0, 1000000000);
                bool ok = pm.updateProduct(id, sz, newStock);
                cout << (ok ? "Updated.\n" : "Update failed.\n");
                pauseEnter();
                break;
            }
            case 9: {
                int id = readInt("Enter productID: ", 1, 1000000000);
                string newName = readLine("Enter new name: ");
                bool ok = pm.updateProduct(id, newName);
                cout << (ok ? "Updated.\n" : "Update failed.\n");
                pauseEnter();
                break;
            }
            case 10: {
                int id = readInt("Enter productID: ", 1, 1000000000);
                Category newCat = chooseCategory();
                Section newSec = chooseSection(newCat);
                bool ok = pm.updateProduct(id, newCat, newSec);
                cout << (ok ? "Updated.\n" : "Update failed.\n");
                pauseEnter();
                break;
            }
            case 11: {
                bool ok = pm.saveToFile(productFile);
                cout << (ok ? "Saved to products.txt\n" : "Save failed.\n");
                pauseEnter();
                break;
            }
            case 12: {
                bool ok = pm.loadFromFile(productFile);
                cout << (ok ? "Loaded from products.txt\n" : "Load failed.\n");
                pauseEnter();
                break;
            }
            default:
                break;
        }
    }
}

// -------------------- user menu actions --------------------
static void showTransactionFile(const User& u) {
    ifstream fin(u.transactionFileName());
    cout << "\n=== Transaction Records (" << u.transactionFileName() << ") ===\n";
    if (!fin.is_open()) {
        cout << "(No transaction file yet.)\n";
        return;
    }
    string line;
    while (getline(fin, line)) {
        cout << line << "\n";
    }
}

static void userMenu(User& u, ProductManager& pm) {
    // 登录后加载购物车
    u.loadCartFromFile();

    while (true) {
        cout << "\n===== USER MENU (" << u.username << ") =====\n";
        cout << "1) View all products\n";
        cout << "2) View products by category\n";
        cout << "3) View products by section\n";
        cout << "4) View single product\n";
        cout << "5) Add item to cart\n";
        cout << "6) Update item in cart\n";
        cout << "7) Remove product from cart\n";
        cout << "8) View cart\n";
        cout << "9) Checkout\n";
        cout << "10) View my transactions\n";
        cout << "11) View my user info\n";
        cout << "0) Logout\n";

        int op = readInt("Choose: ", 0, 11);

        if (op == 0) {
            // 退出前保存购物车
            u.saveCartToFile();
            cout << "Logged out.\n";
            return;
        }

        switch (op) {
            case 1: {
                pm.displayAllProducts();
                pauseEnter();
                break;
            }
            case 2: {
                Category cat = chooseCategory();
                pm.displayByCategory(cat);
                pauseEnter();
                break;
            }
            case 3: {
                Category cat = chooseCategory();
                Section sec = chooseSection(cat);
                pm.displayBySection(cat, sec);
                pauseEnter();
                break;
            }
            case 4: {
                int id = readInt("Enter productID: ", 1, 1000000000);
                pm.displaySingleProduct(id);
                pauseEnter();
                break;
            }
            case 5: {
                int id = readInt("Enter productID to add: ", 1, 1000000000);
                u.addToCart(id, pm);     // ShoppingCart will prompt quantity/size
                u.saveCartToFile();
                pauseEnter();
                break;
            }
            case 6: {
                int id = readInt("Enter productID to update: ", 1, 1000000000);
                u.updateCartItem(id, pm); // ShoppingCart will prompt
                u.saveCartToFile();
                pauseEnter();
                break;
            }
            case 7: {
                int id = readInt("Enter productID to remove from cart: ", 1, 1000000000);
                u.removeFromCart(id);
                u.saveCartToFile();
                pauseEnter();
                break;
            }
            case 8: {
                u.showCart(pm);
                pauseEnter();
                break;
            }
            case 9: {
                bool ok = u.checkout(pm); // deduct stock + write tx + clear cart
                if (ok) {
                    // checkout 成功后建议及时保存 products
                    cout << "Tip: Ask admin to save products to file if you want persistence.\n";
                }
                pauseEnter();
                break;
            }
            case 10: {
                showTransactionFile(u);
                pauseEnter();
                break;
            }
            case 11: {
                showUserInfo(u);
                pauseEnter();
                break;
            }
            default:
                break;
        }
    }
}

// -------------------- main: entry menu --------------------
int main() {
    ProductManager pm;
    const string productFile = "products.txt";

    // 尝试加载 products（没有文件也没关系）
    pm.loadFromFile(productFile);

    vector<User> users;
    int nextUserID = 1;
    User::loadAll(users, nextUserID);

    while (true) {
        cout << "\n===== ONLINE SHOPPING SYSTEM =====\n";
        cout << "1) Register\n";
        cout << "2) Login\n";
        cout << "3) Admin Login (uses normal login; must be admin)\n";
        cout << "4) Save & Exit\n";

        int op = readInt("Choose: ", 1, 4);

        if (op == 1) {
            string name = readLine("Username: ");
            string pwd  = readLine("Password (min 4 chars): ");
            int adminFlag = readInt("Register as admin? (0/1): ", 0, 1);
            bool ok = User::registerUser(users, nextUserID, name, pwd, adminFlag == 1);
            if (ok) User::saveAll(users, nextUserID);
            pauseEnter();
        }
        else if (op == 2) {
            string name = readLine("Username: ");
            string pwd  = readLine("Password: ");
            User* u = User::login(users, name, pwd);
            if (!u) {
                pauseEnter();
                continue;
            }
            // 普通用户也可以进入 userMenu（管理员也可以）
            userMenu(*u, pm);

            // userMenu 里 checkout 可能更新 totalSpent/level，所以要保存 users
            User::saveAll(users, nextUserID);
            pauseEnter();
        }
        else if (op == 3) {
            string name = readLine("Admin username: ");
            string pwd  = readLine("Admin password: ");
            User* u = User::login(users, name, pwd);
            if (!u) {
                pauseEnter();
                continue;
            }
            if (!u->isAdmin) {
                cout << "Access denied: not an admin.\n";
                pauseEnter();
                continue;
            }
            adminMenu(pm);

            // 管理员操作完 products，建议保存
            pm.saveToFile(productFile);
            pauseEnter();
        }
        else if (op == 4) {
            // 保存并退出
            User::saveAll(users, nextUserID);
            pm.saveToFile(productFile);
            cout << "Saved. Bye!\n";
            break;
        }
    }

    return 0;
}