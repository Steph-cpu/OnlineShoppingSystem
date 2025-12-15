#include "User.h"

// -------------------- small utilities --------------------
vector<string> User::split(const string& s, char delim) {
    vector<string> out;
    string cur;
    for (char c : s) {
        if (c == delim) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

bool User::isUsernameValid(const string& name) {
    // 你 README 对 productName 限制更严格；userName 这里给一个温和规则：
    // 非空、不能包含分隔符 '|'
    if (name.empty()) return false;
    if (name.find('|') != string::npos) return false;
    return true;
}

bool User::isPasswordValid(const string& pwd) {
    if (pwd.size() < 4) return false;          // 作业里最低 4
    if (pwd.find('|') != string::npos) return false;
    return true;
}

bool User::usernameExists(const vector<User>& users, const string& username) {
    for (const auto& u : users) {
        if (u.username == username) return true;
    }
    return false;
}

// users file line format:
// userID|username|password|level|isAdmin|totalSpent
optional<User> User::parseUserLine(const string& line) {
    auto parts = split(line, '|');
    if (parts.size() != 6) return nullopt;
    try {
        int id = stoi(parts[0]);
        string name = parts[1];
        string pwd = parts[2];
        int lvl = stoi(parts[3]);
        bool admin = (stoi(parts[4]) != 0);
        double spent = stod(parts[5]);

        if (id <= 0) return nullopt;
        if (!isUsernameValid(name) || !isPasswordValid(pwd)) return nullopt;
        if (lvl < 1) lvl = 1;
        if (spent < 0) spent = 0;

        return User(id, name, pwd, lvl, admin, spent);
    } catch (...) {
        return nullopt;
    }
}

string User::toUserLine(const User& u) {
    return to_string(u.userID) + "|" + u.username + "|" + u.password + "|" +
           to_string(u.level) + "|" + (u.isAdmin ? "1" : "0") + "|" +
           to_string(u.totalSpent);
}

// -------------------- Load / Save all users --------------------
bool User::loadAll(vector<User>& users, int& nextUserID, const string& filename) {
    ifstream fin(filename);
    users.clear();
    nextUserID = 1;

    if (!fin.is_open()) {
        // 文件不存在也算“正常”：第一次运行
        return true;
    }

    // first line: nextUserID
    string first;
    if (!getline(fin, first)) return true;
    if (!first.empty()) {
        try {
            nextUserID = stoi(first);
            if (nextUserID < 1) nextUserID = 1;
        } catch (...) {
            nextUserID = 1;
        }
    }

    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        auto u = parseUserLine(line);
        if (u.has_value()) {
            users.push_back(*u);
        }
    }
    return true;
}

bool User::saveAll(const vector<User>& users, int nextUserID, const string& filename) {
    ofstream fout(filename);
    if (!fout.is_open()) {
        cout << "Failed to open users file for writing: " << filename << endl;
        return false;
    }
    fout << nextUserID << "\n";
    for (const auto& u : users) {
        fout << toUserLine(u) << "\n";
    }
    return true;
}

// -------------------- Register / Login --------------------
bool User::registerUser(vector<User>& users, int& nextUserID,
                        const string& username, const string& password,
                        bool isAdmin) {
    if (!isUsernameValid(username)) {
        cout << "Register failed: invalid username." << endl;
        return false;
    }
    if (!isPasswordValid(password)) {
        cout << "Register failed: invalid password (min length 4)." << endl;
        return false;
    }
    if (usernameExists(users, username)) {
        cout << "Register failed: username already exists." << endl;
        return false;
    }

    int id = nextUserID++;
    users.emplace_back(id, username, password, 1, isAdmin, 0.0);
    cout << "Register success. userID=" << id << endl;
    return true;
}

User* User::login(vector<User>& users, const string& username, const string& password) {
    for (auto& u : users) {
        if (u.username == username && u.password == password) {
            cout << "Login success. userID=" << u.userID
                 << (u.isAdmin ? " (Admin)" : "") << endl;
            return &u;
        }
    }
    cout << "Login failed: wrong username or password." << endl;
    return nullptr;
}

// -------------------- Cart file binding --------------------
bool User::loadCartFromFile() {
    return cart.loadFromFile(cartFileName());
}

bool User::saveCartToFile() const {
    return cart.saveToFile(cartFileName());
}

// -------------------- Discount / Level --------------------
double User::discountRate() const {
    // 简单等级折扣：你可随意改成作业要求的规则
    // level 1: 100%
    // level 2: 98%
    // level 3: 95%
    // level 4+: 90%
    if (isAdmin) return 0.80;          // 管理员演示：更低折扣（可删）
    if (level <= 1) return 1.00;
    if (level == 2) return 0.98;
    if (level == 3) return 0.95;
    return 0.90;
}

void User::updateLevelBySpent() {
    // 基于 totalSpent 的自动升级规则（可调整）
    // >= 500 -> level 2
    // >= 2000 -> level 3
    // >= 5000 -> level 4
    if (totalSpent >= 5000) level = max(level, 4);
    else if (totalSpent >= 2000) level = max(level, 3);
    else if (totalSpent >= 500) level = max(level, 2);
    else level = max(level, 1);
}

// -------------------- Checkout (simple) --------------------
bool User::checkout(ProductManager& pm) {
    const auto& cartItems = cart.getItems();
    if (cartItems.empty()) {
        cout << "Checkout failed: cart is empty." << endl;
        return false;
    }

    // 1) check stock for every item (all-or-nothing)
    for (const auto& [productID, qtyVec] : cartItems) {
        const Product* p = pm.getProduct(productID);
        if (!p) {
            cout << "Checkout failed: product not found, ID=" << productID << endl;
            return false;
        }
        const auto& stock = p->getSizeStock();
        bool hasSize = p->getHasSize();

        for (int i = 0; i < 6; ++i) {
            int q = (i < (int)qtyVec.size()) ? qtyVec[i] : 0;
            if (q <= 0) continue;

            Size sz = static_cast<Size>(i);
            if (!hasSize && sz != Size::None) {
                cout << "Checkout failed: product has no size but cart contains sized quantity. ID="
                     << productID << endl;
                return false;
            }
            if (hasSize && sz == Size::None) {
                cout << "Checkout failed: sized product but cart contains None quantity. ID="
                     << productID << endl;
                return false;
            }
            if (q > stock[i]) {
                cout << "Checkout failed: insufficient stock. ProductID=" << productID
                     << " size=" << sizeToString(sz)
                     << " need=" << q << " available=" << stock[i] << endl;
                return false;
            }
        }
    }

    // 2) compute total
    double rawTotal = cart.calculateTotal(pm);
    double rate = discountRate();
    double finalTotal = rawTotal * rate;

    // 3) deduct stock (now safe)
    for (const auto& [productID, qtyVec] : cartItems) {
        Product* p = pm.getProduct(productID);
        if (!p) return false; // should not happen
        for (int i = 0; i < 6; ++i) {
            int q = (i < (int)qtyVec.size()) ? qtyVec[i] : 0;
            if (q <= 0) continue;
            // updateStock(size, -q)
            if (!p->updateStock(static_cast<Size>(i), -q)) {
                cout << "Checkout failed: stock deduction error. ID=" << productID << endl;
                return false;
            }
        }
    }

    // 4) append a simple transaction record
    {
        ofstream tx(transactionFileName(), ios::app);
        if (tx.is_open()) {
            tx << "USER=" << userID
               << " RAW=" << rawTotal
               << " RATE=" << rate
               << " FINAL=" << finalTotal
               << "\n";
            // 可选：把每个商品也写进去
            for (const auto& [productID, qtyVec] : cartItems) {
                tx << "  PID=" << productID;
                for (int i = 0; i < 6; ++i) {
                    int q = (i < (int)qtyVec.size()) ? qtyVec[i] : 0;
                    if (q > 0) tx << " " << sizeToString(static_cast<Size>(i)) << "=" << q;
                }
                tx << "\n";
            }
        }
    }

    // 5) update user spent & level, clear cart
    totalSpent += finalTotal;
    updateLevelBySpent();

    cart.clearCart();
    saveCartToFile(); // 结算后购物车清空写回

    cout << "Checkout success. raw=" << rawTotal
         << " discountRate=" << rate
         << " final=" << finalTotal
         << " newLevel=" << level << endl;

    return true;
}
