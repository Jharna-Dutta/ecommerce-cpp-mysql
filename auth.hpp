#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>
#include <random>
#include <sstream>
#include <iomanip>

#include "sha256.hpp"

// Pulled out of main.cpp into its own header so it can be unit-tested
// independently of the console app and the database.

// Generates a random salt as a hex string (numBytes*2 hex characters).
// Uses random_device to seed, which pulls from the OS's entropy source.
inline std::string generateSalt(size_t numBytes = 16) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < numBytes; ++i) {
        oss << std::setw(2) << dist(gen);
    }
    return oss.str();
}

// Combines a salt and a plaintext password into the stored hash.
// Separated out mainly so tests (and any future call site) don't have to
// know/repeat the exact "salt + password" concatenation convention.
inline std::string hashPassword(const std::string& salt, const std::string& password) {
    return sha256Hex(salt + password);
}

#endif
