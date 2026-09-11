#include "catch.hpp"
#include "../db.hpp"

// runQuery() itself isn't tested here -- it shells out to a live mysql
// process, which makes it an integration point rather than something a
// unit test should call. escapeSql() and parseRows() are pure functions
// with no I/O, so they're the parts worth covering directly.

TEST_CASE("escapeSql doubles single quotes") {
    REQUIRE(escapeSql("O'Brien") == "O''Brien");
}

TEST_CASE("escapeSql strips semicolons") {
    REQUIRE(escapeSql("a;b") == "ab");
}

TEST_CASE("escapeSql strips backslashes") {
    REQUIRE(escapeSql("a\\b") == "ab");
}

TEST_CASE("escapeSql strips double quotes") {
    REQUIRE(escapeSql("a\"b") == "ab");
}

TEST_CASE("escapeSql leaves ordinary text untouched") {
    REQUIRE(escapeSql("Jharna Dutta") == "Jharna Dutta");
}

TEST_CASE("escapeSql handles a classic injection attempt") {
    // Not a claim that this makes the app fully injection-proof (it
    // doesn't, to the standard of real parameterized queries -- see the
    // README) -- just confirming the specific characters it's meant to
    // neutralize actually get neutralized.
    std::string attempt = "x'; DROP TABLE users; --";
    std::string escaped = escapeSql(attempt);
    REQUIRE(escaped.find(';') == std::string::npos);
    REQUIRE(escaped.find("';") == std::string::npos);
}

TEST_CASE("parseRows skips the header row") {
    std::string raw = "id\tname\n1\tMouse\n";
    auto rows = parseRows(raw);
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0][0] == "1");
    REQUIRE(rows[0][1] == "Mouse");
}

TEST_CASE("parseRows handles multiple data rows") {
    std::string raw = "id\tname\tstock\n1\tMouse\t50\n2\tKeyboard\t30\n";
    auto rows = parseRows(raw);
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0][2] == "50");
    REQUIRE(rows[1][1] == "Keyboard");
}

TEST_CASE("parseRows returns nothing for header-only input") {
    std::string raw = "id\tname\n";
    auto rows = parseRows(raw);
    REQUIRE(rows.empty());
}

TEST_CASE("parseRows returns nothing for completely empty input") {
    auto rows = parseRows("");
    REQUIRE(rows.empty());
}
