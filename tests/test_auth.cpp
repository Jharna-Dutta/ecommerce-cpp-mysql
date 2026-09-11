#include "catch.hpp"
#include "../auth.hpp"

TEST_CASE("generateSalt produces a 32-character hex string by default") {
    std::string salt = generateSalt();
    REQUIRE(salt.size() == 32); // 16 bytes -> 32 hex characters
    for (char c : salt) {
        REQUIRE(((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')));
    }
}

TEST_CASE("generateSalt respects a custom byte count") {
    REQUIRE(generateSalt(8).size() == 16);
    REQUIRE(generateSalt(4).size() == 8);
}

TEST_CASE("generateSalt is different across calls") {
    // Not a rigorous randomness test, just a sanity check that we're not
    // accidentally returning a constant.
    std::string a = generateSalt();
    std::string b = generateSalt();
    REQUIRE(a != b);
}

TEST_CASE("hashPassword is deterministic for the same salt and password") {
    std::string salt = "fixed_salt_for_test";
    REQUIRE(hashPassword(salt, "hunter2") == hashPassword(salt, "hunter2"));
}

TEST_CASE("hashPassword changes if the password changes") {
    std::string salt = "fixed_salt_for_test";
    REQUIRE(hashPassword(salt, "hunter2") != hashPassword(salt, "different_password"));
}

TEST_CASE("hashPassword changes if the salt changes") {
    REQUIRE(hashPassword("salt_one", "same_password") !=
            hashPassword("salt_two", "same_password"));
}

TEST_CASE("hashPassword matches sha256Hex(salt + password) directly") {
    std::string salt = "abc123";
    std::string password = "myPassword!";
    REQUIRE(hashPassword(salt, password) == sha256Hex(salt + password));
}
