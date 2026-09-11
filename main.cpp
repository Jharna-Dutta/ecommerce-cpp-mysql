#include <iostream>
#include <vector>
#include <string>
#include <limits>

#include "models.hpp"
#include "service.hpp"

using namespace std;

void printProducts(const vector<Product>& products) {
    cout << "\n--- Products ---\n";
    for (auto& p : products) {
        cout << " " << p.id << ". " << p.name
             << "  |  Rs " << p.price
             << "  |  Stock: " << p.stock << "\n";
    }
}

void printCheckoutResult(const CheckoutResult& result) {
    if (result.status == "SUCCESS")               cout << "Order placed successfully!\n";
    else if (result.status == "INSUFFICIENT_STOCK") cout << "Not enough stock available.\n";
    else if (result.status == "PRODUCT_NOT_FOUND")  cout << "No product with that id.\n";
    else                                             cout << "Checkout failed (" << result.status << ").\n";
}

void printOrders(const vector<Order>& orders) {
    cout << "\n--- Your Orders ---\n";
    if (orders.empty()) {
        cout << "No orders yet.\n";
        return;
    }
    for (auto& o : orders) {
        cout << " Order #" << o.id << "  |  Rs " << o.total
             << "  |  " << o.status << "  |  " << o.createdAt << "\n";
    }
}

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

            RegisterResult result = registerUser(name, email, password);
            cout << result.message << "\n";

        } else if (choice == 2) {
            string email, password;
            cout << "Email: ";    getline(cin, email);
            cout << "Password: "; getline(cin, password);

            LoginResult result = loginUser(email, password);
            if (result.success) {
                loggedIn = true;
                currentUser = result.user;
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
            printCheckoutResult(checkout(currentUser.id, pid, qty));

        } else if (choice == 3) {
            printOrders(getOrders(currentUser.id));

        } else if (choice == 4) {
            break;
        }
    }

    cout << "Goodbye!\n";
    return 0;
}
