#include "User.h"
#include <algorithm>

// Initialize static member
vector<pair<string, string>> User::pendingAdmins;

// -------------------- small utilities --------------------
vector<string> User::split(const string& s, char delim) {
    vector<string> out;
    string cur;
    for (char c : s) {
        if (c == delim) { out.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    out.push_back(cur);
    return out;
}

bool User::isUsernameValid(const string& name) {
    if (name.empty()) return false;
    if (name.find('|') != string::npos) return false;
    return true;
}

bool User::isPasswordValid(const string& pwd) {
    if (pwd.size() < 4) return false;
    if (pwd.find('|') != string::npos) return false;
    return true;
}

bool User::usernameExists(const vector<User>& users, const string& username) {
    for (const auto& u : users) if (u.username == username) return true;
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

        // clamp level to 1..3
        if (lvl < 1) lvl = 1;
        if (lvl > 3) lvl = 3;
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
void User::createDefaultAdmin(vector<User>& users, int& nextUserID) {
    // Ensure nextUserID starts from at least 2
    if (nextUserID <= 1) nextUserID = 2;

    // Create default admin with fixed credentials
    users.emplace_back(1, "admin", "passwd123", 1, true, 0.0);
    cout << "Default admin account created (username: admin, password: passwd123)" << endl;
}
bool User::loadAll(vector<User>& users, int& nextUserID, const string& filename) {
    ifstream fin(filename);
    users.clear();
    nextUserID = 1;

    if (!fin.is_open()) {
        // File doesn't exist, create default admin
        createDefaultAdmin(users, nextUserID);
        return true;
    }

    string first;
    if (!getline(fin, first)) {
        // Empty file, create default admin
        createDefaultAdmin(users, nextUserID);
        return true;
    }
    if (!first.empty()) {
        try {
            nextUserID = stoi(first);
            if (nextUserID < 1) nextUserID = 1;
        } catch (...) { nextUserID = 1; }
    }

    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        auto u = parseUserLine(line);
        if (u.has_value()) users.push_back(*u);
    }

    // Check if default admin exists, if not, add it
    bool hasDefaultAdmin = false;
    for (const auto& u : users) {
        if (u.userID == 1 && u.username == "admin") {
            hasDefaultAdmin = true;
            break;
        }
    }
    if (!hasDefaultAdmin) {
        createDefaultAdmin(users, nextUserID);
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
    for (const auto& u : users) fout << toUserLine(u) << "\n";
    return true;
}
// -------------------- Admin Approval System --------------------
bool User::requestAdminAccess(const string& username, const string& password) {
    // Check if already requested
    for (const auto& req : pendingAdmins) {
        if (req.first == username) {
            cout << "Admin access already requested for this username." << endl;
            return false;
        }
    }

    pendingAdmins.emplace_back(username, password);
    cout << "Admin access request submitted. Waiting for approval." << endl;
    return true;
}

bool User::approveAdminRequest(vector<User>& users, int& nextUserID, int requestIndex) {
    if (requestIndex < 0 || requestIndex >= static_cast<int>(pendingAdmins.size())) {
        cout << "Invalid request index." << endl;
        return false;
    }

    auto& request = pendingAdmins[requestIndex];
    const string& username = request.first;
    const string& password = request.second;

    // Check if username still available
    if (usernameExists(users, username)) {
        cout << "Username '" << username << "' is no longer available." << endl;
        pendingAdmins.erase(pendingAdmins.begin() + requestIndex);
        return false;
    }

    // Create admin user
    int id = nextUserID++;
    users.emplace_back(id, username, password, 1, true, 0.0);
    cout << "Admin request approved. New admin userID=" << id << endl;

    // Remove from pending list
    pendingAdmins.erase(pendingAdmins.begin() + requestIndex);
    return true;
}

bool User::rejectAdminRequest(int requestIndex) {
    if (requestIndex < 0 || requestIndex >= static_cast<int>(pendingAdmins.size())) {
        cout << "Invalid request index." << endl;
        return false;
    }

    const auto& request = pendingAdmins[requestIndex];
    cout << "Admin request for '" << request.first << "' rejected." << endl;
    pendingAdmins.erase(pendingAdmins.begin() + requestIndex);
    return true;
}

void User::displayPendingRequests() {
    if (pendingAdmins.empty()) {
        cout << "No pending admin requests." << endl;
        return;
    }

    cout << "Pending Admin Requests:" << endl;
    for (size_t i = 0; i < pendingAdmins.size(); ++i) {
        cout << i << ") Username: " << pendingAdmins[i].first << endl;
    }
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

    if (isAdmin) {
        // Request admin access instead of direct registration
        return requestAdminAccess(username, password);
    }

    int id = nextUserID++;
    users.emplace_back(id, username, password, 1, false, 0.0);
    cout << "Register success. userID=" << id << endl;
    return true;
}

User* User::login(vector<User>& users, const string& username, const string& password) {
    for (auto& u : users) {
        if (u.username == username && u.password == password) {
            u.ensureTxmBound();
            cout << "Login success. userID=" << u.userID
                 << (u.isAdmin ? " (Admin)" : "") << endl;
            return &u;
        }
    }
    cout << "Login failed: wrong username or password." << endl;
    return nullptr;
}

// Forgot password (simple recovery)
bool User::resetPassword(vector<User>& users,
                         const string& username,
                         const string& newPassword) {
    if (!isPasswordValid(newPassword)) {
        cout << "Reset failed: invalid password (min length 4)." << endl;
        return false;
    }

    // Prevent password reset for default admin
    if (username == "admin") {
        cout << "Reset failed: Default admin password cannot be changed." << endl;
        return false;
    }

    for (auto& u : users) {
        if (u.username == username) {
            u.password = newPassword;
            cout << "Password reset success for username: " << username << endl;
            return true;
        }
    }
    cout << "Reset failed: username not found." << endl;
    return false;
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
    // match TransactionManager rules (3 levels)
    if (level <= 1) return 1.00; // Silver
    if (level == 2) return 0.98; // Gold
    return 0.95;                 // Diamond
}

void User::updateLevelBySpent() {
    // 3 levels only. Adjust thresholds if your rubric provides numbers.
    // Example:
    // < 500 : Silver
    // < 2000: Gold
    // >=2000: Diamond
    if (totalSpent >= 2000) level = 3;
    else if (totalSpent >= 500) level = 2;
    else level = 1;
}

// -------------------- Checkout (via TransactionManager) --------------------
bool User::checkout(ProductManager& pm) {
    if (isAdmin) {
        cout << "Admin account cannot checkout as a customer." << endl;
        return false;
    }
    if (cart.getItems().empty()) {
        cout << "Checkout failed: cart is empty." << endl;
        return false;
    }

    ensureTxmBound();

    // ensure latest tx records for this user (filtered total)
    txm.loadFromFile();
    double before = txm.getTotalSpent();

    bool ok = txm.processTransaction(cart, pm, level, isAdmin);

    // cart may be cleared or modified; persist
    saveCartToFile();

    if (!ok) return false;

    // compute delta from tx records
    double after = txm.getTotalSpent();
    double delta = after - before;
    if (delta < 0) delta = 0;

    totalSpent += delta;
    updateLevelBySpent();

    cout << "User totalSpent updated by: " << delta
         << " -> totalSpent=" << totalSpent
         << " level=" << levelName(level) << endl;

    return true;
}
