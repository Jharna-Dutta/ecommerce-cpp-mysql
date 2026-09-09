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

#endif
