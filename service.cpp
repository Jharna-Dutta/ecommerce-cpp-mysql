#include "service.hpp"
#include "db.hpp"
#include "auth.hpp"

using namespace std;

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

RegisterResult registerUser(const string& name, const string& email, const string& password) {
    string salt = generateSalt();
    string hash = hashPassword(salt, password);
    // Note: the password itself never goes into the SQL string -- only its
    // hash and the salt do, and both are hex ([0-9a-f]), so escapeSql isn't
    // even needed for them.

    string q = "INSERT INTO users (name, email, salt, password_hash) VALUES ('"
             + escapeSql(name) + "', '" + escapeSql(email) + "', '" + salt + "', '" + hash + "');";
    string result = runQuery(q);

    if (result.find("ERROR") != string::npos) {
        return { false, "Registration failed (maybe that email is already used)." };
    }
    return { true, "Registered successfully." };
}

LoginResult loginUser(const string& email, const string& password) {
    string q = "SELECT id, name, email, salt, password_hash FROM users WHERE email='"
             + escapeSql(email) + "';";
    auto rows = parseRows(runQuery(q));

    if (rows.empty() || rows[0].size() < 5) {
        return { false, User{} };
    }

    const string& salt = rows[0][3];
    const string& storedHash = rows[0][4];
    string attemptHash = hashPassword(salt, password);

    if (attemptHash != storedHash) {
        return { false, User{} }; // wrong password
    }

    User u;
    u.id = stoi(rows[0][0]);
    u.name = rows[0][1];
    u.email = rows[0][2];
    return { true, u };
}

// Calls the checkout_order stored procedure (see schema.sql). All the
// "check stock, then decrement it, then create the order" logic runs as
// one atomic transaction inside MySQL itself, so it stays safe even if
// two customers are buying the same product at once.
CheckoutResult checkout(int userId, int productId, int quantity) {
    string q = "CALL checkout_order(" + to_string(userId) + ", " + to_string(productId)
             + ", " + to_string(quantity) + ", @status); SELECT @status AS status;";

    auto rows = parseRows(runQuery(q));
    string status = (!rows.empty() && !rows[0].empty()) ? rows[0][0] : "UNKNOWN_ERROR";
    return { status };
}

vector<Order> getOrders(int userId) {
    string q = "SELECT id, total, status, created_at FROM orders "
               "WHERE user_id = " + to_string(userId) + " ORDER BY id DESC;";
    auto rows = parseRows(runQuery(q));

    vector<Order> orders;
    for (auto& row : rows) {
        if (row.size() < 4) continue;
        Order o;
        o.id = stoi(row[0]);
        o.total = stod(row[1]);
        o.status = row[2];
        o.createdAt = row[3];
        orders.push_back(o);
    }
    return orders;
}
