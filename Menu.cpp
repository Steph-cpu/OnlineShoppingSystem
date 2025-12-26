#include <iostream>
#include <string>
#include <vector>
#include <limits>

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
    cout << "level: " << User::levelName(u.level) << "\n";
    cout << "isAdmin: " << (u.isAdmin ? "Yes" : "No") << "\n";
    cout << "totalSpent: " << u.totalSpent << "\n";
    cout << "discountRate: " << u.discountRate() << "\n";
}

// -------------------- admin transaction view (NEW) --------------------
static void adminTransactionsMenu() {
    TransactionManager adminTM(-1); // -1 => no filter, view all
    adminTM.loadFromFile();

    while (true) {
        cout << "\n===== ADMIN: TRANSACTION RECORDS =====\n";
        cout << "1) Summary (all users)\n";
        cout << "2) Show all invoices (all users)\n";
        cout << "3) Show one invoice by Transaction ID\n";
        cout << "0) Back\n";

        int op = readInt("Choose: ", 0, 3);
        if (op == 0) return;

        switch (op) {
            case 1:
                adminTM.displayTransactionSummary();
                pauseEnter();
                break;
            case 2:
                adminTM.displayAllTransactions();
                pauseEnter();
                break;
            case 3: {
                int txid = readInt("Enter Transaction ID: ", 1, 1000000000);
                adminTM.displayTransaction(txid);
                pauseEnter();
                break;
            }
            default:
                break;
        }
    }
}

// -------------------- admin menu actions --------------------
// Note: adminMenu now takes additional parameters for user management
static void adminMenu(ProductManager& pm, vector<User>& users, int& nextUserID) {
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
        cout << "13) View transaction records (TransactionRecord.txt)\n";
        cout << "14) Manage admin requests\n"; // NEW
        cout << "0) Back\n";

        int op = readInt("Choose: ", 0, 14);
        if (op == 0) return;

        switch (op) {
            case 1: pm.displayAllProducts(); pauseEnter(); break;
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
                int newID = pm.addProduct(name, cat, sec, price);
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
                if (p->getHasSize()) sz = chooseSizeXSXL();
                else {
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
            case 13: {
                adminTransactionsMenu();
                break;
            }
            case 14: {
                // Manage admin requests
                while (true) {
                    cout << "\n===== ADMIN REQUEST MANAGEMENT =====\n";
                    User::displayPendingRequests();
                    if (!User::hasPendingRequests()) {
                        pauseEnter();
                        break;
                    }
                    cout << "\nOptions:\n";
                    cout << "1) Approve request\n";
                    cout << "2) Reject request\n";
                    cout << "0) Back\n";

                    int subOp = readInt("Choose: ", 0, 2);
                    if (subOp == 0) break;

                    int reqIdx = readInt("Enter request index: ", 0, 100);
                    bool success = false;
                    if (subOp == 1) {
                        success = User::approveAdminRequest(users, nextUserID, reqIdx);
                        if (success) User::saveAll(users, nextUserID);
                    } else if (subOp == 2) {
                        success = User::rejectAdminRequest(reqIdx);
                    }
                    if (success) pauseEnter();
                }
                break;
            }
            default:
                break;
        }
    }
}

// -------------------- transaction menu (user) --------------------
static void showTransactionsMenu(User& u) {
    while (true) {
        cout << "\n===== MY TRANSACTIONS =====\n";
        cout << "1) Summary (brief)\n";
        cout << "2) Show all (full invoices)\n";
        cout << "3) Show one by Transaction ID\n";
        cout << "0) Back\n";

        int op = readInt("Choose: ", 0, 3);
        if (op == 0) return;

        switch (op) {
            case 1: u.displayTransactionSummary(); pauseEnter(); break;
            case 2: u.displayAllTransactions(); pauseEnter(); break;
            case 3: {
                int txid = readInt("Enter Transaction ID: ", 1, 1000000000);
                u.displayTransaction(txid);
                pauseEnter();
                break;
            }
            default:
                break;
        }
    }
}

// -------------------- user menu actions --------------------
static void userMenu(User& u, ProductManager& pm) {
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
            u.saveCartToFile();
            cout << "Logged out.\n";
            return;
        }

        switch (op) {
            case 1: pm.displayAllProducts(); pauseEnter(); break;
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
                u.addToCart(id, pm);
                u.saveCartToFile();
                pauseEnter();
                break;
            }
            case 6: {
                int id = readInt("Enter productID to update: ", 1, 1000000000);
                u.updateCartItem(id, pm);
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
            case 8: u.showCart(pm); pauseEnter(); break;
            case 9: {
                bool ok = u.checkout(pm);
                if (ok) cout << "Checkout finished.\n";
                pauseEnter();
                break;
            }
            case 10: {
                showTransactionsMenu(u);
                break;
            }
            case 11: showUserInfo(u); pauseEnter(); break;
            default:
                break;
        }
    }
}

// -------------------- main: entry menu --------------------
int main() {
    ProductManager pm;
    const string productFile = "products.txt";

    pm.loadFromFile(productFile);

    vector<User> users;
    int nextUserID = 1;
    User::loadAll(users, nextUserID);

    while (true) {
        cout << "\n===== ONLINE SHOPPING SYSTEM =====\n";
        cout << "1) Register\n";
        cout << "2) Login\n";
        cout << "3) Forgot password\n"; // NEW
        cout << "4) Admin Login\n";
        cout << "5) Save & Exit\n";

        int op = readInt("Choose: ", 1, 5);

        if (op == 1) {
            string name = readLine("Username: ");
            string pwd  = readLine("Password (min 4 chars): ");
            int adminFlag = readInt("Request admin privileges? (0=No, 1=Yes - requires approval): ", 0, 1);
            bool ok = User::registerUser(users, nextUserID, name, pwd, adminFlag == 1);
            if (ok && adminFlag == 0) User::saveAll(users, nextUserID); // Only save immediately for regular users
            pauseEnter();
        }
        else if (op == 2) {
            string name = readLine("Username: ");
            string pwd  = readLine("Password: ");
            User* u = User::login(users, name, pwd);
            if (!u) { pauseEnter(); continue; }

            if (u->isAdmin) {
                cout << "This is an admin account. Please use Admin Login option.\n";
                pauseEnter();
                continue;
            }

            userMenu(*u, pm);
            User::saveAll(users, nextUserID);
            pauseEnter();
        }
        else if (op == 3) {
            string name = readLine("Enter username: ");
            string newPwd = readLine("Enter new password (min 4 chars): ");
            bool ok = User::resetPassword(users, name, newPwd);
            if (ok) User::saveAll(users, nextUserID);
            pauseEnter();
        }
        else if (op == 4) {
            string name = readLine("Admin username: ");
            string pwd  = readLine("Admin password: ");
            User* u = User::login(users, name, pwd);
            if (!u) { pauseEnter(); continue; }
            if (!u->isAdmin) {
                cout << "Access denied: not an admin.\n";
                pauseEnter();
                continue;
            }
            adminMenu(pm, users, nextUserID);
            pm.saveToFile(productFile);
            User::saveAll(users, nextUserID);
            pauseEnter();
        }
        else if (op == 5) {
            User::saveAll(users, nextUserID);
            pm.saveToFile(productFile);
            cout << "Saved. Bye!\n";
            break;
        }
    }

    return 0;
}
