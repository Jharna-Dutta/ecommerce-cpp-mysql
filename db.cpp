#include "db.hpp"

#include <cstdio>
#include <array>
#include <stdexcept>
#include <sstream>

#ifdef _WIN32
// On Windows, the POSIX names are _popen/_pclose.
#define popen  _popen
#define pclose _pclose
#endif

std::string runQuery(const std::string& query) {
    // --defaults-extra-file keeps the password out of the command line
    // (and out of the process list), unlike passing -pYOURPASSWORD directly.
    // 2>&1 pulls MySQL's error messages into the same stream we read, so
    // a bad query shows up in the returned string instead of vanishing.
    std::string command =
        "mysql --defaults-extra-file=db_config.cnf -D ecommerce --batch --raw -e \""
        + query + "\" 2>&1";

    std::array<char, 512> buffer;
    std::string result;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Could not start mysql. Is it on your PATH?");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    pclose(pipe);
    return result;
}

std::vector<std::vector<std::string>> parseRows(const std::string& raw) {
    std::vector<std::vector<std::string>> rows;
    std::istringstream lineStream(raw);
    std::string line;
    bool firstLine = true;

    while (std::getline(lineStream, line)) {
        if (firstLine) {
            firstLine = false;
            continue; // skip the header row
        }
        if (line.empty()) continue;

        std::vector<std::string> cols;
        std::istringstream colStream(line);
        std::string cell;
        while (std::getline(colStream, cell, '\t')) {
            cols.push_back(cell);
        }
        rows.push_back(cols);
    }
    return rows;
}

std::string escapeSql(const std::string& input) {
    std::string out;
    for (char c : input) {
        if (c == '\'') {
            out += "''";
        } else if (c == '\\' || c == ';' || c == '"') {
            // drop characters that could break out of the query
            continue;
        } else {
            out += c;
        }
    }
    return out;
}
