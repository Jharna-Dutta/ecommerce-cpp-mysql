#include "catch.hpp"
#include "../sha256.hpp"

// These are the official NIST/FIPS 180-4 test vectors for SHA-256 --
// the same three values that were checked by hand before this hash
// implementation was ever wired into the app's login/register logic.

TEST_CASE("sha256Hex matches the known hash of the empty string") {
    REQUIRE(sha256Hex("") ==
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST_CASE("sha256Hex matches the known hash of \"abc\"") {
    REQUIRE(sha256Hex("abc") ==
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST_CASE("sha256Hex matches the known hash of a longer string") {
    REQUIRE(sha256Hex("The quick brown fox jumps over the lazy dog") ==
        "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
}

TEST_CASE("sha256Hex is deterministic") {
    REQUIRE(sha256Hex("same input") == sha256Hex("same input"));
}

TEST_CASE("sha256Hex output is always 64 lowercase hex characters") {
    std::string h = sha256Hex("anything at all, any length");
    REQUIRE(h.size() == 64);
    for (char c : h) {
        REQUIRE(((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')));
    }
}
