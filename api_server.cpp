// A minimal HTTP API on top of the exact same database, stored
// procedure, and service layer the console app (main.cpp) uses --
// nothing about the underlying logic changes, only how input comes in
// (JSON over HTTP instead of console prompts) and how output goes out
// (a JSON response with a status code instead of printed text).
//
// Endpoints:
//   POST /register          { "name", "email", "password" }
//   POST /login              { "email", "password" }
//   GET  /products
//   POST /orders              { "user_id", "product_id", "quantity" }
//   GET  /users/{id}/orders
//
// Deliberately left out (see README): auth tokens/sessions -- every
// request is independent and re-supplies whatever identifies the user
// (there's no login "session" to carry between requests, which a real
// production API would add via something like a JWT or a session
// cookie). HTTPS is also left out -- cpp-httplib only speaks plain HTTP
// here since enabling TLS support pulls in OpenSSL, which would
// reintroduce the exact kind of external-library linking risk this
// project has specifically been avoiding.

#include "httplib.h"
#include "nlohmann/json.hpp"
#include "service.hpp"

#include <iostream>

using json = nlohmann::json;

json productToJson(const Product& p) {
    return json{ {"id", p.id}, {"name", p.name}, {"price", p.price}, {"stock", p.stock} };
}

json orderToJson(const Order& o) {
    return json{ {"id", o.id}, {"total", o.total}, {"status", o.status}, {"created_at", o.createdAt} };
}

void sendError(httplib::Response& res, int status, const std::string& message) {
    res.status = status;
    res.set_content(json{ {"error", message} }.dump(), "application/json");
}

int main() {
    httplib::Server svr;

    svr.Post("/register", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            std::string name = body.at("name").get<std::string>();
            std::string email = body.at("email").get<std::string>();
            std::string password = body.at("password").get<std::string>();

            RegisterResult result = registerUser(name, email, password);
            res.status = result.success ? 201 : 400;
            res.set_content(json{ {"success", result.success}, {"message", result.message} }.dump(),
                             "application/json");
        } catch (const std::exception& e) {
            sendError(res, 400, std::string("Invalid request body: ") + e.what());
        }
    });

    svr.Post("/login", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            std::string email = body.at("email").get<std::string>();
            std::string password = body.at("password").get<std::string>();

            LoginResult result = loginUser(email, password);
            if (!result.success) {
                // Same deliberately vague message as the console app --
                // doesn't reveal whether the email exists or the
                // password was wrong, to resist account enumeration.
                sendError(res, 401, "Incorrect email or password");
                return;
            }

            res.status = 200;
            res.set_content(
                json{ {"id", result.user.id}, {"name", result.user.name}, {"email", result.user.email} }.dump(),
                "application/json");
        } catch (const std::exception& e) {
            sendError(res, 400, std::string("Invalid request body: ") + e.what());
        }
    });

    svr.Get("/products", [](const httplib::Request&, httplib::Response& res) {
        json arr = json::array();
        for (auto& p : listProducts()) arr.push_back(productToJson(p));
        res.set_content(arr.dump(), "application/json");
    });

    svr.Post("/orders", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            int userId = body.at("user_id").get<int>();
            int productId = body.at("product_id").get<int>();
            int quantity = body.at("quantity").get<int>();

            CheckoutResult result = checkout(userId, productId, quantity);

            if (result.status == "SUCCESS")               res.status = 201;
            else if (result.status == "INSUFFICIENT_STOCK") res.status = 409; // Conflict
            else if (result.status == "PRODUCT_NOT_FOUND")  res.status = 404;
            else                                              res.status = 500;

            res.set_content(json{ {"status", result.status} }.dump(), "application/json");
        } catch (const std::exception& e) {
            sendError(res, 400, std::string("Invalid request body: ") + e.what());
        }
    });

    // Matches /users/<digits>/orders and captures the id as matches[1].
    svr.Get(R"(/users/(\d+)/orders)", [](const httplib::Request& req, httplib::Response& res) {
        int userId = std::stoi(req.matches[1]);
        json arr = json::array();
        for (auto& o : getOrders(userId)) arr.push_back(orderToJson(o));
        res.set_content(arr.dump(), "application/json");
    });

    const int port = 8080;
    std::cout << "API server listening on http://localhost:" << port << "\n";
    std::cout << "Endpoints: POST /register, POST /login, GET /products, "
                 "POST /orders, GET /users/{id}/orders\n";
    std::cout << "Press Ctrl+C to stop.\n";

    svr.listen("0.0.0.0", port);
    return 0;
}
