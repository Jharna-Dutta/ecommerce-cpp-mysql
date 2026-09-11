#include <iostream>
#include <vector>
#include <string>
#include <limits>

#include "db.hpp"
#include "models.hpp"
#include "auth.hpp"

using namespace std;

// ---- data access helpers -------------------------------------------------

vector<Product> listProducts() {
    string raw = runQuery("SELECT id, name, price, stock FROM products ORDER BY id;");
    auto rows = parseRows(raw);

    vector<Product> products;
    for (auto& row : rows) {
        if (row.size() < 4) continue; // a malformed/error row, skip it
        Product p;
        p.id = stoi(row[0]);
        p.name = row[1];
        p.price = stod(row[2]);
        p.stock = stoi(row[3]);
        products.push_back(p);
    }
    return products;
}

void printProducts(const vector<Product>& products) {
    cout << "\n--- Products ---\n";
    for (auto& p : products) {
        cout << " " << p.id << ". " << p.name
             << "  |  Rs " << p.price
             << "  |  Stock: " << p.stock << "\n";
    }
}

bool registerUser(const string& name, const string& email, const string& password) {
    string salt = generateSalt();
    string hash = hashPassword(salt, password);
    // Note: the password itself never goes into the SQL string -- only its
    // hash and the salt do, and both are hex ([0-9a-f]), so escapeSql isn't
    // even needed for them.

    string q = "INSERT INTO users (name, email, salt, password_hash) VALUES ('"
             + escapeSql(name) + "', '" + escapeSql(email) + "', '" + salt + "', '" + hash + "');";
    string result = runQuery(q);

    if (result.find("ERROR") != string::npos) {
        cout << "Registration failed (maybe that email is already used):\n" << result << "\n";
        return false;
    }
    cout << "Registered successfully. You can log in now.\n";
    return true;
}

bool login(const string& email, const string& password, User& outUser) {
    string q = "SELECT id, name, email, salt, password_hash FROM users WHERE email='"
             + escapeSql(email) + "';";
    auto rows = parseRows(runQuery(q));
    if (rows.empty() || rows[0].size() < 5) return false;

    const string& salt = rows[0][3];
    const string& storedHash = rows[0][4];
    string attemptHash = hashPassword(salt, password);

    if (attemptHash != storedHash) return false; // wrong password

    outUser.id = stoi(rows[0][0]);
    outUser.name = rows[0][1];
    outUser.email = rows[0][2];
    return true;
}

// Calls the checkout_order stored procedure (see schema.sql). All the
// "check stock, then decrement it, then create the order" logic runs
// as one atomic transaction inside MySQL itself, so it stays safe even
// if you had two customers buying the same product at once.
void buyProduct(int userId, int productId, int quantity) {
    string q = "CALL checkout_order(" + to_string(userId) + ", " + to_string(productId)
             + ", " + to_string(quantity) + ", @status); SELECT @status AS status;";

    auto rows = parseRows(runQuery(q));
    string status = (!rows.empty() && !rows[0].empty()) ? rows[0][0] : "UNKNOWN_ERROR";

    if (status == "SUCCESS")               cout << "Order placed successfully!\n";
    else if (status == "INSUFFICIENT_STOCK") cout << "Not enough stock available.\n";
    else if (status == "PRODUCT_NOT_FOUND")  cout << "No product with that id.\n";
    else                                      cout << "Checkout failed (" << status << ").\n";
}

void viewOrders(int userId) {
    string q = "SELECT id, total, status, created_at FROM orders "
               "WHERE user_id = " + to_string(userId) + " ORDER BY id DESC;";
    auto rows = parseRows(runQuery(q));

    cout << "\n--- Your Orders ---\n";
    if (rows.empty()) {
        cout << "No orders yet.\n";
        return;
    }
    for (auto& row : rows) {
        if (row.size() < 4) continue;
        cout << " Order #" << row[0] << "  |  Rs " << row[1]
             << "  |  " << row[2] << "  |  " << row[3] << "\n";
    }
}

// ---- menu / main ----------------------------------------------------------

int readInt() {
    int value;
    while (!(cin >> value)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Please enter a number: ";
    }
    return value;
}

int main() {
    cout << "=== Simple E-Commerce Backend (C++ + MySQL) ===\n";

    User currentUser;
    bool loggedIn = false;

    while (!loggedIn) {
        cout << "\n1. Register\n2. Login\n3. Exit\nChoice: ";
        int choice = readInt();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            string name, email, password;
            cout << "Name: ";     getline(cin, name);
            cout << "Email: ";    getline(cin, email);
            cout << "Password: "; getline(cin, password);
            registerUser(name, email, password);

        } else if (choice == 2) {
            string email, password;
            cout << "Email: ";    getline(cin, email);
            cout << "Password: "; getline(cin, password);
            if (login(email, password, currentUser)) {
                loggedIn = true;
                cout << "Welcome, " << currentUser.name << "!\n";
            } else {
                // Deliberately vague: don't reveal whether the email exists
                // or the password was wrong -- telling an attacker which one
                // failed makes it easier to enumerate valid accounts.
                cout << "Incorrect email or password.\n";
            }

        } else if (choice == 3) {
            return 0;
        }
    }

    while (true) {
        cout << "\n1. Browse products\n2. Buy a product\n3. View my orders\n4. Exit\nChoice: ";
        int choice = readInt();

        if (choice == 1) {
            printProducts(listProducts());

        } else if (choice == 2) {
            printProducts(listProducts());
            cout << "Enter product id: ";
            int pid = readInt();
            cout << "Enter quantity: ";
            int qty = readInt();
            buyProduct(currentUser.id, pid, qty);

        } else if (choice == 3) {
            viewOrders(currentUser.id);

        } else if (choice == 4) {
            break;
        }
    }

    cout << "Goodbye!\n";
    return 0;
}
