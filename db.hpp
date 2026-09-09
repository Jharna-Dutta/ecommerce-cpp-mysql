#ifndef DB_HPP
#define DB_HPP

#include <string>
#include <vector>

// Runs one or more ';'-separated SQL statements against the `ecommerce`
// database by shelling out to the mysql command-line client, and returns
// whatever it printed (in --batch --raw mode: tab-separated, one row per
// line). This is the "skip the connector library" approach -- no MySQL
// C/C++ headers or link libraries needed, just the mysql.exe that's
// already on your PATH.
std::string runQuery(const std::string& query);

// Turns runQuery()'s raw tab-separated output into rows of columns.
// The first line is a header row (column names) and is skipped.
std::vector<std::vector<std::string>> parseRows(const std::string& raw);

// Makes a value safe to place inside single quotes in a query string.
// Doubles any single quotes, and strips backslashes/semicolons/double
// quotes so a value can't break out of the query or chain a second
// statement onto it. This is a basic defense, not a substitute for
// real parameterized queries -- worth knowing as a limitation of the
// "call the CLI as a subprocess" approach.
std::string escapeSql(const std::string& input);

#endif
