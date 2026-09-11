#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>

struct Product {
    int id;
    std::string name;
    double price;
    int stock;
};

struct User {
    int id;
    std::string name;
    std::string email;
};

struct Order {
    int id;
    double total;
    std::string status;
    std::string createdAt;
};

#endif
