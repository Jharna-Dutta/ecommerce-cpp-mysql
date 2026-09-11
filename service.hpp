#ifndef SERVICE_HPP
#define SERVICE_HPP

// The business logic layer: everything that actually talks to the
// database. Both the console app (main.cpp) and the HTTP API
// (api_server.cpp) call these same functions -- neither one duplicates
// this logic, they just differ in how they get input and how they
// present the result (printed text vs. JSON).

#include <string>
#include <vector>
#include "models.hpp"

struct RegisterResult {
    bool success;
    std::string message; // human-readable reason, mainly useful on failure
};

struct LoginResult {
    bool success;
    User user; // only meaningful when success == true
};

// Mirrors the status strings the checkout_order stored procedure returns
// (see schema.sql): "SUCCESS", "INSUFFICIENT_STOCK", "PRODUCT_NOT_FOUND",
// "DB_ERROR", or "UNKNOWN_ERROR" if the call itself couldn't be parsed.
struct CheckoutResult {
    std::string status;
};

std::vector<Product> listProducts();
RegisterResult registerUser(const std::string& name, const std::string& email, const std::string& password);
LoginResult loginUser(const std::string& email, const std::string& password);
CheckoutResult checkout(int userId, int productId, int quantity);
std::vector<Order> getOrders(int userId);

#endif
